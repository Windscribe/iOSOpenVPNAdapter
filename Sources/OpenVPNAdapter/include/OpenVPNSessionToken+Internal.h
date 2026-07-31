#import "OpenVPNSessionToken.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNSessionToken (Internal)

- (instancetype)initWithSessionToken:(ClientAPI::SessionToken)token;

@end

NS_ASSUME_NONNULL_END