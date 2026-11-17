//    OpenVPN -- An application to securely tunnel IP networks
//               over a single port, with support for SSL/TLS-based
//               session authentication, key exchange,
//               packet encryption, packet authentication, and
//               packet compression.
//
//    SPDX-License-Identifier: MPL-2.0 OR AGPL-3.0-only WITH openvpn3-openssl-exception
//

// Anti-censorship features for OpenVPN 3.
//
// This module is the OpenVPN 3 port of the OpenVPN 2 anti-censorship
// patches (anti_censorship.c/h) and implements:
//
//   - udp-stuffing    : send random UDP packets before the hard-reset packet
//   - tcp-split-reset : split the TCP hard-reset packet into small pieces
//                       and emit multiple hard-reset packets
//   - i1..i5          : user-defined pre-connect packets sent before the
//                       hard-reset packet (UDP only)
//   - jc/jmin/jmax    : Amnezia-style junk packets sent around the i1..i5
//                       pre-connect packets (UDP only)
//   - junk-first      : send junk packets before (instead of after) i1..i5
//
// All logic is intentionally kept in this separate file to ease maintenance
// of out-of-tree code, mirroring the anti_censorship.c/h layout of the
// OpenVPN 2 patches.

#ifndef OPENVPN_CLIENT_ANTICENSORSHIP_H
#define OPENVPN_CLIENT_ANTICENSORSHIP_H

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

#include <ctime>

#include <openvpn/common/exception.hpp>
#include <openvpn/common/options.hpp>
#include <openvpn/common/size.hpp>
#include <openvpn/buffer/buffer.hpp>
#include <openvpn/random/randapi.hpp>
#include <openvpn/transport/protocol.hpp>

namespace openvpn::AntiCensorship {

// UDP stuffing parameters (same values as the OpenVPN 2 patch)
constexpr size_t STUFFING_LEN_MAX = 900;
constexpr size_t STUFFING_LEN_MIN = 200;
constexpr size_t STUFFING_NUM_MAX = 100;
constexpr size_t STUFFING_NUM_MIN = 10;

// TCP split-reset parameters
constexpr size_t SPLIT_PIECE_LEN_MAX = 9; // random piece length is 2..9 bytes
constexpr size_t SPLIT_PIECE_LEN_MIN = 2;
constexpr size_t SPLIT_RESET_COUNT_MAX = 3; // 2..3 hard-reset packets
constexpr size_t SPLIT_RESET_COUNT_MIN = 2;

// AmneziaWG-style pre-connect packet (i1..i5) parameters
constexpr size_t PRE_CONNECT_COUNT = 5;    // number of i1..i5 slots
constexpr size_t PRE_CONNECT_MAX = 1400;   // max assembled packet size
constexpr size_t TAG_RAND_MAX = 1400;      // max bytes for <r N>
constexpr size_t TAG_RANDC_MAX = 1400;     // max chars for <rc N>/<rd N>

// Junk packet parameters (jc/jmin/jmax)
constexpr int JUNK_PACKET_COUNT_MAX = 1000;
constexpr int JUNK_PACKET_SIZE_MAX = 1400;

// OpenVPN packet opcodes needed by the anti-censorship logic.
constexpr unsigned int P_OPCODE_SHIFT = 3;
constexpr unsigned int P_CONTROL_HARD_RESET_CLIENT_V2 = 7;
constexpr unsigned int P_CONTROL_HARD_RESET_SERVER_V2 = 8;
constexpr unsigned int P_CONTROL_HARD_RESET_CLIENT_V3 = 10;

/**
 * Extract the opcode from the first byte of an OpenVPN packet.
 */
inline unsigned int opcode_extract(const unsigned char first_byte)
{
    return first_byte >> P_OPCODE_SHIFT;
}

/**
 * True if the packet is a hard-reset packet (any direction).
 */
inline bool is_hard_reset(const unsigned char first_byte)
{
    const unsigned int op = opcode_extract(first_byte);
    return op == P_CONTROL_HARD_RESET_CLIENT_V2
           || op == P_CONTROL_HARD_RESET_CLIENT_V3
           || op == P_CONTROL_HARD_RESET_SERVER_V2;
}

/**
 * True if the packet is a client hard-reset packet (the packet type
 * a client emits to initiate a session).
 */
inline bool is_client_hard_reset(const unsigned char first_byte)
{
    const unsigned int op = opcode_extract(first_byte);
    return op == P_CONTROL_HARD_RESET_CLIENT_V2
           || op == P_CONTROL_HARD_RESET_CLIENT_V3;
}

// Configuration of the anti-censorship features, parsed from the
// OpenVPN configuration options.
struct Config
{
    bool udp_stuffing = false; // udp-stuffing
    bool tcp_split_reset = false; // tcp-split-reset
    bool junk_first = false;      // junk-first

    int junk_packet_count = 0;    // jc: number of junk packets
    int junk_packet_min_size = 0; // jmin: minimum junk packet size
    int junk_packet_max_size = 0; // jmax: maximum junk packet size

    std::array<std::string, PRE_CONNECT_COUNT> i_packets; // i1..i5 specs

    bool udp_pre_reset_defined() const
    {
        return udp_stuffing || junk_packet_count > 0
               || std::any_of(i_packets.begin(),
                              i_packets.end(),
                              [](const std::string &s)
                              { return !s.empty(); });
    }
};

/**
 * Parse the anti-censorship options from an OptionList.
 *
 * All recognized options are marked as touched so that they don't show
 * up as unknown/unsupported options.  Invalid values throw option_error,
 * like other options.
 *
 * This mirrors the option parsing of the OpenVPN 2 patches:
 *   udp-stuffing, tcp-split-reset, i1..i5/I1..I5, jc/Jc, jmin/Jmin,
 *   jmax/Jmax and junk-first.
 */
inline void parse_options(const OptionList &opt, Config &cfg)
{
    auto touch = [](const OptionList &optlist, const char *name)
    {
        const Option *o = optlist.get_ptr(name);
        if (o)
            o->touch();
    };

    // udp-stuffing (no parameter)
    if (opt.exists("udp-stuffing"))
    {
        touch(opt, "udp-stuffing");
        cfg.udp_stuffing = true;
    }

    // tcp-split-reset (no parameter)
    if (opt.exists("tcp-split-reset"))
    {
        touch(opt, "tcp-split-reset");
        cfg.tcp_split_reset = true;
    }

    // i1..i5 (both lower and upper case accepted, like OpenVPN 2)
    for (size_t i = 0; i < PRE_CONNECT_COUNT; ++i)
    {
        const std::string lower = "i" + std::to_string(i + 1);
        const std::string upper = "I" + std::to_string(i + 1);
        for (const char *name : {lower.c_str(), upper.c_str()})
        {
            const Option *o = opt.get_ptr(name);
            if (o)
            {
                o->min_args(2);
                o->exact_args(2);
                cfg.i_packets[i] = o->get(1, 0); // no length limit
                o->touch();
            }
        }
    }

    // jc/jmin/jmax
    {
        const Option *o = opt.get_ptr("jc");
        if (!o)
            o = opt.get_ptr("Jc");
        if (o)
        {
            o->min_args(2);
            o->exact_args(2);
            const int val = o->get_num<int>(1);
            if (val < 0 || val > JUNK_PACKET_COUNT_MAX)
                OPENVPN_THROW_ARG1(option_error, ERR_INVALID_OPTION_VAL, o->err_ref() << " must be between 0 and " << JUNK_PACKET_COUNT_MAX);
            cfg.junk_packet_count = val;
            o->touch();
        }
    }
    for (const char *minname : {"jmin", "Jmin"})
    {
        const Option *o = opt.get_ptr(minname);
        if (o)
        {
            o->min_args(2);
            o->exact_args(2);
            const int val = o->get_num<int>(1);
            if (val < 1 || val > JUNK_PACKET_SIZE_MAX)
                OPENVPN_THROW_ARG1(option_error, ERR_INVALID_OPTION_VAL, o->err_ref() << " must be between 1 and " << JUNK_PACKET_SIZE_MAX);
            cfg.junk_packet_min_size = val;
            o->touch();
        }
    }
    for (const char *maxname : {"jmax", "Jmax"})
    {
        const Option *o = opt.get_ptr(maxname);
        if (o)
        {
            o->min_args(2);
            o->exact_args(2);
            const int val = o->get_num<int>(1);
            if (val < 1 || val > JUNK_PACKET_SIZE_MAX)
                OPENVPN_THROW_ARG1(option_error, ERR_INVALID_OPTION_VAL, o->err_ref() << " must be between 1 and " << JUNK_PACKET_SIZE_MAX);
            cfg.junk_packet_max_size = val;
            o->touch();
        }
    }

    // junk-first (no parameter)
    if (opt.exists("junk-first"))
    {
        touch(opt, "junk-first");
        cfg.junk_first = true;
    }
}

/**
 * Log the "UDP only" warnings for options used with a non-UDP transport,
 * mirroring options_postprocess_mutate_ce() of the OpenVPN 2 patches.
 * Called once the actual transport protocol is known.
 */
inline void warn_wrong_proto(const Config &cfg, const Protocol &proto)
{
    if (proto.is_udp())
        return;

    if (std::any_of(cfg.i_packets.begin(),
                    cfg.i_packets.end(),
                    [](const std::string &s)
                    { return !s.empty(); }))
    {
        OPENVPN_LOG("NOTICE: --I1-I5 are suitable only for proto UDP");
    }

    if (cfg.junk_packet_count > 0 || cfg.junk_packet_min_size > 0
        || cfg.junk_packet_max_size > 0)
    {
        OPENVPN_LOG("NOTICE: --Jc/--Jmin/--Jmax are suitable only for proto UDP");
    }

    if (cfg.junk_packet_min_size > 0 && cfg.junk_packet_max_size > 0
        && cfg.junk_packet_min_size > cfg.junk_packet_max_size)
    {
        OPENVPN_LOG("NOTICE: --Jmin (" << cfg.junk_packet_min_size
                                      << ") is greater than --Jmax ("
                                      << cfg.junk_packet_max_size
                                      << "), Jmin will be used as packet size");
    }
}

/**
 * Parse a hex string (with optional "0x" prefix) and append the decoded
 * bytes to buf.  Returns true on success.
 */
inline bool parse_hex_tag(const char *hex, size_t hexlen, Buffer &out)
{
    // skip optional "0x" / "0X" prefix
    if (hexlen >= 2 && hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
    {
        hex += 2;
        hexlen -= 2;
    }

    if (hexlen == 0 || hexlen % 2 != 0)
        return false;

    for (size_t i = 0; i < hexlen; i += 2)
    {
        if (!std::isxdigit((unsigned char)hex[i]) || !std::isxdigit((unsigned char)hex[i + 1]))
            return false;

        unsigned char byte = (unsigned char)(
            (std::isdigit((unsigned char)hex[i]) ? hex[i] - '0' : std::tolower((unsigned char)hex[i]) - 'a' + 10) << 4
            | (std::isdigit((unsigned char)hex[i + 1]) ? hex[i + 1] - '0' : std::tolower((unsigned char)hex[i + 1]) - 'a' + 10));
        out.write(&byte, 1);
    }
    return true;
}

/**
 * Build a pre-connect packet buffer from a tag-spec string.
 *
 * Tag syntax (identical to the OpenVPN 2 patches):
 *   <b hex_data>   static bytes (hex, even length, optional 0x prefix)
 *   <t>            32-bit Unix timestamp, network byte order
 *   <r N>          N cryptographically secure random bytes (N <= TAG_RAND_MAX)
 *   <rc N>         N random ASCII alphanumeric chars [A-Za-z0-9]
 *   <rd N>         N random decimal digit chars [0-9]
 *
 * @param spec  the i1..i5 option value
 * @param out   output buffer (capacity PRE_CONNECT_MAX, empty)
 * @param rng   random number generator
 * @return true on success, false on parse error (the packet is skipped)
 */
inline bool build_pre_connect_packet(const std::string &spec, BufferAllocated &out, RandomAPI &rng)
{
    const char *p = spec.c_str();

    while (*p)
    {
        // skip whitespace between tags
        while (*p == ' ' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        if (*p != '<')
        {
            OPENVPN_LOG("anti-censorship: build_pre_connect_packet: expected '<', got '" << *p << "'");
            return false;
        }
        p++; // skip '<'

        // find closing '>'
        const char *close = std::strchr(p, '>');
        if (!close)
        {
            OPENVPN_LOG("anti-censorship: build_pre_connect_packet: unclosed tag");
            return false;
        }

        const size_t taglen = (size_t)(close - p);
        const std::string tag(p, taglen);
        p = close + 1; // advance past '>'

        if (tag.empty())
        {
            OPENVPN_LOG("anti-censorship: build_pre_connect_packet: empty tag");
            return false;
        }

        // <t>: 32-bit Unix timestamp, network byte order
        if (tag == "t")
        {
            const uint32_t ts = htonl((uint32_t)time(nullptr));
            out.write(&ts, sizeof(ts));
        }
        // <b hex_data>: static hex-encoded payload
        else if (tag.size() > 2 && tag[0] == 'b' && tag[1] == ' ')
        {
            const char *hexdata = tag.c_str() + 2;
            if (!parse_hex_tag(hexdata, tag.size() - 2, out))
            {
                OPENVPN_LOG("anti-censorship: build_pre_connect_packet: invalid hex in <b> tag: " << hexdata);
                return false;
            }
        }
        // <r N>: N cryptographically secure random bytes
        else if (tag.size() > 2 && tag[0] == 'r' && tag[1] == ' ')
        {
            const int n = std::atoi(tag.c_str() + 2);
            if (n <= 0 || (size_t)n > TAG_RAND_MAX)
            {
                OPENVPN_LOG("anti-censorship: build_pre_connect_packet: <r> length out of range: " << n);
                return false;
            }
            if (out.size() + (size_t)n > PRE_CONNECT_MAX)
            {
                OPENVPN_LOG("anti-censorship: build_pre_connect_packet: <r> packet exceeds buffer size");
                return false;
            }
            unsigned char randbuf[TAG_RAND_MAX];
            rng.rand_bytes(randbuf, (size_t)n);
            out.write(randbuf, (size_t)n);
        }
        // <rc N>: N random alphanumeric ASCII chars [A-Za-z0-9]
        else if (tag.size() > 3 && tag[0] == 'r' && tag[1] == 'c' && tag[2] == ' ')
        {
            const int n = std::atoi(tag.c_str() + 3);
            if (n <= 0 || (size_t)n > TAG_RANDC_MAX)
            {
                OPENVPN_LOG("anti-censorship: build_pre_connect_packet: <rc> length out of range: " << n);
                return false;
            }
            if (out.size() + (size_t)n > PRE_CONNECT_MAX)
            {
                OPENVPN_LOG("anti-censorship: build_pre_connect_packet: <rc> packet exceeds buffer size");
                return false;
            }
            static const char alphanum[] =
                "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
            for (int i = 0; i < n; i++)
            {
                unsigned char idx;
                rng.rand_bytes(&idx, 1);
                const unsigned char ch = (unsigned char)alphanum[idx % (sizeof(alphanum) - 1)];
                out.write(&ch, 1);
            }
        }
        // <rd N>: N random decimal digit chars [0-9]
        else if (tag.size() > 3 && tag[0] == 'r' && tag[1] == 'd' && tag[2] == ' ')
        {
            const int n = std::atoi(tag.c_str() + 3);
            if (n <= 0 || (size_t)n > TAG_RANDC_MAX)
            {
                OPENVPN_LOG("anti-censorship: build_pre_connect_packet: <rd> length out of range: " << n);
                return false;
            }
            if (out.size() + (size_t)n > PRE_CONNECT_MAX)
            {
                OPENVPN_LOG("anti-censorship: build_pre_connect_packet: <rd> packet exceeds buffer size");
                return false;
            }
            for (int i = 0; i < n; i++)
            {
                unsigned char rnd;
                rng.rand_bytes(&rnd, 1);
                const unsigned char ch = (unsigned char)('0' + (rnd % 10));
                out.write(&ch, 1);
            }
        }
        else
        {
            OPENVPN_LOG("anti-censorship: build_pre_connect_packet: unknown tag: <" << tag << ">");
            return false;
        }
    }

    return true;
}

/**
 * Number of hard-reset packets to send for the TCP split-reset feature.
 * Returns 2 or 3 packets (chosen randomly), like the OpenVPN 2 patch.
 */
inline unsigned int multiple_resets(RandomAPI &rng)
{
    return 2 + rng.randrange32(2); // randrange32(2) yields 0..1
}

} // namespace openvpn::AntiCensorship

#endif // OPENVPN_CLIENT_ANTICENSORSHIP_H
