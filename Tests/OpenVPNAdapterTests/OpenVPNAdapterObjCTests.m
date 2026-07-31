#import <XCTest/XCTest.h>
#import "../../Sources/OpenVPNAdapter/include/OpenVPNAdapter.h"
#import "../../Sources/OpenVPNAdapter/include/OpenVPNConfiguration.h"
#import "../../Sources/OpenVPNAdapter/include/OpenVPNCredentials.h"
#import "../../Sources/OpenVPNAdapter/include/OpenVPNLogger.h"
#import "../../Sources/OpenVPNAdapter/include/OpenVPNNetworkSettingsBuilder.h"
#import "../../Sources/OpenVPNAdapter/include/OpenVPNPacketFlowBridge.h"

@interface OpenVPNAdapterObjCTests : XCTestCase
@end

@implementation OpenVPNAdapterObjCTests

- (void)testAdapterInitialization {
    OpenVPNAdapter *adapter = [[OpenVPNAdapter alloc] init];
    XCTAssertNotNil(adapter);
}

- (void)testStaticMethods {
    NSString *copyright = [OpenVPNAdapter copyright];
    XCTAssertNotNil(copyright);
    XCTAssertTrue(copyright.length > 0);

    NSString *platform = [OpenVPNAdapter platform];
    XCTAssertNotNil(platform);
    XCTAssertTrue(platform.length > 0);
}

- (void)testConfigurationCreation {
    OpenVPNConfiguration *config = [[OpenVPNConfiguration alloc] init];
    XCTAssertNotNil(config);

    NSString *testConfigString = @"client\ndev tun\nproto udp\nremote test.example.com 1194\n";
    config.fileContent = [testConfigString dataUsingEncoding:NSUTF8StringEncoding];
    XCTAssertNotNil(config.fileContent);
}

- (void)testCredentialsCreation {
    OpenVPNCredentials *credentials = [[OpenVPNCredentials alloc] init];
    credentials.username = @"testuser";
    credentials.password = @"testpass";

    XCTAssertEqualObjects(credentials.username, @"testuser");
    XCTAssertEqualObjects(credentials.password, @"testpass");
}

- (void)testLoggerSingleton {
    OpenVPNLogger *logger = [OpenVPNLogger sharedLogger];
    XCTAssertNotNil(logger);

    logger.minimumLevel = OpenVPNLogLevelDebug;
    XCTAssertEqual(logger.minimumLevel, OpenVPNLogLevelDebug);
}

- (void)testPacketFlowBridge {
    OpenVPNPacketFlowBridge *bridge = [[OpenVPNPacketFlowBridge alloc] init];
    XCTAssertNotNil(bridge);
}

- (void)testNetworkSettingsBuilder {
    OpenVPNNetworkSettingsBuilder *builder = [[OpenVPNNetworkSettingsBuilder alloc] init];
    XCTAssertNotNil(builder);

    [builder.dnsServers addObject:@"8.8.8.8"];
    XCTAssertEqual(builder.dnsServers.count, 1);
    XCTAssertEqualObjects(builder.dnsServers[0], @"8.8.8.8");
}

@end