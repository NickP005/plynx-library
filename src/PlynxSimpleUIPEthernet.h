/**
 * @file       PlynxSimpleUipEthernet.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxSimpleUipEthernet_h
#define PlynxSimpleUipEthernet_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "ENC28J60"
#endif

#define PLYNX_ENC28J60_FIX
#include <UIPEthernet.h>
#include <Adapters/PlynxEthernet.h>

static EthernetClient _plynxEthernetClient;
static PlynxArduinoClient _plynxTransport(_plynxEthernetClient);
PlynxEthernet Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
