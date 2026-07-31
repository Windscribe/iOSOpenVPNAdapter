#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, OpenVPNReachabilityStatus) {
    OpenVPNReachabilityStatusNotReachable = 0,
    OpenVPNReachabilityStatusReachableViaWiFi,
    OpenVPNReachabilityStatusReachableViaWWAN
};

typedef void (^OpenVPNReachabilityTrackerHandler)(OpenVPNReachabilityStatus status);

@interface OpenVPNReachability : NSObject

@property (readonly, nonatomic) OpenVPNReachabilityStatus reachabilityStatus;
@property (readonly, nonatomic, getter=isTracking) BOOL tracking;

- (void)startTrackingWithCallback:(nullable OpenVPNReachabilityTrackerHandler)callback;
- (void)stopTracking;

@end

NS_ASSUME_NONNULL_END