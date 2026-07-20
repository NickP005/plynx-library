/**
 * @file       PlynxSerialBLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Mar 2016
 * @brief
 *
 */

#ifndef PlynxSerialBLE_h
#define PlynxSerialBLE_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "SerialBLE"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK      20
#define PLYNX_SEND_THROTTLE   40
#define PLYNX_NO_INFO

#include <Adapters/PlynxSerial.h>

PlynxTransportStream _plynxTransport;
PlynxStream Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
