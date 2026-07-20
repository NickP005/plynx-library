/**
 * @file       PlynxSimpleEthernet.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxSimpleEthernet_h
#define PlynxSimpleEthernet_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "W5100"
#endif

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Adapters/PlynxEthernet.h>

static EthernetClient _plynxEthernetClient;
static PlynxArduinoClient _plynxTransport(_plynxEthernetClient);
PlynxEthernet Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
