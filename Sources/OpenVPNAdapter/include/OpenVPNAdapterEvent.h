#import <Foundation/Foundation.h>

/**
 OpenVPN event codes
 */
typedef NS_ENUM(NSInteger, OpenVPNAdapterEvent) {
    OpenVPNAdapterEventDisconnected,
    OpenVPNAdapterEventConnected,
    OpenVPNAdapterEventReconnecting,
    OpenVPNAdapterEventAuthPending,
    OpenVPNAdapterEventResolve,
    OpenVPNAdapterEventWait,
    OpenVPNAdapterEventWaitProxy,
    OpenVPNAdapterEventConnecting,
    OpenVPNAdapterEventGetConfig,
    OpenVPNAdapterEventAssignIP,
    OpenVPNAdapterEventAddRoutes,
    OpenVPNAdapterEventEcho,
    OpenVPNAdapterEventInfo,
    OpenVPNAdapterEventWarn,
    OpenVPNAdapterEventPause,
    OpenVPNAdapterEventResume,
    OpenVPNAdapterEventRelay,
    OpenVPNAdapterEventCompressionEnabled,
    OpenVPNAdapterEventUnsupportedFeature,
    OpenVPNAdapterEventUnknown
};