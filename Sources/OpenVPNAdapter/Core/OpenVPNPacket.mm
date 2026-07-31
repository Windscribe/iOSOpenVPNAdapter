//  Handles packet format conversion between NEPacketTunnelFlow and socket formats

#import "../include/OpenVPNPacket.h"
#import <arpa/inet.h>

// Protocol families
#ifndef AF_INET
#define AF_INET 2
#endif

#ifndef AF_INET6
#define AF_INET6 30
#endif

@interface OpenVPNPacket ()
@property (nonatomic, readwrite) NSData *vpnData;
@property (nonatomic, readwrite) NSData *packetFlowData;
@property (nonatomic, readwrite) NSNumber *protocolFamily;
@end

@implementation OpenVPNPacket

- (instancetype)initWithPacketFlowData:(NSData *)data protocolFamily:(NSNumber *)protocolFamily {
    self = [super init];
    if (self) {
        _packetFlowData = data;
        _protocolFamily = protocolFamily;

        // Create VPN data by prepending protocol family header
        NSMutableData *vpnData = [[NSMutableData alloc] init];

        // Convert protocol family to network byte order (big-endian)
        uint32_t protocolHeader = htonl([protocolFamily unsignedIntValue]);
        [vpnData appendBytes:&protocolHeader length:sizeof(protocolHeader)];
        [vpnData appendData:data];

        _vpnData = [vpnData copy];
    }
    return self;
}

- (instancetype)initWithVPNData:(NSData *)data {
    self = [super init];
    if (self) {
        _vpnData = data;

        // Extract protocol family from first 4 bytes
        if (data.length >= sizeof(uint32_t)) {
            uint32_t protocolHeader;
            [data getBytes:&protocolHeader length:sizeof(protocolHeader)];

            // Convert from network byte order to host byte order
            uint32_t protocolFamilyValue = ntohl(protocolHeader);
            _protocolFamily = @(protocolFamilyValue);

            // Extract packet data (skip protocol header)
            _packetFlowData = [data subdataWithRange:NSMakeRange(sizeof(uint32_t), data.length - sizeof(uint32_t))];
        } else {
            // Invalid packet
            _protocolFamily = @(AF_INET);
            _packetFlowData = [[NSData alloc] init];
        }
    }
    return self;
}

@end