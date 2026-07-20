/**
 * @file       PlynxSimpleWiFly.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxSimpleWiFly_h
#define PlynxSimpleWiFly_h

#include <Adapters/PlynxWiFly.h>

static PlynxTransportWiFly _plynxTransport;
PlynxWiFly Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
