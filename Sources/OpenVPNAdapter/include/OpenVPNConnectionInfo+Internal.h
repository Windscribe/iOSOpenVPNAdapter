#import "OpenVPNConnectionInfo.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNConnectionInfo (Internal)

- (instancetype)initWithConnectionInfo:(ClientAPI::ConnectionInfo)info;

@end

NS_ASSUME_NONNULL_END