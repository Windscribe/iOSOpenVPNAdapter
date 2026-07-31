#import <Foundation/Foundation.h>

/**
 Minimum TLS version options
 */
typedef NS_ENUM(NSInteger, OpenVPNMinTLSVersion) {
    /// Disable TLS version negotiation
    OpenVPNMinTLSVersionDisabled,
    /// TLS 1.0
    OpenVPNMinTLSVersion10,
    /// TLS 1.1
    OpenVPNMinTLSVersion11,
    /// TLS 1.2
    OpenVPNMinTLSVersion12,
    /// Use default
    OpenVPNMinTLSVersionDefault
};