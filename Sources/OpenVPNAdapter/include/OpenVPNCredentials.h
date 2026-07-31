#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNCredentials : NSObject

@property (nonatomic, nullable) NSString *username;
@property (nonatomic, nullable) NSString *password;
@property (nonatomic, nullable) NSString *response;
@property (nonatomic, nullable) NSString *dynamicChallengeCookie;
@property (nonatomic) BOOL replacePasswordWithSessionID;

@end

NS_ASSUME_NONNULL_END