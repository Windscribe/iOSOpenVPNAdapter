//  Unified logging system implementation
//

#import "OpenVPNLogger.h"
#include <stdarg.h>

@implementation OpenVPNLogger

+ (instancetype)sharedLogger {
    static OpenVPNLogger *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[OpenVPNLogger alloc] init];
        sharedInstance.enabled = NO;  // Disabled by default
        sharedInstance.minimumLevel = OpenVPNLogLevelInfo;
    });
    return sharedInstance;
}

- (void)logMessage:(NSString *)message level:(OpenVPNLogLevel)level {
    // Check if logging is enabled and level is appropriate
    if (!self.enabled || level < self.minimumLevel) {
        return;
    }

    // Send to receiver (usually packet tunnel provider)
    if (self.receiver) {
        [self.receiver handleLogMessage:message level:level];
    }
}

- (void)logVerbose:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *message = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self logMessage:message level:OpenVPNLogLevelVerbose];
}

- (void)logDebug:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *message = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self logMessage:message level:OpenVPNLogLevelDebug];
}

- (void)logInfo:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *message = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self logMessage:message level:OpenVPNLogLevelInfo];
}

- (void)logWarning:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *message = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self logMessage:message level:OpenVPNLogLevelWarning];
}

- (void)logError:(NSString *)format, ... {
    va_list args;
    va_start(args, format);
    NSString *message = [[NSString alloc] initWithFormat:format arguments:args];
    va_end(args);
    [self logMessage:message level:OpenVPNLogLevelError];
}

@end

// C++ Bridge Functions
void OpenVPNLoggerLog(const char *message, int level) {
    @autoreleasepool {
        NSString *msg = [NSString stringWithUTF8String:message];
        [[OpenVPNLogger sharedLogger] logMessage:msg level:(OpenVPNLogLevel)level];
    }
}

void OpenVPNLoggerLogVerbose(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OpenVPNLoggerLog(buffer, OpenVPNLogLevelVerbose);
}

void OpenVPNLoggerLogDebug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OpenVPNLoggerLog(buffer, OpenVPNLogLevelDebug);
}

void OpenVPNLoggerLogInfo(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OpenVPNLoggerLog(buffer, OpenVPNLogLevelInfo);
}

void OpenVPNLoggerLogWarning(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OpenVPNLoggerLog(buffer, OpenVPNLogLevelWarning);
}

void OpenVPNLoggerLogError(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    OpenVPNLoggerLog(buffer, OpenVPNLogLevelError);
}