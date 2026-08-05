// swift-tools-version: 5.10
import PackageDescription

let package = Package(
    name: "iOSOpenVPNAdapter",
    platforms: [
        .iOS(.v14),
        .tvOS(.v14)
    ],
    products: [
        .library(
            name: "OpenVPNAdapter",
            targets: ["OpenVPNAdapter"]
        ),
    ],
    dependencies: [],
    targets: [
        .target(
            name: "OpenVPNCore",
            dependencies: [],
            path: "Sources/OpenVPNCore",
            exclude: [
                "OpenVPN3/test",
                "OpenVPN3/javacli",
                "OpenVPN3/deps",
                "OpenVPN3/scripts",
                "OpenVPN3/doc",
                "OpenVPN3/openvpn/ovpnagent",
                "OpenVPN3/openvpn/omi",
                "mbedTLS/tests",
                "mbedTLS/programs",
                "mbedTLS/docs",
                "mbedTLS/scripts",
                "mbedTLS/doxygen",
                "mbedTLS/visualc",
                "mbedTLS/.github",
                "mbedTLS/library/Makefile",
                "mbedTLS/library/CMakeLists.txt",
                "LZ4/tests",
                "LZ4/programs",
                "LZ4/examples",
                "LZ4/doc",
                "LZ4/.circleci"
            ],
            sources: [
                "OpenVPN3/client/ovpncli.cpp",
                "OpenVPN3/openvpn/crypto/data_epoch.cpp",
                "mbedTLS/library",
                "LZ4/lib/lz4.c",
                "LZ4/lib/lz4hc.c"
            ],
            publicHeadersPath: "include",
            cSettings: [
                .headerSearchPath("mbedTLS/include"),
                .headerSearchPath("LZ4/lib"),
                .define("MBEDTLS_RELAXED_X509_DATE")
                // Custom config removed for mbedTLS 2.28.4 compatibility
            ],
            cxxSettings: [
                .headerSearchPath("OpenVPN3"),
                .headerSearchPath("mbedTLS/include"),
                .headerSearchPath("ASIO/asio/include"),
                .headerSearchPath("LZ4/lib"),
                .define("MBEDTLS_RELAXED_X509_DATE"),
                .define("USE_MBEDTLS"),
                .define("USE_ASIO"),
                .define("USE_ASIO_THREADLOCAL"),
                .define("ASIO_STANDALONE"),
                .define("ASIO_NO_DEPRECATED"),
                .define("ASIO_HAS_STD_STRING_VIEW"),
                .define("ENABLE_OVPNDCO", .when(platforms: [.linux])),
                .define("OPENVPN_PLATFORM_IPHONE", .when(platforms: [.iOS, .tvOS])),
                .define("TARGET_OS_TV", .when(platforms: [.tvOS]))
                // Custom config removed for mbedTLS 2.28.4 compatibility
            ],
            linkerSettings: [
                .linkedFramework("Security"),
                .linkedFramework("SystemConfiguration"),
                .linkedFramework("CoreFoundation"),
                .linkedFramework("NetworkExtension", .when(platforms: [.iOS, .tvOS]))
            ]
        ),
        .target(
            name: "OpenVPNAdapter",
            dependencies: ["OpenVPNCore"],
            path: "Sources/OpenVPNAdapter",
            sources: ["Core"],
            publicHeadersPath: "include",
            cxxSettings: [
                .headerSearchPath("../OpenVPNCore/OpenVPN3"),
                .headerSearchPath("../OpenVPNCore/mbedTLS/include"),
                .headerSearchPath("../OpenVPNCore/ASIO/asio/include"),
                .headerSearchPath("../OpenVPNCore/LZ4/lib"),
                .define("MBEDTLS_RELAXED_X509_DATE"),
                .define("USE_MBEDTLS"),
                .define("USE_ASIO"),
                .define("USE_ASIO_THREADLOCAL"),
                .define("ASIO_STANDALONE"),
                .define("ASIO_NO_DEPRECATED"),
                .define("ASIO_HAS_STD_STRING_VIEW"),
                .define("OPENVPN_PLATFORM_IPHONE", .when(platforms: [.iOS, .tvOS]))
                // Custom config removed for mbedTLS 2.28.4 compatibility
            ]
        ),
        .testTarget(
            name: "OpenVPNAdapterTests",
            dependencies: ["OpenVPNAdapter"],
            path: "Tests/OpenVPNAdapterTests",
            sources: ["."]
        ),
    ],
    cxxLanguageStandard: .gnucxx17
)