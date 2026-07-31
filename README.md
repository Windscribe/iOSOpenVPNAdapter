# ModernOpenVPNAdapter

Pure Objective-C OpenVPN adapter for iOS and tvOS with OpenVPN3 core integration.

## Features

- Pure Objective-C implementation with Objective-C++ bridge to OpenVPN3 core
- iOS 13.0+ and tvOS 13.0+ support
- Socket pair bridge pattern for packet flow
- Windscribe anti-censorship features (UDP stuffing, TCP split/reset)
- Swift Package Manager compatible
- Network Extension integration ready

## Architecture

```
┌─────────────────────────────┐
│    iOS/tvOS Application     │
├─────────────────────────────┤
│     OpenVPNAdapter (ObjC)   │  ← Main interface
├─────────────────────────────┤
│   OpenVPNClient (ObjC++)    │  ← C++ bridge with delegate pattern
├─────────────────────────────┤
│  OpenVPNPacketFlowBridge    │  ← Socket pair implementation
├─────────────────────────────┤
│    OpenVPN3 C++ Core        │  ← Core VPN protocol (3.11.7)
├─────────────────────────────┤
│  mbedTLS | ASIO | LZ4       │  ← Dependencies
└─────────────────────────────┘
```

## Installation

### Swift Package Manager

Add to your `Package.swift`:

```swift
dependencies: [
    .package(url: "https://github.com/Windscribe/ModernOpenVPNAdapter.git", from: "1.0.0")
]
```

Or add via Xcode: File → Add Package Dependencies

## Usage

### Basic Connection in PacketTunnelProvider

```objc
#import <OpenVPNAdapter/OpenVPNAdapter.h>

@interface PacketTunnelProvider () <OpenVPNAdapterDelegate, OpenVPNAdapterPacketFlow>
@property (nonatomic, strong) OpenVPNAdapter *adapter;
@end

@implementation PacketTunnelProvider

- (void)startTunnelWithOptions:(NSDictionary *)options
             completionHandler:(void (^)(NSError *))completionHandler {

    // Create adapter
    self.adapter = [[OpenVPNAdapter alloc] init];
    self.adapter.delegate = self;

    // Configure
    OpenVPNConfiguration *configuration = [[OpenVPNConfiguration alloc] init];
    configuration.fileContent = ovpnFileData;
    configuration.disableClientCert = YES;  // For username/password auth

    NSError *error;
    [self.adapter applyConfiguration:configuration error:&error];
    if (error) {
        completionHandler(error);
        return;
    }

    // Provide credentials
    OpenVPNCredentials *credentials = [[OpenVPNCredentials alloc] init];
    credentials.username = @"username";
    credentials.password = @"password";
    [self.adapter provideCredentials:credentials error:&error];

    // Connect
    [self.adapter connectUsingPacketFlow:self];
}

#pragma mark - OpenVPNAdapterDelegate

- (void)openVPNAdapter:(OpenVPNAdapter *)openVPNAdapter
configureTunnelWithNetworkSettings:(NEPacketTunnelNetworkSettings *)networkSettings
     completionHandler:(void (^)(NSError *))completionHandler {

    // Apply network settings to tunnel
    [self setTunnelNetworkSettings:networkSettings completionHandler:completionHandler];
}

- (void)openVPNAdapter:(OpenVPNAdapter *)openVPNAdapter
           handleEvent:(OpenVPNAdapterEvent)event
               message:(NSString *)message {

    switch (event) {
        case OpenVPNAdapterEventConnected:
            NSLog(@"Connected");
            break;
        case OpenVPNAdapterEventDisconnected:
            NSLog(@"Disconnected");
            break;
        default:
            break;
    }
}

- (void)openVPNAdapter:(OpenVPNAdapter *)openVPNAdapter
           handleError:(NSError *)error {
    NSLog(@"Error: %@", error);
}

#pragma mark - OpenVPNAdapterPacketFlow

- (void)writePackets:(NSArray<NSData *> *)packets withProtocols:(NSArray<NSNumber *> *)protocols {
    [self.packetFlow writePackets:packets withProtocols:protocols];
}

- (void)readPacketsWithCompletionHandler:(void (^)(NSArray<NSData *> *, NSArray<NSNumber *> *))completionHandler {
    [self.packetFlow readPacketsWithCompletionHandler:completionHandler];
}

@end
```

### Swift Usage

```swift
import OpenVPNAdapter

class PacketTunnelProvider: NEPacketTunnelProvider {
    let adapter = OpenVPNAdapter()

    override func startTunnel(options: [String : NSObject]?,
                            completionHandler: @escaping (Error?) -> Void) {

        adapter.delegate = self

        // Configure
        let configuration = OpenVPNConfiguration()
        configuration.fileContent = ovpnData
        configuration.disableClientCert = true  // For username/password auth

        do {
            try adapter.applyConfiguration(configuration)

            // Credentials
            let credentials = OpenVPNCredentials()
            credentials.username = "username"
            credentials.password = "password"
            try adapter.provideCredentials(credentials)

            // Connect
            adapter.connect(using: packetFlow)

        } catch {
            completionHandler(error)
        }
    }
}

extension PacketTunnelProvider: OpenVPNAdapterDelegate {
    func openVPNAdapter(_ adapter: OpenVPNAdapter,
                       configureTunnelWith settings: NEPacketTunnelNetworkSettings?,
                       completionHandler: @escaping (Error?) -> Void) {

        setTunnelNetworkSettings(settings, completionHandler: completionHandler)
    }

    func openVPNAdapter(_ adapter: OpenVPNAdapter,
                       handleEvent event: OpenVPNAdapterEvent,
                       message: String?) {
        // Handle events
    }

    func openVPNAdapter(_ adapter: OpenVPNAdapter, handleError error: Error) {
        // Handle errors
    }
}
```

## Key Components

### OpenVPNAdapter
Main Objective-C interface providing connection management and configuration.

### OpenVPNPacketFlowBridge
Implements socket pair pattern using `socketpair()` and `CFSocket` for packet forwarding between OpenVPN3 and NEPacketTunnelFlow.

### OpenVPNClient
C++ bridge implementing `ClientAPI::OpenVPNClient` with delegation pattern. All TunBuilder methods delegate to Objective-C for proper tunnel configuration.

### Model Objects
- `OpenVPNConfiguration` - VPN configuration settings
- `OpenVPNCredentials` - Authentication credentials
- `OpenVPNConnectionInfo` - Connection status information
- `OpenVPNInterfaceStats` - Interface statistics
- `OpenVPNTransportStats` - Transport layer statistics

## Anti-Censorship Features

Includes Windscribe's anti-censorship implementations:

- **UDP Stuffing**: Sends 10-100 random padding packets during handshake to evade DPI
- **TCP Split/Reset**: Splits TCP handshake into 2-10 byte fragments to confuse censorship

These features are automatically applied when upgrading OpenVPN3 via the upgrade script.

## Project Structure

```
ModernOpenVPNAdapter/
├── Package.swift
├── Sources/
│   ├── OpenVPNCore/                # C++ dependencies
│   │   ├── OpenVPN3/               # OpenVPN3 core (3.11.7)
│   │   ├── mbedTLS/                # Crypto library (3.6.2)
│   │   ├── ASIO/                   # Network I/O (1.31.0)
│   │   └── LZ4/                    # Compression (1.10.0)
│   └── OpenVPNAdapter/
│       ├── include/                # Public headers
│       │   ├── OpenVPNAdapter.h
│       │   ├── OpenVPNConfiguration.h
│       │   ├── OpenVPNCredentials.h
│       │   └── module.modulemap
│       └── Core/                   # Implementation
│           ├── OpenVPNAdapter.mm
│           ├── OpenVPNClient.mm    # C++ bridge
│           └── OpenVPNPacketFlowBridge.mm
├── patches/
│   ├── hostname-verification.patch # Fix for mbedTLS 2.28.10+
│   └── anticensorship-files/      # Anti-censorship implementations
└── scripts/
    └── upgrade-dependencies.sh     # Upgrade OpenVPN3 and dependencies
```

## Upgrading Dependencies

To upgrade OpenVPN3 and dependencies:

```bash
# Edit version numbers at top of script
vi scripts/upgrade-dependencies.sh

# Run upgrade
./scripts/upgrade-dependencies.sh

# Build and test
swift build
swift test
```

The script automatically:
- Downloads specified versions
- Applies hostname verification patch
- Copies anti-censorship implementations

## Requirements

- iOS 13.0+ / tvOS 13.0+
- Xcode 14.0+
- Swift 5.5+ (for Swift usage)

## Important Notes

1. **Authentication**: Set `disableClientCert = YES` when using username/password authentication without client certificates.

2. **Socket Pair Bridge**: The adapter uses socket pairs created with `socketpair(PF_LOCAL, SOCK_DGRAM, IPPROTO_IP)` for packet forwarding.

3. **Events**: OpenVPN3 sends event names in UPPERCASE (e.g., "CONNECTED" not "connected").

4. **Platform Support**: This package only supports iOS and tvOS. macOS support has been removed.

## License

Components include:
- OpenVPN3 (AGPLv3)
- mbedTLS (Apache 2.0)
- ASIO (Boost Software License)
- LZ4 (BSD 2-Clause)