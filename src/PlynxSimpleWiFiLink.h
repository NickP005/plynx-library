/**
 * @file PlynxSimpleWifiLink.h
 * @
 * @license This project is released under the MIT License (MIT)
 * @copyright Copyright (c) 2015 Volodymyr Shymanskyy
 * @date Jan 2017
 * @brief
 *
 */
#ifndef PlynxSimpleWiFiLink_h
#define PlynxSimpleWiFiLink_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "WiFiLink"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 64

#include <WiFiLink.h>
#include <Adapters/PlynxWiFiCommon.h>

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifiCommon Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
