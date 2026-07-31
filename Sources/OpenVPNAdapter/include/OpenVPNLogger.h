//  Unified logging system for OpenVPN adapter
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// Log levels matching OpenVPN3's levels
typedef NS_ENUM(NSInteger, OpenVPNLogLevel) {
    OpenVPNLogLevelVerbose = 0,  // Most detailed logging
    OpenVPNLogLevelDebug = 1,
    OpenVPNLogLevelInfo = 2,
    OpenVPNLogLevelWarning = 3,
    OpenVPNLogLevelError = 4,
    OpenVPNLogLevelNone = 5      // No logging
};

// Protocol for log receivers
@protocol OpenVPNLogReceiver <NSObject>
- (void)handleLogMessage:(NSString *)message level:(OpenVPNLogLevel)level;
@end

// Unified logger singleton
@interface OpenVPNLogger : NSObject

// Singleton instance
+ (instancetype)sharedLogger;

// Configuration
@property (nonatomic, assign) BOOL enabled;
@property (nonatomic, assign) OpenVPNLogLevel minimumLevel;
@property (nonatomic, weak, nullable) id<OpenVPNLogReceiver> receiver;

// Logging methods
- (void)logMessage:(NSString *)message level:(OpenVPNLogLevel)level;
- (void)logVerbose:(NSString *)format, ... NS_FORMAT_FUNCTION(1,2);
- (void)logDebug:(NSString *)format, ... NS_FORMAT_FUNCTION(1,2);
- (void)logInfo:(NSString *)format, ... NS_FORMAT_FUNCTION(1,2);
- (void)logWarning:(NSString *)format, ... NS_FORMAT_FUNCTION(1,2);
- (void)logError:(NSString *)format, ... NS_FORMAT_FUNCTION(1,2);

// C++ bridge functions (for OpenVPN3 core integration)
void OpenVPNLoggerLog(const char *message, int level);
void OpenVPNLoggerLogVerbose(const char *format, ...);
void OpenVPNLoggerLogDebug(const char *format, ...);
void OpenVPNLoggerLogInfo(const char *format, ...);
void OpenVPNLoggerLogWarning(const char *format, ...);
void OpenVPNLoggerLogError(const char *format, ...);

@end

NS_ASSUME_NONNULL_END