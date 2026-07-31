#import <Foundation/Foundation.h>

@interface OpenVPNServerEntry : NSObject

@property (nullable, readonly, nonatomic) NSString *server;
@property (nullable, readonly, nonatomic) NSString *friendlyName;

- (nonnull instancetype) init NS_UNAVAILABLE;

@end