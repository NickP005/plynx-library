/**
 * @file       PlynxSimpleEnergiaEthernet.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Oct 2016
 * @brief
 *
 */

#ifndef PlynxSimpleEnergiaEthernet_h
#define PlynxSimpleEnergiaEthernet_h

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Adapters/PlynxEthernet.h>

static EthernetClient _plynxEthernetClient;
static PlynxArduinoClient _plynxTransport(_plynxEthernetClient);
PlynxEthernet Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
