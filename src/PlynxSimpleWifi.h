/**
 * @file       PlynxSimpleWifi.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxSimpleWifi_h
#define PlynxSimpleWifi_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "HDG204"
#endif

// Fix for WiFi shield (it has a crazy ping duration)
#define PLYNX_TIMEOUT_MS 6000UL
#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 64

#include <WiFi.h>
#include <Adapters/PlynxWiFiCommon.h>

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifiCommon Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
