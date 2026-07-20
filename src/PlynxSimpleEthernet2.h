/**
 * @file       PlynxSimpleEthernet2.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2016
 * @brief
 *
 */

#ifndef PlynxSimpleEthernet2_h
#define PlynxSimpleEthernet2_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "W5500"
#endif

#include <Ethernet2.h>
#include <EthernetClient.h>
#include <Adapters/PlynxEthernet.h>

static EthernetClient _plynxEthernetClient;
static PlynxArduinoClient _plynxTransport(_plynxEthernetClient);
PlynxEthernet Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
