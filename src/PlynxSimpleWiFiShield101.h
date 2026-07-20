/**
 * @file       PlynxSimpleWiFiShield101.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2016
 * @brief
 *
 */

#ifndef PlynxSimpleWiFiShield101_h
#define PlynxSimpleWiFiShield101_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "WiFi101"
#endif

#include <WiFi101.h>
#include <Adapters/PlynxWiFiCommon.h>

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifiCommon Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
