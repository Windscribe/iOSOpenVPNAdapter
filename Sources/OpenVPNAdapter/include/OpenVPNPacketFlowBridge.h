#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol OpenVPNAdapterPacketFlow;

@interface OpenVPNPacketFlowBridge: NSObject

@property (nonatomic, weak) id<OpenVPNAdapterPacketFlow> packetFlow;

@property (nonatomic, readonly) CFSocketRef openVPNSocket;
@property (nonatomic, readonly) CFSocketRef packetFlowSocket;

- (BOOL)configureSocketsWithError:(NSError **)error;
- (void)invalidateSocketsIfNeeded;

- (void)startReading;

@end

NS_ASSUME_NONNULL_END