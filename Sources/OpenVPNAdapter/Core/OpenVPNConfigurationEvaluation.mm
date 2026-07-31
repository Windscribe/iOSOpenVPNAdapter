#import "OpenVPNConfigurationEvaluation.h"
#import "OpenVPNConfigurationEvaluation+Internal.h"

#include <openvpn/common/number.hpp>

#import "OpenVPNConfiguration.h"
#import "OpenVPNTransportProtocol.h"
#import "OpenVPNServerEntry.h"

using namespace openvpn;

// Forward declarations
@interface OpenVPNConfiguration (Internal)
+ (OpenVPNTransportProtocol)getTransportProtocolFromValue:(NSString *)value;
@end

@interface OpenVPNServerEntry (Internal)
- (instancetype)initWithServerEntry:(ClientAPI::ServerEntry)entry;
@end

@implementation OpenVPNConfigurationEvaluation

- (instancetype)initWithEvalConfig:(ClientAPI::EvalConfig)eval {
    if (self = [super init]) {
        _userLockedUsername = !eval.userlockedUsername.empty() ? [NSString stringWithUTF8String:eval.userlockedUsername.c_str()] : nil;

        _profileName = !eval.profileName.empty() ? [NSString stringWithUTF8String:eval.profileName.c_str()] : nil;
        _friendlyName = !eval.friendlyName.empty() ? [NSString stringWithUTF8String:eval.friendlyName.c_str()] : nil;

        _autologin = eval.autologin;
        _externalPKI = eval.externalPki;

        _staticChallenge = !eval.staticChallenge.empty() ? [NSString stringWithUTF8String:eval.staticChallenge.c_str()] : nil;
        _staticChallengeEcho = eval.staticChallengeEcho;

        _privateKeyPasswordRequired = eval.privateKeyPasswordRequired;
        // allowPasswordSave doesn't exist in OpenVPN3

        _remoteHost = !eval.remoteHost.empty() ? [NSString stringWithUTF8String:eval.remoteHost.c_str()] : nil;

        uint16_t port = 0;
        parse_number(eval.remotePort, port);

        _remotePort = port;

        NSString *currentProto = [[[NSString stringWithUTF8String:eval.remoteProto.c_str()] componentsSeparatedByString:@"-"] firstObject];
        _remoteProto = [OpenVPNConfiguration getTransportProtocolFromValue:currentProto];

        _servers = nil;

        if (!eval.serverList.empty()) {
            NSMutableArray *servers = [NSMutableArray new];

            for (ClientAPI::ServerEntry entry : eval.serverList) {
                OpenVPNServerEntry *serverEntry = [[OpenVPNServerEntry alloc] initWithServerEntry:entry];
                [servers addObject:serverEntry];
            }

            _servers = servers;
        }
    }
    return self;
}

@end