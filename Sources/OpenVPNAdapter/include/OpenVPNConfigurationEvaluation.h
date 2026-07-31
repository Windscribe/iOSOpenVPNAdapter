#import <Foundation/Foundation.h>

typedef NS_ENUM(NSInteger, OpenVPNTransportProtocol);
@class OpenVPNServerEntry;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNConfigurationEvaluation : NSObject

@property (nullable, readonly, nonatomic) NSString *userLockedUsername;
@property (nullable, readonly, nonatomic) NSString *profileName;
@property (nullable, readonly, nonatomic) NSString *friendlyName;
@property (readonly, nonatomic) BOOL autologin;
@property (readonly, nonatomic) BOOL externalPKI;
@property (nullable, readonly, nonatomic) NSString *staticChallenge;
@property (readonly, nonatomic) BOOL staticChallengeEcho;
@property (readonly, nonatomic) BOOL privateKeyPasswordRequired;
@property (nullable, readonly, nonatomic) NSString *remoteHost;
@property (readonly, nonatomic) NSUInteger remotePort;
@property (readonly, nonatomic) OpenVPNTransportProtocol remoteProto;
@property (nullable, readonly, nonatomic) NSArray<OpenVPNServerEntry *> *servers;

- (nonnull instancetype) init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END