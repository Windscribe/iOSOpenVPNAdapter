#import <Foundation/Foundation.h>

/**
 Transport protocol options
 */
typedef NS_ENUM(NSInteger, OpenVPNTransportProtocol) {
    ///
    OpenVPNTransportProtocolUDP,
    ///
    OpenVPNTransportProtocolTCP,
    ///
    OpenVPNTransportProtocolAdaptive,
    /// Use a transport protocol specified in the profile
    OpenVPNTransportProtocolDefault
};