#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNSessionToken : NSObject

@property (nonatomic, readonly, nullable) NSString *username;
@property (nonatomic, readonly, nullable) NSString *sessionID;

@end

NS_ASSUME_NONNULL_END