//    OpenVPN -- An application to securely tunnel IP networks
//               over a single port, with support for SSL/TLS-based
//               session authentication and key exchange,
//               packet encryption, packet authentication, and
//               packet compression.
//
//    Copyright (C) 2012- OpenVPN Inc.
//
//    SPDX-License-Identifier: MPL-2.0 OR AGPL-3.0-only WITH openvpn3-openssl-exception
//

// UDP transport object specialized for client.

#ifndef OPENVPN_TRANSPORT_CLIENT_UDPCLI_H
#define OPENVPN_TRANSPORT_CLIENT_UDPCLI_H

#include <sstream>

#include <openvpn/io/io.hpp>

#include <openvpn/common/bigmutex.hpp>
#include <openvpn/common/likely.hpp>
#include <openvpn/common/platform.hpp>
#include <openvpn/transport/udplink.hpp>
#include <openvpn/transport/client/transbase.hpp>
#include <openvpn/transport/socket_protect.hpp>
#include <openvpn/client/remotelist.hpp>
#include <openvpn/client/anti_censorship.hpp>

namespace openvpn::UDPTransport {

class ClientConfig : public TransportClientFactory
{
  public:
    typedef RCPtr<ClientConfig> Ptr;

    RemoteList::Ptr remote_list;
    bool server_addr_float;
    bool synchronous_dns_lookup;
    int n_parallel;
    Frame::Ptr frame;
    SessionStats::Ptr stats;

    // Anti-censorship: udp-stuffing, junk packets and pre-connect (i1..i5)
    // packets sent before the hard-reset packet
    AntiCensorship::Config ac;
    RandomAPI::Ptr rng;

    SocketProtect *socket_protect;

#ifdef OPENVPN_GREMLIN
    Gremlin::Config::Ptr gremlin_config;
#endif

    static Ptr new_obj()
    {
        return new ClientConfig;
    }

    TransportClient::Ptr new_transport_client_obj(openvpn_io::io_context &io_context,
                                                  TransportClientParent *parent) override;

    void process_push(const OptionList &opt) override
    {
        remote_list->process_push(opt);
    }

  private:
    ClientConfig()
        : server_addr_float(false),
          synchronous_dns_lookup(false),
          n_parallel(8),
          socket_protect(nullptr)
    {
    }
};

class Client : public TransportClient, AsyncResolvableUDP
{
    typedef RCPtr<Client> Ptr;

    friend class ClientConfig;      // calls constructor
    friend class UDPLink<Client *>; // calls udp_read_handler

    typedef UDPLink<Client *> LinkImpl;

  public:
    void transport_start() override
    {
        if (!impl)
        {
            halt = false;
            if (config->remote_list->endpoint_available(&server_host, &server_port, &server_protocol))
            {
                start_connect_();
            }
            else
            {
                parent->transport_pre_resolve();

                if (config->synchronous_dns_lookup)
                {
                    openvpn_io::error_code error;
                    results_type results = resolver.resolve(server_host, server_port, error);
                    resolve_callback(error, results);
                }
                else
                {
                    async_resolve_name(server_host, server_port);
                }
            }
        }
    }

    bool transport_send_const(const Buffer &buf) override
    {
        return send(buf);
    }

    bool transport_send(BufferAllocated &buf) override
    {
        return send(buf);
    }

    bool transport_send_queue_empty() override // really only has meaning for TCP
    {
        return false;
    }

    bool transport_has_send_queue() override
    {
        return false;
    }

    void transport_stop_requeueing() override
    {
    }

    size_t transport_send_queue_size() override
    {
        return 0;
    }

    void reset_align_adjust(const size_t align_adjust) override
    {
        if (impl)
            impl->reset_align_adjust(align_adjust);
    }

    void server_endpoint_info(std::string &host, std::string &port, std::string &proto, std::string &ip_addr) const override
    {
        host = server_host;
        port = server_port;
        proto = server_protocol.str();
        const IP::Addr addr = server_endpoint_addr();
        ip_addr = addr.to_string();
    }

    IP::Addr server_endpoint_addr() const override
    {
        return IP::Addr::from_asio(server_endpoint.address());
    }

    unsigned short server_endpoint_port() const override
    {
        return server_endpoint.port();
    }

    openvpn_io::detail::socket_type native_handle() override
    {
        return socket.native_handle();
    }

    Protocol transport_protocol() const override
    {
        return server_protocol;
    }

    void stop() override
    {
        stop_();
    }
    ~Client() override
    {
        stop_();
    }

  private:
    Client(openvpn_io::io_context &io_context_arg,
           ClientConfig *config_arg,
           TransportClientParent *parent_arg)
        : AsyncResolvableUDP(io_context_arg),
          socket(io_context_arg),
          config(config_arg),
          parent(parent_arg),
          resolver(io_context_arg),
          halt(false)
    {
    }

    void transport_reparent(TransportClientParent *parent_arg) override
    {
        parent = parent_arg;
    }

    // Anti-censorship: send Jc junk UDP packets of random size between
    // Jmin and Jmax bytes, filled with random data.
    void send_junk_packets()
    {
        const AntiCensorship::Config &ac = config->ac;
        int jmin = ac.junk_packet_min_size;
        int jmax = ac.junk_packet_max_size;

        if (jmin <= 0 || jmax <= 0)
            return;

        if (jmin > jmax)
            jmax = jmin;

        const int range = jmax - jmin;

        for (int i = 0; i < ac.junk_packet_count; i++)
        {
            // pick a random size in [jmin, jmax]
            const int pkt_size = jmin + (range > 0 ? (int)config->rng->randrange32((uint32_t)range + 1) : 0);

            BufferAllocated junk_buf((size_t)pkt_size, 0);
            unsigned char *data = junk_buf.write_alloc((size_t)pkt_size);
            config->rng->rand_bytes(data, (size_t)pkt_size);
            impl->send(junk_buf, nullptr);
        }
    }

    // Anti-censorship: send the user-defined pre-connect packets (i1..i5)
    // built from their tag-spec strings.
    void send_pre_connect_packets()
    {
        const AntiCensorship::Config &ac = config->ac;
        for (size_t i = 0; i < AntiCensorship::PRE_CONNECT_COUNT; i++)
        {
            if (ac.i_packets[i].empty())
                continue;

            BufferAllocated pkt(AntiCensorship::PRE_CONNECT_MAX, 0);
            if (!AntiCensorship::build_pre_connect_packet(ac.i_packets[i], pkt, *config->rng))
            {
                OPENVPN_LOG("anti-censorship: failed to build packet i" << (i + 1) << ", skipping");
                continue;
            }

            if (pkt.empty())
                continue;

            impl->send(pkt, nullptr);
        }
    }

    // Anti-censorship: send random UDP stuffing packets (10..99 packets
    // of 200..899 bytes).
    void send_stuffing_packets()
    {
        const size_t stuffing_num = AntiCensorship::STUFFING_NUM_MIN + config->rng->randrange32((uint32_t)(AntiCensorship::STUFFING_NUM_MAX - AntiCensorship::STUFFING_NUM_MIN));

        for (size_t i = 0; i < stuffing_num; i++)
        {
            const size_t stuffing_len = AntiCensorship::STUFFING_LEN_MIN + config->rng->randrange32((uint32_t)(AntiCensorship::STUFFING_LEN_MAX - AntiCensorship::STUFFING_LEN_MIN));
            BufferAllocated stuffing_buf((size_t)stuffing_len, 0);
            unsigned char *data = stuffing_buf.write_alloc(stuffing_len);
            config->rng->rand_bytes(data, stuffing_len);
            impl->send(stuffing_buf, nullptr);
        }
    }

    bool send(const Buffer &buf)
    {
        if (impl)
        {
            // Anti-censorship: before the hard-reset packet is sent, emit
            // (in order) the junk packets (junk-first), the pre-connect
            // packets (i1..i5), the junk packets (default) and the random
            // UDP stuffing packets.
            if (!buf.empty() && config->rng
                && AntiCensorship::is_hard_reset(buf.c_data()[0]))
            {
                const AntiCensorship::Config &ac = config->ac;
                if (ac.junk_first && ac.junk_packet_count > 0)
                    send_junk_packets();
                send_pre_connect_packets();
                if (!ac.junk_first && ac.junk_packet_count > 0)
                    send_junk_packets();
                if (ac.udp_stuffing)
                    send_stuffing_packets();
            }

            const int err = impl->send(buf, nullptr);
            if (unlikely(err))
            {
                // While UDP errors are generally ignored, certain
                // errors should be forwarded up to the higher levels.
#ifdef OPENVPN_PLATFORM_IPHONE
                if (err == EADDRNOTAVAIL)
                {
                    stop();
                    parent->transport_error(Error::TRANSPORT_ERROR, "EADDRNOTAVAIL: Can't assign requested address");
                }
#endif
#ifdef OPENVPN_PLATFORM_WIN
                if (err == WSAEINVAL)
                {
                    stop();
                    parent->transport_error(Error::TRANSPORT_ERROR, "WSAEINVAL: An invalid argument was supplied.");
                }
#endif
                return false;
            }
            else
                return true;
        }
        else
            return false;
    }

    void udp_read_handler(PacketFrom::SPtr &pfp) // called by LinkImpl
    {
        if (config->server_addr_float || pfp->sender_endpoint == server_endpoint)
            parent->transport_recv(pfp->buf);
        else
            config->stats->error(Error::BAD_SRC_ADDR);
    }

    void stop_()
    {
        if (!halt)
        {
            halt = true;
            if (impl)
                impl->stop();
            socket.close();
            resolver.cancel();
            async_resolve_cancel();
        }
    }

    // called after DNS resolution has succeeded or failed
    void resolve_callback(const openvpn_io::error_code &error,
                          results_type results) override
    {
        if (!halt)
        {
            if (!error)
            {
                // save resolved endpoint list in remote_list
                config->remote_list->set_endpoint_range(results);
                start_connect_();
            }
            else
            {
                std::ostringstream os;
                os << "DNS resolve error on '" << server_host << "' for UDP session: " << error.message();
                config->stats->error(Error::RESOLVE_ERROR);
                stop();
                parent->transport_error(Error::UNDEF, os.str());
            }
        }
    }

    // do UDP connect
    void start_connect_()
    {
        config->remote_list->get_endpoint(server_endpoint);
        OPENVPN_LOG("Contacting " << server_endpoint << " via UDP");
        parent->transport_wait();
        socket.open(server_endpoint.protocol());

        if (config->socket_protect)
        {
            if (!config->socket_protect->socket_protect(socket.native_handle(), server_endpoint_addr()))
            {
                config->stats->error(Error::SOCKET_PROTECT_ERROR);
                stop();
                parent->transport_error(Error::UNDEF, "socket_protect error (UDP)");
                return;
            }
        }

        socket.async_connect(server_endpoint, [self = Ptr(this)](const openvpn_io::error_code &error)
                             {
                                                OPENVPN_ASYNC_HANDLER;
                                                self->start_impl_(error); });
    }

    // start I/O on UDP socket
    void start_impl_(const openvpn_io::error_code &error)
    {
        if (!halt)
        {
            if (!error)
            {
                impl.reset(new LinkImpl(this,
                                        socket,
                                        (*config->frame)[Frame::READ_LINK_UDP],
                                        config->stats));
#ifdef OPENVPN_GREMLIN
                impl->gremlin_config(config->gremlin_config);
#endif
                impl->start(config->n_parallel);
                parent->transport_connecting();
            }
            else
            {
                std::ostringstream os;
                os << "UDP connect error on '" << server_host << ':' << server_port << "' (" << server_endpoint << "): " << error.message();
                config->stats->error(Error::UDP_CONNECT_ERROR);
                stop();
                parent->transport_error(Error::UNDEF, os.str());
            }
        }
    }

    std::string server_host;
    std::string server_port;

    Protocol server_protocol;

    openvpn_io::ip::udp::socket socket;
    ClientConfig::Ptr config;
    TransportClientParent *parent;
    LinkImpl::Ptr impl;
    openvpn_io::ip::udp::resolver resolver;
    UDPTransport::AsioEndpoint server_endpoint;
    bool halt;
};

inline TransportClient::Ptr ClientConfig::new_transport_client_obj(openvpn_io::io_context &io_context,
                                                                   TransportClientParent *parent)
{
    return TransportClient::Ptr(new Client(io_context, this, parent));
}
} // namespace openvpn::UDPTransport

#endif
