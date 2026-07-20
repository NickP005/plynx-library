/**
 * @file       PlynxConfig.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Configuration of different aspects of library
 *
 */

#ifndef PlynxConfig_h
#define PlynxConfig_h

#include <Plynx/PlynxDetectDevice.h>

/***************************************************
 * Change these settings to match your need
 ***************************************************/
#define PLYNX_DEFAULT_DOMAIN     "yes.plynx.cc"

#define PLYNX_DEFAULT_PORT       8080
#define PLYNX_DEFAULT_PORT_SSL   443

/***************************************************
 * Professional settings
 ***************************************************/
// Library version.
#define PLYNX_VERSION        "1.0.2"

#define PLYNX_NEW_LIBRARY

// Heartbeat period in seconds.
#ifndef PLYNX_HEARTBEAT
#define PLYNX_HEARTBEAT      45
#endif

// Network timeout in milliseconds.
#ifndef PLYNX_TIMEOUT_MS
#define PLYNX_TIMEOUT_MS     6000UL
#endif

// Limit the amount of outgoing commands per second.
#ifndef PLYNX_MSG_LIMIT
#define PLYNX_MSG_LIMIT      15
#endif

// Limit the incoming command length.
#ifndef PLYNX_MAX_READBYTES
#define PLYNX_MAX_READBYTES  256
#endif

// Limit the outgoing command length.
#ifndef PLYNX_MAX_SENDBYTES
#define PLYNX_MAX_SENDBYTES  128
#endif

// Uncomment to disable built-in analog and digital operations.
//#define PLYNX_NO_BUILTIN

// Uncomment to enable debug prints.
//#define PLYNX_DEBUG

// Uncomment to force-enable 128 virtual pins
//#define PLYNX_USE_128_VPINS

// Uncomment to disable fancy logo
//#define PLYNX_NO_FANCY_LOGO

// Uncomment to enable 3D fancy logo
//#define PLYNX_FANCY_LOGO_3D

// Uncomment to enable experimental functions.
//#define PLYNX_EXPERIMENTAL

// Uncomment to disable all float/double usage
//#define PLYNX_NO_FLOAT

// Uncomment to switch to direct-connect mode
//#define PLYNX_USE_DIRECT_CONNECT


// Uncomment to append command body to header (uses more RAM)
//#define PLYNX_SEND_ATOMIC

// Split whole command into chunks (in bytes)
//#define PLYNX_SEND_CHUNK 64

// Wait after sending each chunk (in milliseconds)
//#define PLYNX_SEND_THROTTLE 10

#endif
