#import "OpenVPNConfiguration.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNConfiguration (Internal)

@property (readonly) ClientAPI::Config config;

@end

NS_ASSUME_NONNULL_END