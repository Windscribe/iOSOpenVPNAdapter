#import "OpenVPNConfigurationEvaluation.h"

#ifndef OVPNCLI_HPP_INCLUDED
#define OVPNCLI_HPP_INCLUDED
#include <client/ovpncli.hpp>
#endif

using namespace openvpn;

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNConfigurationEvaluation (Internal)

- (instancetype)initWithEvalConfig:(ClientAPI::EvalConfig)eval;

@end

NS_ASSUME_NONNULL_END