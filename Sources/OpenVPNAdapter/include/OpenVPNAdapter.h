#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class OpenVPNAdapter;
@class OpenVPNConfiguration;
@class OpenVPNConfigurationEvaluation;
@class OpenVPNCredentials;
@class OpenVPNConnectionInfo;
@class OpenVPNInterfaceStats;
@class OpenVPNSessionToken;
@class OpenVPNTransportStats;
@class OpenVPNReachability;
@class NEPacketTunnelNetworkSettings;

typedef NS_ENUM(NSInteger, OpenVPNAdapterEvent);

@protocol OpenVPNAdapterPacketFlow;

@protocol OpenVPNAdapterDelegate <NSObject>

- (void)openVPNAdapter:(OpenVPNAdapter *)openVPNAdapter configureTunnelWithNetworkSettings:(nullable NEPacketTunnelNetworkSettings *)networkSettings completionHandler:(void (^)(NSError * _Nullable error))completionHandler;
- (void)openVPNAdapter:(OpenVPNAdapter *)openVPNAdapter handleEvent:(OpenVPNAdapterEvent)event message:(nullable NSString *)message;
- (void)openVPNAdapter:(OpenVPNAdapter *)openVPNAdapter handleError:(NSError *)error;

@optional
- (void)openVPNAdapter:(OpenVPNAdapter *)openVPNAdapter handleLogMessage:(NSString *)logMessage;
- (void)openVPNAdapterDidReceiveClockTick:(OpenVPNAdapter *)openVPNAdapter;

@end

@interface OpenVPNAdapter : NSObject

@property (nonatomic, weak) id<OpenVPNAdapterDelegate> delegate;
@property (nonatomic, readonly, nullable) NSString *sessionName;
@property (nonatomic, readonly) OpenVPNReachability *vpnReachability;

+ (nullable OpenVPNConfigurationEvaluation *)evaluateConfiguration:(OpenVPNConfiguration *)configuration error:(NSError **)error;

- (nullable OpenVPNConfigurationEvaluation *)applyConfiguration:(OpenVPNConfiguration *)configuration error:(NSError **)error;
- (BOOL)provideCredentials:(OpenVPNCredentials *)credentials error:(NSError **)error;

- (void)connectUsingPacketFlow:(id<OpenVPNAdapterPacketFlow>)packetFlow;
- (void)reconnectAfterTimeInterval:(NSTimeInterval)timeInterval;
- (void)disconnect;

- (void)pauseWithReason:(NSString *)reason;
- (void)resume;

+ (NSString *)copyright;
+ (NSString *)platform;

- (nullable OpenVPNConnectionInfo *)connectionInformation;
- (nullable OpenVPNInterfaceStats *)interfaceStatistics;
- (nullable OpenVPNSessionToken *)sessionToken;
- (nullable OpenVPNTransportStats *)transportStatistics;

@end

NS_ASSUME_NONNULL_END