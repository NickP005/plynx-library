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
/**
 * @brief The server a board connects to when the sketch names none,
 * "yes.plynx.cc".
 *
 * Plynx.begin(auth, ssid, pass) uses it. To use another server, pass its
 * address to begin() instead of changing this line.
 *
 * @code
 * #include <PlynxSimpleEsp32.h>
 *
 * void setup()
 * {
 *   Plynx.begin("YourAuthToken", "YourWiFiName", "YourWiFiPassword",
 *               "my.server.com", 8080);   // your own server
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 * }
 * @endcode
 *
 * @see PLYNX_DEFAULT_PORT
 */
#define PLYNX_DEFAULT_DOMAIN     "yes.plynx.cc"

/**
 * @brief The port of the server for a plain connection, 8080.
 *
 * The traffic on this port travels unencrypted.
 *
 * @code
 * void showPort()
 * {
 *   Serial.println(PLYNX_DEFAULT_PORT);
 * }
 * @endcode
 *
 * @see PLYNX_DEFAULT_PORT_SSL, PLYNX_DEFAULT_DOMAIN
 */
#define PLYNX_DEFAULT_PORT       8080

/**
 * @brief The port of the server for an encrypted connection, 443.
 *
 * The SSL board headers, such as `<PlynxSimpleEsp32_SSL.h>`, use it.
 *
 * @code
 * void showSslPort()
 * {
 *   Serial.println(PLYNX_DEFAULT_PORT_SSL);
 * }
 * @endcode
 *
 * @see PLYNX_DEFAULT_PORT
 */
#define PLYNX_DEFAULT_PORT_SSL   443

/***************************************************
 * Professional settings
 ***************************************************/
// Library version.
#define PLYNX_VERSION        "1.4.0"

#define PLYNX_NEW_LIBRARY

#ifndef PLYNX_HEARTBEAT
/**
 * @brief How often, in seconds, a quiet board tells the server it is still
 * there. The default is 45.
 *
 * When nothing has travelled for this long, Plynx.run() sends a short ping.
 * The board tells the server this number at every connect, and a Plynx server
 * closes a board that stays silent for about 2.3 times as long.
 *
 * @par Where to call
 * Before the board header.
 *
 * @code
 * #define PLYNX_HEARTBEAT 20     // notice a lost board sooner
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_TIMEOUT_MS, PlynxProtocol::run
 */
#define PLYNX_HEARTBEAT      45
#endif

#ifndef PLYNX_TIMEOUT_MS
/**
 * @brief How long, in milliseconds, the board waits for the network before
 * giving up on a step. The default is 6000.
 *
 * A login the server does not answer in this time is tried again, and
 * reading a message waits at most this long. Plynx.connect() waits three
 * times as long by default.
 *
 * @par Where to call
 * Before the board header.
 *
 * @code
 * #define PLYNX_TIMEOUT_MS 10000UL   // a slow GSM link
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_HEARTBEAT, PlynxProtocol::connect
 */
#define PLYNX_TIMEOUT_MS     6000UL
#endif

#ifndef PLYNX_MSG_LIMIT
/**
 * @brief The most messages per second the board sends to its widgets. The
 * default is 15.
 *
 * Between two messages the library keeps a gap of 1000 / PLYNX_MSG_LIMIT
 * milliseconds (66 with the default), counted from the last message sent or
 * received. A virtualWrite(), setProperty(), syncVirtual(), syncAll(), a
 * Bridge write, an email or a notification that comes sooner waits inside the
 * call, running Plynx.run() meanwhile. logEvent() does not wait. 0 removes the
 * gap.
 *
 * @par Where to call
 * Before the board header.
 *
 * @code
 * #define PLYNX_MSG_LIMIT 30
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @warning Raising it does not make a flooding sketch work better. The server
 * has its own limit and drops what comes in above it.
 *
 * @see PlynxApi::virtualWrite, PLYNX_SEND_THROTTLE
 */
#define PLYNX_MSG_LIMIT      15
#endif

#ifndef PLYNX_MAX_READBYTES
/**
 * @brief The largest message, in bytes, the board accepts from the server.
 * The default is 256.
 *
 * The board tells the server this number at connect. A longer message makes
 * the board drop the connection and reconnect, and with PLYNX_PRINT the
 * serial monitor shows "Packet too big".
 *
 * @par Where to call
 * Before the board header.
 *
 * @code
 * #define PLYNX_MAX_READBYTES 1024   // long texts from a Text Input
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_MAX_SENDBYTES
 */
#define PLYNX_MAX_READBYTES  256
#endif

#ifndef PLYNX_MAX_SENDBYTES
/**
 * @brief The size, in bytes, of the buffer that holds one message the board
 * sends. The default is 128.
 *
 * virtualWrite(), setProperty() and logEvent() build their message in a
 * buffer of this size, on the stack. A text value that does not fit is left
 * out of the message.
 *
 * @par Where to call
 * Before the board header.
 *
 * @code
 * #define PLYNX_MAX_SENDBYTES 256   // longer texts to the app
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @warning A larger buffer takes more stack at every send. On AVR boards,
 * with 2 KB of RAM on an Uno, stay small.
 *
 * @see PLYNX_MAX_READBYTES, PlynxApi::virtualWrite
 */
#define PLYNX_MAX_SENDBYTES  128
#endif

// The most unacknowledged firmware bytes a pushed OTA may have in flight, and
// the single knob of the three: the ack window is derived from it and so is the
// receive buffer below. Announced to whoever sends, as "ota-win". See
// Plynx/PlynxOta.h for why the three numbers cannot be chosen separately.
#ifndef PLYNX_OTA_PUSH_WINDOW
/**
 * @brief For an update over Bluetooth, the most firmware bytes the sender may
 * send before the board confirms them. The default is 4096.
 *
 * The board announces it at connect, and the receive buffer is sized from it.
 *
 * @code
 * #define PLYNX_OTA_PUSH_WINDOW 2048   // less RAM, slower update
 * #include <PlynxSimpleEsp32_NimBLE.h>
 * @endcode
 */
#define PLYNX_OTA_PUSH_WINDOW 4096
#endif

// The room a push transport must give its receive queue: a whole window of
// firmware, plus the framing it travels in ("ota\0d\0<offset>\0" and the
// protocol header, about a tenth at the chunk sizes we use), plus one entire
// frame of slack so the last chunk of a window is never the one that is cut.
#ifndef PLYNX_OTA_RX_BUFFER
#define PLYNX_OTA_RX_BUFFER \
    (PLYNX_OTA_PUSH_WINDOW + PLYNX_OTA_PUSH_WINDOW / 4 + PLYNX_MAX_READBYTES)
#endif

// Which ESP32 DevKit the sketch runs on, by the pins on its header: 30 or 38.
//
//   #define PLYNX_BOARD_PINS 38
//   #include <PlynxSimpleEsp32.h>
//
// The board then says it in its info block ("pins") at every connect, and the
// Plynx app draws that pinout and stops asking. The two DevKits carry the same
// chip, so nothing the board can measure tells them apart: only the sketch
// knows. Leave it out and the app keeps asking, as before.
//
// Only the two real headers are accepted. Any other number stops the build
// here, because a wrong count would put a pinout on screen that the board in
// the user's hand does not have.
#ifdef PLYNX_BOARD_PINS
    #if PLYNX_BOARD_PINS == 30
        #define PLYNX_INFO_PINS "30"
    #elif PLYNX_BOARD_PINS == 38
        #define PLYNX_INFO_PINS "38"
    #else
        #error "PLYNX_BOARD_PINS must be 30 or 38 (the pins on your ESP32 DevKit header)"
    #endif
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

/**
 * @def PLYNX_VERSION
 * @brief The version of the Plynx library, as a text such as "1.4.0".
 *
 * The board sends it to the server at every connect, and the serial monitor
 * shows it at start when PLYNX_PRINT is on.
 *
 * @code
 * void printVersion()
 * {
 *   Serial.println("Plynx library " PLYNX_VERSION);
 * }
 * @endcode
 */

#endif
