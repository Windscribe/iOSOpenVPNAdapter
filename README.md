# Modern OpenVPN Adapter

A Swift implementation of OpenVPN adapter for iOS/macOS with Objective-C++ bridge to OpenVPN3 core.

## Features

- Swift interface with Objective-C++ bridge to OpenVPN3 C++ core
- iOS 14.0+, macOS 11.0+, tvOS 14.0+ support
- Windscribe anti-censorship features (UDP stuffing, TCP split/reset)
- Swift Package Manager compatible
- Network Extension integration ready

## Architecture

```
┌─────────────────────────────┐
│        Swift Layer          │  ← Your app uses this
├─────────────────────────────┤
│   Objective-C++ Bridge      │  ← OpenVPNClientWrapper
├─────────────────────────────┤
│     OpenVPN3 C++ Core       │  ← Core VPN protocol
├─────────────────────────────┤
│  mbedTLS | ASIO | LZ4       │  ← Dependencies
└─────────────────────────────┘
```

## Installation

### Swift Package Manager

Add to your `Package.swift` or Xcode project:

```swift
dependencies: [
    .package(path: "../ModernOpenVPNAdapter")
]
```

## Usage

### Basic Connection

```swift
import OpenVPNAdapter

// Create and configure adapter
let adapter = OpenVPNAdapter()

let configuration = OpenVPNConfiguration()
configuration.fileContent = ovpnFileData
configuration.server = "vpn.example.com"
configuration.port = 1194
configuration.proto = .udp

// Anti-censorship features (optional)
configuration.udpStuffing = true      // Evades DPI
configuration.tcpSplitReset = true    // Confuses censorship

try adapter.applyConfiguration(configuration)

// Set credentials
let credentials = OpenVPNCredentials()
credentials.username = "user"
credentials.password = "pass"
try adapter.provideCredentials(credentials)

// Connect
adapter.connect { event, message in
    switch event {
    case .connected:
        print("Connected")
    case .disconnected:
        print("Disconnected")
    default:
        break
    }
}
```

### Network Extension

```swift
class PacketTunnelProvider: NEPacketTunnelProvider {
    let adapter = OpenVPNAdapter()

    override func startTunnel(options: [String : NSObject]?,
                            completionHandler: @escaping (Error?) -> Void) {
        // Configure adapter with PacketFlow
        adapter.connectWithPacketFlow(packetFlow) { event, message in
            // Handle connection events
        }
    }
}
```

## Anti-Censorship Features

This adapter includes Windscribe's anti-censorship patches:

- **UDP Stuffing**: Adds random padding to UDP packets to evade Deep Packet Inspection
- **TCP Split/Reset**: Splits TCP handshake to confuse censorship systems

These features are preserved when upgrading OpenVPN3. To verify patches are active:

```bash
./scripts/check-versions.sh
```

To reapply patches after OpenVPN3 upgrade:

```bash
cd Sources/OpenVPNCore/OpenVPN3
git apply ../../../patches/windscribe-anticensorship.patch
```

## Project Structure

```
ModernOpenVPNAdapter/
├── Package.swift                    # SPM configuration
├── Sources/
│   ├── OpenVPNCore/                # C++ dependencies
│   │   ├── OpenVPN3/               # OpenVPN 3 core
│   │   ├── mbedTLS/                # Crypto library
│   │   ├── ASIO/                   # Network I/O
│   │   └── LZ4/                    # Compression
│   ├── OpenVPNAdapter/
│   │   ├── Core/                   # Objective-C++ bridge
│   │   │   ├── OpenVPNClientWrapper.mm
│   │   │   └── OpenVPNPacketFlowBridge.mm
│   │   └── Swift/                  # Swift API
│   │       └── OpenVPNAdapter.swift
│   └── PacketTunnelProvider/       # Example Network Extension
├── patches/
│   └── windscribe-anticensorship.patch
└── scripts/
    ├── check-versions.sh           # Check dependencies & patches
    └── apply-patches.sh            # Apply anti-censorship patches
```

## Building

```bash
# Command line
swift build
swift test

# Or open Package.swift in Xcode
```

## Requirements

- iOS 14.0+ / macOS 11.0+ / tvOS 14.0+
- Xcode 14.0+
- Swift 5.10+

## License

Components include:
- OpenVPN 3 (AGPLv3)
- mbedTLS (Apache 2.0)
- ASIO (Boost Software License)
- LZ4 (BSD 2-Clause)