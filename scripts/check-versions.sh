#!/bin/bash

# Check dependency versions and anti-censorship patches

echo "📊 OpenVPN Adapter Dependencies"
echo "==============================="
echo ""

# Check if dependencies are vendored or submodules
if [ -f ".gitmodules" ]; then
    echo "✅ Using git submodules"
    git submodule status
else
    echo "📦 Dependencies are vendored (copied directly)"
    echo ""
    echo "Components:"
    echo "  • OpenVPN3 - VPN protocol implementation"
    echo "  • mbedTLS  - Cryptography library"
    echo "  • LZ4      - Compression library"
    echo "  • ASIO     - Network I/O library"
fi

echo ""
echo "🛡️  Anti-Censorship Patches"
echo "============================"

# Check for anti-censorship features (new full port: anti_censorship.hpp
# applied via patches/anti-censorship-3.11.7.patch)
if [ -f "Sources/OpenVPNCore/OpenVPN3/openvpn/client/anti_censorship.hpp" ] && \
   grep -q "AntiCensorship::parse_options" Sources/OpenVPNCore/OpenVPN3/openvpn/client/cliopt.hpp 2>/dev/null; then
    echo "✅ Anti-censorship patches: APPLIED (udp-stuffing, tcp-split-reset, i1-i5, Jc/Jmin/Jmax, junk-first)"
else
    echo "❌ Anti-censorship patches: NOT APPLIED"
    echo "   Run: ./scripts/upgrade-dependencies.sh (applies patches/anti-censorship-3.11.7.patch)"
fi

echo ""
echo "To reapply patches after upgrading: ./scripts/upgrade-dependencies.sh"