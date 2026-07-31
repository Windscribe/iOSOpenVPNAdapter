#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@protocol OpenVPNAdapterPacketFlow <NSObject>

- (void)readPacketsWithCompletionHandler:(void (^)(NSArray<NSData *> * _Nonnull packets, NSArray<NSNumber *> * _Nonnull protocols))completionHandler;
- (BOOL)writePackets:(NSArray<NSData *> *)packets withProtocols:(NSArray<NSNumber *> *)protocols;

@end

NS_ASSUME_NONNULL_END