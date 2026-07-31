//  Handles packet format conversion between NEPacketTunnelFlow and socket formats

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNPacket : NSObject

// The packet data in VPN format (with protocol header)
@property (nonatomic, readonly) NSData *vpnData;

// The packet data for NEPacketTunnelFlow (without header)
@property (nonatomic, readonly) NSData *packetFlowData;

// Protocol family number (AF_INET or AF_INET6)
@property (nonatomic, readonly) NSNumber *protocolFamily;

// Initialize from NEPacketTunnelFlow data
- (instancetype)initWithPacketFlowData:(NSData *)data protocolFamily:(NSNumber *)protocolFamily;

// Initialize from VPN socket data (includes protocol header)
- (instancetype)initWithVPNData:(NSData *)data;

@end

NS_ASSUME_NONNULL_END