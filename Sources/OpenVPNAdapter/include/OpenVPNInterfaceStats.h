#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNInterfaceStats : NSObject

@property (nonatomic, readonly) long long bytesIn;
@property (nonatomic, readonly) long long bytesOut;
@property (nonatomic, readonly) long long packetsIn;
@property (nonatomic, readonly) long long packetsOut;
@property (nonatomic, readonly) long long errorsIn;
@property (nonatomic, readonly) long long errorsOut;

@end

NS_ASSUME_NONNULL_END