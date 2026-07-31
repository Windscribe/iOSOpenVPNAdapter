#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface OpenVPNConfiguration : NSObject

@property (nonatomic, nullable) NSData *fileContent;
@property (nonatomic, nullable) NSDictionary<NSString *, NSString *> *settings;
@property (nonatomic) NSInteger connectionTimeout;
@property (nonatomic) BOOL tunPersist;
@property (nonatomic) BOOL disableClientCert;
@property (nonatomic, nullable) NSString *privateKeyPassword;
@property (nonatomic) BOOL forceCiphersuitesAESCBC;
@property (nonatomic) BOOL tlsWrapStrategy;
@property (nonatomic) BOOL disableAltCertChain;
@property (nonatomic, nullable) NSString *guiVersion;
@property (nonatomic, nullable) NSString *ssoMethods;
@property (nonatomic, nullable) NSString *proxyHost;
@property (nonatomic) NSInteger proxyPort;
@property (nonatomic) BOOL proxyAllowCleartextAuth;
@property (nonatomic) BOOL altProxy;
@property (nonatomic) BOOL dco;

@end

NS_ASSUME_NONNULL_END