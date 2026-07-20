/**
 * @file PlynxSimpleWizFi310.h
 * @
 * @license This project is released under the MIT License (MIT)
 * @copyright Copyright (c) 2015 Volodymyr Shymanskyy
 * @date Oct 2017
 * @brief
 *
 */
#ifndef PlynxSimpleWizFi310_h
#define PlynxSimpleWizFi310_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "WizFi310"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 64

#include <WizFi310.h>
#include <Adapters/PlynxWiFiCommon.h>

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifiCommon Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
