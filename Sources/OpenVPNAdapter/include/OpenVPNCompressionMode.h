#import <Foundation/Foundation.h>

/**
 Compression mode options
 */
typedef NS_ENUM(NSInteger, OpenVPNCompressionMode) {
    /// Enable compression
    OpenVPNCompressionModeEnabled,
    /// Disable compression
    OpenVPNCompressionModeDisabled,
    /// Allow asymmetric compression
    OpenVPNCompressionModeAsym,
    /// Use compression mode from server
    OpenVPNCompressionModeDefault
};