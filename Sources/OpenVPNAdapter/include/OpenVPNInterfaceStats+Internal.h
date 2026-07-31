#import "OpenVPNInterfaceStats.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNInterfaceStats (Internal)

- (instancetype)initWithInterfaceStats:(ClientAPI::InterfaceStats)stats;

@end

NS_ASSUME_NONNULL_END