#import "../include/NSArray+OpenVPNAdditions.h"

@implementation NSArray (OpenVPNEmptyArray)

- (BOOL)ovpn_isNotEmpty {
    return (self.count > 0) ? YES : NO;
}

@end