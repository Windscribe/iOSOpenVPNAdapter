#import "../include/OpenVPNReachability.h"
#import <SystemConfiguration/SystemConfiguration.h>
#import <netinet/in.h>

@interface OpenVPNReachability ()

@property (nonatomic) SCNetworkReachabilityRef reachabilityRef;
@property (nonatomic, copy) OpenVPNReachabilityTrackerHandler callback;
@property (readwrite, nonatomic) OpenVPNReachabilityStatus reachabilityStatus;
@property (readwrite, nonatomic, getter=isTracking) BOOL tracking;

@end

@implementation OpenVPNReachability

- (instancetype)init {
    self = [super init];
    if (self) {
        _reachabilityStatus = OpenVPNReachabilityStatusNotReachable;
        _tracking = NO;

        // Create zero address to monitor general network connectivity
        struct sockaddr_in zeroAddress;
        bzero(&zeroAddress, sizeof(zeroAddress));
        zeroAddress.sin_len = sizeof(zeroAddress);
        zeroAddress.sin_family = AF_INET;

        _reachabilityRef = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, (const struct sockaddr *)&zeroAddress);
    }
    return self;
}

- (void)dealloc {
    [self stopTracking];
    if (_reachabilityRef) {
        CFRelease(_reachabilityRef);
    }
}

- (void)startTrackingWithCallback:(OpenVPNReachabilityTrackerHandler)callback {
    if (self.tracking) {
        return;
    }

    self.callback = callback;
    self.tracking = YES;

    if (!self.reachabilityRef) {
        return;
    }

    // Set up the callback
    SCNetworkReachabilityContext context = {0, (__bridge void *)self, NULL, NULL, NULL};

    if (SCNetworkReachabilitySetCallback(self.reachabilityRef, ReachabilityCallback, &context)) {
        if (SCNetworkReachabilityScheduleWithRunLoop(self.reachabilityRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
            // Update status immediately
            [self updateReachabilityStatus];
        }
    }
}

- (void)stopTracking {
    if (!self.tracking) {
        return;
    }

    self.tracking = NO;
    self.callback = nil;

    if (self.reachabilityRef) {
        SCNetworkReachabilityUnscheduleFromRunLoop(self.reachabilityRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }
}

- (void)updateReachabilityStatus {
    if (!self.reachabilityRef) {
        return;
    }

    SCNetworkReachabilityFlags flags;
    if (SCNetworkReachabilityGetFlags(self.reachabilityRef, &flags)) {
        self.reachabilityStatus = [self reachabilityStatusForFlags:flags];

        if (self.callback) {
            self.callback(self.reachabilityStatus);
        }
    }
}

- (OpenVPNReachabilityStatus)reachabilityStatusForFlags:(SCNetworkReachabilityFlags)flags {
    if ((flags & kSCNetworkReachabilityFlagsReachable) == 0) {
        // The target host is not reachable.
        return OpenVPNReachabilityStatusNotReachable;
    }

    OpenVPNReachabilityStatus status = OpenVPNReachabilityStatusNotReachable;

    if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0) {
        // If the target host is reachable and no connection is required then we'll assume it's Wi-Fi
        status = OpenVPNReachabilityStatusReachableViaWiFi;
    }

    if ((flags & kSCNetworkReachabilityFlagsConnectionOnDemand) != 0 ||
        (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0) {
        // The connection is on-demand (or on-traffic) if the calling application is using the CFSocketStream or higher APIs
        if ((flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0) {
            // On-demand and no user intervention needed, so it's Wi-Fi
            status = OpenVPNReachabilityStatusReachableViaWiFi;
        }
    }

#if TARGET_OS_IPHONE
    if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN) {
        // WWAN connections are cellular
        status = OpenVPNReachabilityStatusReachableViaWWAN;
    }
#endif

    return status;
}

static void ReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info) {
    OpenVPNReachability *reachability = (__bridge OpenVPNReachability *)info;
    [reachability updateReachabilityStatus];
}

@end