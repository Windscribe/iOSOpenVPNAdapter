#import "OpenVPNTransportStats.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNTransportStats (Internal)

- (instancetype)initWithTransportStats:(ClientAPI::TransportStats)stats;

@end

NS_ASSUME_NONNULL_END