#import <Foundation/Foundation.h>

/**
 TLS certificate profile options
 */
typedef NS_ENUM(NSInteger, OpenVPNTLSCertProfile) {
    /// Legacy certificate profile
    OpenVPNTLSCertProfileLegacy,
    /// Preferred certificate profile
    OpenVPNTLSCertProfilePreferred,
    /// Suite B certificate profile
    OpenVPNTLSCertProfileSuiteB,
    /// Legacy default certificate profile
    OpenVPNTLSCertProfileLegacyDefault,
    /// Preferred default certificate profile
    OpenVPNTLSCertProfilePreferredDefault,
    /// Default certificate profile
    OpenVPNTLSCertProfileDefault
};