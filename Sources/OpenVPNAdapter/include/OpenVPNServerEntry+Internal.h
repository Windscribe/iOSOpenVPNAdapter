#import "OpenVPNServerEntry.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNServerEntry (Internal)

- (instancetype)initWithServerEntry:(ClientAPI::ServerEntry)entry;

@end

NS_ASSUME_NONNULL_END