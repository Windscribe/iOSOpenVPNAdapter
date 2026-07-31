#import "OpenVPNCredentials.h"
#import "OpenVPNCredentials+Internal.h"

using namespace openvpn;

@interface OpenVPNCredentials () {
    ClientAPI::ProvideCreds _credentials;
}

@end

@implementation OpenVPNCredentials (Internal)

- (ClientAPI::ProvideCreds)credentials {
    return _credentials;
}

@end

@implementation OpenVPNCredentials

- (NSString *)username {
    return !_credentials.username.empty() ? [NSString stringWithUTF8String:_credentials.username.c_str()] : nil;
}

- (void)setUsername:(NSString *)username {
    _credentials.username = username ? std::string([username UTF8String]) : "";
}

- (NSString *)password {
    return !_credentials.password.empty() ? [NSString stringWithUTF8String:_credentials.password.c_str()] : nil;
}

- (void)setPassword:(NSString *)password {
    _credentials.password = password ? std::string([password UTF8String]) : "";
}

- (NSString *)response {
    return !_credentials.response.empty() ? [NSString stringWithUTF8String:_credentials.response.c_str()] : nil;
}

- (void)setResponse:(NSString *)response {
    _credentials.response = response ? std::string([response UTF8String]) : "";
}

- (NSString *)dynamicChallengeCookie {
    // dynamicChallengeCookie doesn't exist in newer OpenVPN3
    return nil;
}

- (void)setDynamicChallengeCookie:(NSString *)dynamicChallengeCookie {
    // dynamicChallengeCookie doesn't exist in newer OpenVPN3
}

- (BOOL)replacePasswordWithSessionID {
    // replacePasswordWithSessionID doesn't exist in newer OpenVPN3
    return NO;
}

- (void)setReplacePasswordWithSessionID:(BOOL)replacePasswordWithSessionID {
    // replacePasswordWithSessionID doesn't exist in newer OpenVPN3
}

- (BOOL)cachePassword {
    // cachePassword doesn't exist in newer OpenVPN3
    return NO;
}

- (void)setCachePassword:(BOOL)cachePassword {
    // cachePassword doesn't exist in newer OpenVPN3
}

@end