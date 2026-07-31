#import "OpenVPNCredentials.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNCredentials (Internal)

@property (readonly) ClientAPI::ProvideCreds credentials;

@end

NS_ASSUME_NONNULL_END