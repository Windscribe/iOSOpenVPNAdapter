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

// Get the local MAC addr of the interface that owns the default route

#ifndef OPENVPN_NETCONF_HWADDR_H
#define OPENVPN_NETCONF_HWADDR_H

#include <string>

#include <openvpn/common/platform.hpp>
#include <openvpn/addr/macaddr.hpp>

#if defined(OPENVPN_PLATFORM_WIN) && !defined(OPENVPN_PLATFORM_UWP)
#include <openvpn/tun/win/tunutil.hpp>
#elif defined(OPENVPN_PLATFORM_IPHONE) || defined(TARGET_OS_IPHONE)
// UIKit cannot be included in C++ files, only in Objective-C
// iOS/tvOS don't allow MAC address access for privacy reasons
#endif

namespace openvpn {
inline std::string get_hwaddr([[maybe_unused]] IP::Addr server_addr)
{
#if defined(OPENVPN_PLATFORM_WIN) && !defined(OPENVPN_PLATFORM_UWP)
    const TunWin::Util::BestGateway dg{AF_INET};
    if (dg.defined())
    {
        const TunWin::Util::IPAdaptersInfo ai_list;
        const IP_ADAPTER_INFO *ai = ai_list.adapter(dg.interface_index());
        if (ai)
        {
            const MACAddr mac(ai->Address);
            return mac.to_string();
        }
    }
#elif defined(OPENVPN_PLATFORM_IPHONE) || defined(TARGET_OS_IPHONE)
    // as reported at https://developer.apple.com/library/content/releasenotes/General/WhatsNewIniOS/Articles/iOS7.html#//apple_ref/doc/uid/TP40013162-SW34
    // we can't get the MAC address from iOS/tvOS for privacy reasons
    // Return a placeholder value since we can't access UIDevice from C++
    // The actual device ID can be obtained from the Objective-C bridge if needed
    return "00:00:00:00:00:00";
#endif
    return std::string();
}
} // namespace openvpn

#endif
