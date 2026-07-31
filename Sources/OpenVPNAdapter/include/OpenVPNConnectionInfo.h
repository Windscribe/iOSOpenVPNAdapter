#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNConnectionInfo : NSObject

@property (nonatomic, readonly, nullable) NSString *user;
@property (nonatomic, readonly, nullable) NSString *serverHost;
@property (nonatomic, readonly, nullable) NSString *serverPort;
@property (nonatomic, readonly, nullable) NSString *serverProto;
@property (nonatomic, readonly, nullable) NSString *serverIP;
@property (nonatomic, readonly, nullable) NSString *vpnIPv4;
@property (nonatomic, readonly, nullable) NSString *vpnIPv6;
@property (nonatomic, readonly, nullable) NSString *gatewayIPv4;
@property (nonatomic, readonly, nullable) NSString *gatewayIPv6;
@property (nonatomic, readonly, nullable) NSString *clientIP;
@property (nonatomic, readonly, nullable) NSString *tunName;
@property (nonatomic, readonly) BOOL defined;

@end

NS_ASSUME_NONNULL_END