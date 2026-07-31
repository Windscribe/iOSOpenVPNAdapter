#!/bin/bash

# Dependency versions - CHANGE THESE TO UPGRADE
OPENVPN3_VERSION="release/3.11.7"  # OpenVPN3 latest stable
MBEDTLS_VERSION="v3.6.2"           # mbedTLS uses vX.X.X format
LZ4_VERSION="v1.10.0"              # LZ4 uses vX.X.X format
ASIO_VERSION="1.31.0"              # ASIO uses X.X.X format

echo "🔄 OpenVPN Adapter Dependency Upgrade"
echo "====================================="
echo ""
echo "Target versions:"
echo "  OpenVPN3: $OPENVPN3_VERSION"
echo "  mbedTLS:  $MBEDTLS_VERSION"
echo "  LZ4:      $LZ4_VERSION"
echo "  ASIO:     $ASIO_VERSION"
echo ""

# Confirm before proceeding
read -p "⚠️  This will DELETE and re-download all dependencies. Continue? (y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cancelled."
    exit 1
fi

cd Sources/OpenVPNCore

echo ""
echo "🗑️  Removing old dependencies..."
rm -rf OpenVPN3 mbedTLS LZ4 ASIO

echo "📥 Downloading OpenVPN3 $OPENVPN3_VERSION..."
git clone --depth 1 --branch $OPENVPN3_VERSION https://github.com/OpenVPN/openvpn3.git OpenVPN3

echo "📥 Downloading mbedTLS $MBEDTLS_VERSION..."
git clone --depth 1 --branch $MBEDTLS_VERSION https://github.com/Mbed-TLS/mbedtls.git mbedTLS

echo "📥 Downloading LZ4 $LZ4_VERSION..."
git clone --depth 1 --branch $LZ4_VERSION https://github.com/lz4/lz4.git LZ4

echo "📥 Downloading ASIO $ASIO_VERSION..."
# ASIO doesn't use v prefix for tags
git clone --depth 1 --branch asio-${ASIO_VERSION//./-} https://github.com/chriskohlhoff/asio.git ASIO

echo ""
echo "🔧 Applying patches..."

cd OpenVPN3

# Apply iOS UIKit fix (removes UIKit dependency from C++ code)
if [ -f "../../../patches/fix-ios-uikit.patch" ]; then
    git apply ../../../patches/fix-ios-uikit.patch
    echo "✅ iOS UIKit patch applied (fixes C++ build errors with UIKit headers)"
fi

# Apply hostname verification fix for mbedTLS
if [ -f "../../../patches/hostname-verification.patch" ]; then
    git apply ../../../patches/hostname-verification.patch
    echo "✅ Hostname verification patch applied (fixes SSL verification with IP addresses)"
fi

# Apply Windscribe anti-censorship features (FULL IMPLEMENTATION)
# Copy pre-patched files with full anti-censorship implementation
if [ -d "../../../patches/anticensorship-files" ]; then
    cp ../../../patches/anticensorship-files/cliopt.hpp openvpn/client/cliopt.hpp 2>/dev/null
    cp ../../../patches/anticensorship-files/tcpcli.hpp openvpn/transport/client/tcpcli.hpp 2>/dev/null
    cp ../../../patches/anticensorship-files/udpcli.hpp openvpn/transport/client/udpcli.hpp 2>/dev/null
    echo "✅ Windscribe anti-censorship features applied (UDP stuffing + TCP split/reset)"
else
    echo "⚠️  No anti-censorship patches found (directory removed)"
fi

cd ../../..

echo ""
echo "📊 Upgrade complete! Current versions:"
echo "======================================"
./scripts/check-versions.sh

echo ""
echo "🧪 Test build with: swift build"