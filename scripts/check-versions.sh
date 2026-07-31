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

# Check for Windscribe patches
if grep -q "WINDSCRIBE ANTICENSORSHIP" Sources/OpenVPNCore/OpenVPN3/openvpn/client/cliopt.hpp 2>/dev/null; then
    echo "✅ Anti-censorship markers: FOUND"
    echo "   - UDP Stuffing placeholders"
    echo "   - TCP Split/Reset placeholders"
else
    echo "❌ Anti-censorship patches: NOT APPLIED"
    echo "   Run: ./scripts/apply-patches.sh"
fi

echo ""
echo "To reapply patches after upgrading: ./scripts/apply-patches.sh"