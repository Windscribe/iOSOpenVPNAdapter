#define OPENVPN_EXTERN extern

#define TUNNEL_CONFIGURATION_TIMEOUT 30

#import "../include/OpenVPNAdapter.h"

#import <NetworkExtension/NetworkExtension.h>

#import "../include/OpenVPNClient.h"
#import "../include/OpenVPNError.h"
#import "../include/OpenVPNAdapterEvent.h"
#import "../include/OpenVPNPacketFlowBridge.h"
#import "../include/OpenVPNNetworkSettingsBuilder.h"
#import "../include/OpenVPNAdapterPacketFlow.h"
#import "../include/OpenVPNReachability.h"
#import "../include/OpenVPNCredentials+Internal.h"
#import "../include/OpenVPNConfiguration+Internal.h"
#import "../include/OpenVPNConnectionInfo+Internal.h"
#import "../include/OpenVPNInterfaceStats+Internal.h"
#import "../include/OpenVPNConfigurationEvaluation+Internal.h"
#import "../include/OpenVPNSessionToken+Internal.h"
#import "../include/OpenVPNTransportStats+Internal.h"
#import "../include/NSError+OpenVPNError.h"

@interface OpenVPNAdapter () <OpenVPNClientDelegate>

@property (nonatomic) OpenVPNClient *vpnClient;

@property (nonatomic) OpenVPNPacketFlowBridge *packetFlowBridge;
@property (nonatomic) OpenVPNNetworkSettingsBuilder *networkSettingsBuilder;
@property (nonatomic, readwrite) OpenVPNReachability *vpnReachability;

@end

@implementation OpenVPNAdapter

- (instancetype)init {
    if (self = [super init]) {
        _vpnClient = new OpenVPNClient(self);
        _packetFlowBridge = [[OpenVPNPacketFlowBridge alloc] init];
        _vpnReachability = [[OpenVPNReachability alloc] init];
    }
    return self;
}

#pragma mark - OpenVPNClient Lifecycle

+ (nullable OpenVPNConfigurationEvaluation *)evaluateConfiguration:(OpenVPNConfiguration *)configuration error:(NSError **)error {
    // Need to create a temporary client for evaluation
    // Create a dummy adapter just to satisfy the non-null requirement
    OpenVPNAdapter *tempAdapter = [[OpenVPNAdapter alloc] init];
    OpenVPNClient *client = new OpenVPNClient(tempAdapter);
    ClientAPI::EvalConfig eval = client->eval_config(configuration.config);
    delete client;

    if (eval.error) {
        if (error) {
            NSString *message = [NSString stringWithUTF8String:eval.message.c_str()];
            *error = [NSError ovpn_errorObjectForAdapterError:OpenVPNAdapterErrorConfigurationFailure
                                                  description:@"Failed to evaluate OpenVPN configuration."
                                                      message:message
                                                        fatal:YES];
        }

        return nil;
    }

    return [[OpenVPNConfigurationEvaluation alloc] initWithEvalConfig:eval];
}

- (OpenVPNConfigurationEvaluation *)applyConfiguration:(OpenVPNConfiguration *)configuration error:(NSError * __autoreleasing *)error {
    [self clientLogMessage:@"Applying OpenVPN configuration..."];

    ClientAPI::EvalConfig eval = self.vpnClient->apply_config(configuration.config);

    if (eval.error) {
        NSString *message = [NSString stringWithUTF8String:eval.message.c_str()];
        NSString *errorLog = [NSString stringWithFormat:@"Configuration apply failed: %@", message];
        [self clientLogMessage:errorLog];

        if (error) {
            *error = [NSError ovpn_errorObjectForAdapterError:OpenVPNAdapterErrorConfigurationFailure
                                                  description:@"Failed to apply OpenVPN configuration."
                                                      message:message
                                                        fatal:YES];
        }

        return nil;
    }

    [self clientLogMessage:@"Configuration applied successfully"];
    return [[OpenVPNConfigurationEvaluation alloc] initWithEvalConfig:eval];
}

- (BOOL)provideCredentials:(OpenVPNCredentials *)credentials error:(NSError * __autoreleasing *)error {
    [self clientLogMessage:@"Providing OpenVPN credentials..."];

    ClientAPI::Status status = self.vpnClient->provide_creds(credentials.credentials);

    if (status.error) {
        NSString *message = [NSString stringWithUTF8String:status.message.c_str()];
        NSString *errorLog = [NSString stringWithFormat:@"Credentials provision failed: %@", message];
        [self clientLogMessage:errorLog];

        if (error) {
            *error = [NSError ovpn_errorObjectForAdapterError:OpenVPNAdapterErrorCredentialsFailure
                                          description:@"Failed to provide OpenVPN credentials."
                                              message:message
                                                fatal:YES];
        }

        return NO;
    }

    [self clientLogMessage:@"Credentials provided successfully"];
    return YES;
}

- (void)connectUsingPacketFlow:(id<OpenVPNAdapterPacketFlow>)packetFlow {
    NSAssert(self.delegate != nil, @"delegate property shouldn't be nil, set it before trying to establish connection.");

    self.packetFlowBridge.packetFlow = packetFlow;

    dispatch_queue_t connectQueue = dispatch_queue_create("me.ss-abramchuk.openvpn-adapter.connection.", DISPATCH_QUEUE_SERIAL);

    dispatch_async(connectQueue, ^{
        [self clientLogMessage:@"Starting connection..."];
        ClientAPI::Status status = self.vpnClient->connect();

        NSString *statusLog = [NSString stringWithFormat:@"Connection status - error: %s, status: '%s', message: '%s'",
                               status.error ? "YES" : "NO",
                               status.status.c_str(),
                               status.message.c_str()];
        [self clientLogMessage:statusLog];

        [self handleConnectionStatus:status];
    });
}

- (void)reconnectAfterTimeInterval:(NSTimeInterval)timeInterval {
    self.vpnClient->reconnect(timeInterval);
}

- (void)disconnect {
    self.vpnClient->stop();
}

- (void)pauseWithReason:(NSString *)reason {
    self.vpnClient->pause(std::string(reason.UTF8String));
}

- (void)resume {
    self.vpnClient->resume();
}

- (void)handleConnectionStatus:(ClientAPI::Status)status {
    if (!status.error) {
        [self clientLogMessage:@"Connection initiated successfully"];
        return;
    }

    NSString *statusString = !status.status.empty() ?
        [NSString stringWithUTF8String:status.status.c_str()] : @"UNKNOWN";

    OpenVPNAdapterError adapterError = !status.status.empty() ?
        [NSError ovpn_adapterErrorByName:statusString] :
        OpenVPNAdapterErrorUnknown;

    NSString *message = [NSString stringWithUTF8String:status.message.c_str()];

    NSString *errorLog = [NSString stringWithFormat:@"Connection failed with status '%@': %@",
                          statusString, message];
    [self clientLogMessage:errorLog];

    NSError *error = [NSError ovpn_errorObjectForAdapterError:adapterError
                                          description:@"Failed to establish connection with OpenVPN server."
                                              message:message
                                                fatal:YES];

    [self.delegate openVPNAdapter:self handleError:error];
}

#pragma mark - OpenVPNClient Information

+ (NSString *)copyright {
    return [NSString stringWithUTF8String:ClientAPI::OpenVPNClientHelper::copyright().c_str()];
}

+ (NSString *)platform {
    return [NSString stringWithUTF8String:ClientAPI::OpenVPNClientHelper::platform().c_str()];
}

- (OpenVPNConnectionInfo *)connectionInformation {
    ClientAPI::ConnectionInfo information = self.vpnClient->connection_info();
    return information.defined ? [[OpenVPNConnectionInfo alloc] initWithConnectionInfo:information] : nil;
}

- (OpenVPNInterfaceStats *)interfaceStatistics {
    return [[OpenVPNInterfaceStats alloc] initWithInterfaceStats:self.vpnClient->tun_stats()];
}

- (OpenVPNSessionToken *)sessionToken {
    ClientAPI::SessionToken token;
    return self.vpnClient->session_token(token) ? [[OpenVPNSessionToken alloc] initWithSessionToken:token] : nil;
}

- (OpenVPNTransportStats *)transportStatistics {
    return [[OpenVPNTransportStats alloc] initWithTransportStats:self.vpnClient->transport_stats()];
}

#pragma mark - Lazy Initialization

- (OpenVPNNetworkSettingsBuilder *)networkSettingsBuilder {
    if (!_networkSettingsBuilder) { _networkSettingsBuilder = [[OpenVPNNetworkSettingsBuilder alloc] init]; }
    return _networkSettingsBuilder;
}

#pragma mark - OpenVPNClientDelegate

- (BOOL)setRemoteAddress:(NSString *)address {
    self.networkSettingsBuilder.remoteAddress = address;
    return YES;
}

- (BOOL)addIPV4Address:(NSString *)address subnetMask:(NSString *)subnetMask gateway:(NSString *)gateway {
    self.networkSettingsBuilder.ipv4DefaultGateway = gateway;

    [self.networkSettingsBuilder.ipv4LocalAddresses addObject:address];
    [self.networkSettingsBuilder.ipv4SubnetMasks addObject:subnetMask];

    return YES;
}

- (BOOL)addIPV6Address:(NSString *)address prefixLength:(NSNumber *)prefixLength gateway:(NSString *)gateway {
    self.networkSettingsBuilder.ipv6DefaultGateway = gateway;

    [self.networkSettingsBuilder.ipv6LocalAddresses addObject:address];
    [self.networkSettingsBuilder.ipv6NetworkPrefixLengths addObject:prefixLength];

    return YES;
}

- (BOOL)addIPV4Route:(NEIPv4Route *)route {
    route.gatewayAddress = self.networkSettingsBuilder.ipv4DefaultGateway;

    NSUInteger index = [self.networkSettingsBuilder.ipv4IncludedRoutes indexOfObjectPassingTest:^BOOL(NEIPv4Route *obj, NSUInteger idx, BOOL *stop) {
        return [obj.destinationAddress isEqualToString:route.destinationAddress] &&
            [obj.destinationSubnetMask isEqualToString:route.destinationSubnetMask];
    }];

    if (index == NSNotFound) {
        [self.networkSettingsBuilder.ipv4IncludedRoutes addObject:route];
        return YES;
    } else {
        return NO;
    }
}

- (BOOL)addIPV6Route:(NEIPv6Route *)route {
    route.gatewayAddress = self.networkSettingsBuilder.ipv6DefaultGateway;

    NSUInteger index = [self.networkSettingsBuilder.ipv6IncludedRoutes indexOfObjectPassingTest:^BOOL(NEIPv6Route *obj, NSUInteger idx, BOOL *stop) {
        return [obj.destinationAddress isEqualToString:route.destinationAddress] &&
            obj.destinationNetworkPrefixLength == route.destinationNetworkPrefixLength;
    }];

    if (index == NSNotFound) {
        [self.networkSettingsBuilder.ipv6IncludedRoutes addObject:route];
        return YES;
    } else {
        return NO;
    }
}

- (BOOL)excludeIPV4Route:(NEIPv4Route *)route {
    NSUInteger index = [self.networkSettingsBuilder.ipv4ExcludedRoutes indexOfObjectPassingTest:^BOOL(NEIPv4Route *obj, NSUInteger idx, BOOL *stop) {
        return [obj.destinationAddress isEqualToString:route.destinationAddress] &&
        [obj.destinationSubnetMask isEqualToString:route.destinationSubnetMask];
    }];

    if (index == NSNotFound) {
        [self.networkSettingsBuilder.ipv4ExcludedRoutes addObject:route];
        return YES;
    } else {
        return NO;
    }
}

- (BOOL)excludeIPV6Route:(NEIPv6Route *)route {
    NSUInteger index = [self.networkSettingsBuilder.ipv6ExcludedRoutes indexOfObjectPassingTest:^BOOL(NEIPv6Route *obj, NSUInteger idx, BOOL *stop) {
        return [obj.destinationAddress isEqualToString:route.destinationAddress] &&
        obj.destinationNetworkPrefixLength == route.destinationNetworkPrefixLength;
    }];

    if (index == NSNotFound) {
        [self.networkSettingsBuilder.ipv6ExcludedRoutes addObject:route];
        return YES;
    } else {
        return NO;
    }
}

- (BOOL)addDNS:(NSString *)dns {
    if ([self.networkSettingsBuilder.dnsServers containsObject:dns]) {
        return NO;
    }

    [self.networkSettingsBuilder.dnsServers addObject:dns];
    return YES;
}

- (BOOL)addSearchDomain:(NSString *)domain {
    if ([self.networkSettingsBuilder.searchDomains containsObject:domain]) {
        return NO;
    }

    [self.networkSettingsBuilder.searchDomains addObject:domain];
    return YES;
}

- (BOOL)setMTU:(NSNumber *)mtu {
    self.networkSettingsBuilder.mtu = mtu;
    return YES;
}

- (BOOL)setSessionName:(NSString *)name {
    _sessionName = name;
    return YES;
}

- (BOOL)addProxyBypassHost:(NSString *)bypassHost {
    if ([self.networkSettingsBuilder.proxyExceptionList containsObject:bypassHost]) {
        return NO;
    }

    [self.networkSettingsBuilder.proxyExceptionList addObject:bypassHost];
    return YES;
}

- (BOOL)setProxyAutoConfigurationURL:(NSURL *)url {
    self.networkSettingsBuilder.autoProxyConfigurationEnabled = YES;
    self.networkSettingsBuilder.proxyAutoConfigurationURL = url;

    return YES;
}

- (BOOL)setProxyServer:(NEProxyServer *)server protocol:(OpenVPNProxyServerProtocol)protocol {
    switch (protocol) {
        case OpenVPNProxyServerProtocolHTTP:
            self.networkSettingsBuilder.httpProxyServerEnabled = YES;
            self.networkSettingsBuilder.httpProxyServer = server;
            break;

        case OpenVPNProxyServerProtocolHTTPS:
            self.networkSettingsBuilder.httpsProxyServerEnabled = YES;
            self.networkSettingsBuilder.httpsProxyServer = server;
            break;
    }

    return YES;
}

- (BOOL)establishTunnel {
    NEPacketTunnelNetworkSettings *networkSettings = [self.networkSettingsBuilder networkSettings];

    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    __block NSError *configurationError;
    void (^completionHandler)(NSError *error) = ^(NSError *error) {
        configurationError = error;
        dispatch_semaphore_signal(semaphore);
    };

    [self.delegate openVPNAdapter:self configureTunnelWithNetworkSettings:networkSettings completionHandler:completionHandler];

    long timeout = dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, TUNNEL_CONFIGURATION_TIMEOUT * NSEC_PER_SEC));
    if (timeout) { return NO; }

    if (configurationError) {
        NSDictionary *userInfo = @{
            NSLocalizedDescriptionKey: @"Failed to configure tunnel using provided settings. Check underlying error for more details.",
            NSUnderlyingErrorKey: configurationError,
            OpenVPNAdapterErrorFatalKey: @(YES)
        };

        NSError *error = [NSError errorWithDomain:OpenVPNAdapterErrorDomain
                                             code:OpenVPNAdapterErrorTUNSetupFailed
                                         userInfo:userInfo];

        [self.delegate openVPNAdapter:self handleError:error];

        return NO;
    }

    NSError *socketError;
    if ([self.packetFlowBridge configureSocketsWithError:&socketError]) {
        [self.packetFlowBridge startReading];
        return YES;
    } else {
        if (socketError) { [self.delegate openVPNAdapter:self handleError:socketError]; }
        return NO;
    }
}

- (CFSocketNativeHandle)socketHandle {
    return CFSocketGetNative(self.packetFlowBridge.openVPNSocket);
}

- (void)clientEventName:(NSString *)eventName message:(NSString *)message {
    NSDictionary *events = @{
        @"DISCONNECTED": @(OpenVPNAdapterEventDisconnected),
        @"CONNECTED": @(OpenVPNAdapterEventConnected),
        @"RECONNECTING": @(OpenVPNAdapterEventReconnecting),
        @"AUTH_PENDING": @(OpenVPNAdapterEventAuthPending),
        @"RESOLVE": @(OpenVPNAdapterEventResolve),
        @"WAIT": @(OpenVPNAdapterEventWait),
        @"WAIT_PROXY": @(OpenVPNAdapterEventWaitProxy),
        @"CONNECTING": @(OpenVPNAdapterEventConnecting),
        @"GET_CONFIG": @(OpenVPNAdapterEventGetConfig),
        @"ASSIGN_IP": @(OpenVPNAdapterEventAssignIP),
        @"ADD_ROUTES": @(OpenVPNAdapterEventAddRoutes),
        @"ECHO": @(OpenVPNAdapterEventEcho),
        @"INFO": @(OpenVPNAdapterEventInfo),
        @"WARN": @(OpenVPNAdapterEventWarn),
        @"PAUSE": @(OpenVPNAdapterEventPause),
        @"RESUME": @(OpenVPNAdapterEventResume),
        @"RELAY": @(OpenVPNAdapterEventRelay),
        @"COMPRESSION_ENABLED": @(OpenVPNAdapterEventCompressionEnabled),
        @"UNSUPPORTED_FEATURE": @(OpenVPNAdapterEventUnsupportedFeature)
    };

    OpenVPNAdapterEvent event = events[eventName] != nil ?
        (OpenVPNAdapterEvent)[events[eventName] integerValue] : OpenVPNAdapterEventUnknown;

    [self.delegate openVPNAdapter:self handleEvent:event message:message];
}

- (void)clientErrorName:(NSString *)errorName fatal:(BOOL)fatal message:(NSString *)message {
    OpenVPNAdapterError adapterError = [NSError ovpn_adapterErrorByName:errorName];
    NSString *description = fatal ? @"OpenVPN fatal error occured" : @"OpenVPN error occured";

    NSError *error = [NSError ovpn_errorObjectForAdapterError:adapterError
                                                  description:description
                                                      message:message
                                                        fatal:fatal];

    [self.delegate openVPNAdapter:self handleError:error];
}

- (void)clientLogMessage:(NSString *)logMessage {
    if ([self.delegate respondsToSelector:@selector(openVPNAdapter:handleLogMessage:)]) {
        [self.delegate openVPNAdapter:self handleLogMessage:logMessage];
    }
}

- (void)tick {
    if ([self.delegate respondsToSelector:@selector(openVPNAdapterDidReceiveClockTick:)]) {
        [self.delegate openVPNAdapterDidReceiveClockTick:self];
    }
}

- (void)resetSettings {
    _sessionName = nil;
    _networkSettingsBuilder = nil;
}

- (void)resetTun {
    [_packetFlowBridge invalidateSocketsIfNeeded];

    dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);

    __block NSError *configurationError;
    void (^completionHandler)(NSError *error) = ^(NSError *error) {
        configurationError = error;
        dispatch_semaphore_signal(semaphore);
    };

    [self.delegate openVPNAdapter:self configureTunnelWithNetworkSettings:nil completionHandler:completionHandler];

    dispatch_semaphore_wait(semaphore, dispatch_time(DISPATCH_TIME_NOW, TUNNEL_CONFIGURATION_TIMEOUT * NSEC_PER_SEC));

    if (configurationError) {
        NSDictionary *userInfo = @{
            NSLocalizedDescriptionKey: @"Failed to reset tunnel. Check underlying error for more details.",
            NSUnderlyingErrorKey: configurationError,
            OpenVPNAdapterErrorFatalKey: @(YES)
        };

        NSError *error = [NSError errorWithDomain:OpenVPNAdapterErrorDomain
                                             code:OpenVPNAdapterErrorTUNSetupFailed
                                         userInfo:userInfo];

        [self.delegate openVPNAdapter:self handleError:error];
    }
}

#pragma mark -

- (void)dealloc {
    delete _vpnClient;
}

@end