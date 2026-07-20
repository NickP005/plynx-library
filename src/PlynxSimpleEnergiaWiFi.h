/**
 * @file       PlynxSimpleEnergiaWiFi.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxSimpleEnergiaWiFi_h
#define PlynxSimpleEnergiaWiFi_h

#include <WiFi.h>
#include <Adapters/PlynxWiFiCommon.h>

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifiCommon Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
