/**
 * @file       PlynxSimpleWiFiNINA.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2018 Volodymyr Shymanskyy
 * @date       Sep 2018
 * @brief
 *
 */

#ifndef PlynxSimpleWiFiNINA_h
#define PlynxSimpleWiFiNINA_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "WiFiNINA"
#endif

#define PLYNX_SEND_ATOMIC
//#define PLYNX_USE_SSL

#include <WiFiNINA.h>
#include <Adapters/PlynxWiFiCommon.h>

//static WiFiSSLClient _plynxWifiClient;
static WiFiClient _plynxWifiClient;

static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifiCommon Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
