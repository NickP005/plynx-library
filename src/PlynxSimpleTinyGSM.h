/**
 * @file       PlynxSimpleTinyGSM.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Nov 2016
 * @brief
 *
 */

#ifndef PlynxSimpleTinyGSM_h
#define PlynxSimpleTinyGSM_h

#include <Adapters/PlynxGsmClient.h>

static PlynxArduinoClient _plynxTransport;
PlynxSIM Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
