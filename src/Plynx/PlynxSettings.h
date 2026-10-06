/**
 * @file       PlynxSettings.h
 * @license    This project is released under the MIT License (MIT)
 * @brief      The settings a sketch can write above its board header.
 *
 * Each setting is a `#define` line that goes before the `#include` of the
 * board header.
 *
 * @code
 * #define PLYNX_PRINT Serial
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * This file only documents them. No sketch and no part of the library
 * includes it, and its lines compile to nothing, because the library reads
 * each setting where it uses it. The settings with a default value, such as
 * PLYNX_HEARTBEAT, are documented in Plynx/PlynxConfig.h.
 */

#ifndef PlynxSettings_h
#define PlynxSettings_h

#ifdef DOXYGEN


/**
 * @brief Prints what the library does on a serial port, such as `Serial`.
 *
 * The serial monitor then shows the start, the WiFi, "Connecting to", "Ready"
 * with the ping time, and the reason when something fails, such as "Invalid
 * auth token". It is the first thing to turn on when a board does not appear
 * online.
 *
 * Write it on the first line of the sketch, above the board header. Call
 * Serial.begin() in setup() before Plynx.begin().
 *
 * @code
 * #define PLYNX_PRINT Serial
 * #include <PlynxSimpleEsp32.h>
 *
 * void setup()
 * {
 *   Serial.begin(115200);
 *   Plynx.begin("YourAuthToken", "YourWiFiName", "YourWiFiPassword");
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 * }
 * @endcode
 *
 * @note The messages take flash. Remove the line when the sketch is
 * finished, especially on AVR boards.
 *
 * @see PLYNX_DEBUG
 */
#define PLYNX_PRINT Serial

/**
 * @brief Prints much more detail on the PLYNX_PRINT port, including every
 * message that comes in and goes out.
 *
 * It works only together with PLYNX_PRINT.
 *
 * Write it above the board header, next to PLYNX_PRINT.
 *
 * @code
 * #define PLYNX_PRINT Serial
 * #define PLYNX_DEBUG
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @warning It slows the board down and fills the serial monitor. Use it to
 * find a problem, then remove it.
 *
 * @see PLYNX_PRINT, PLYNX_DEBUG_ALL
 */
#define PLYNX_DEBUG

/**
 * @brief Prints everything PLYNX_DEBUG prints, plus every message the library
 * waits for or skips.
 *
 * It turns PLYNX_DEBUG on by itself and needs PLYNX_PRINT.
 *
 * @code
 * #define PLYNX_PRINT Serial
 * #define PLYNX_DEBUG_ALL
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_DEBUG
 */
#define PLYNX_DEBUG_ALL

/**
 * @brief Tells which ESP32 DevKit the sketch runs on, by the pins on its
 * header, 30 or 38.
 *
 * The board then sends it at every connect, and the Plynx app draws that
 * pinout instead of asking. The two DevKits carry the same chip, so only the
 * sketch knows. Any other number stops the build with "PLYNX_BOARD_PINS must
 * be 30 or 38".
 *
 * Write it above the board header.
 *
 * @code
 * #define PLYNX_BOARD_PINS 38
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 */
#define PLYNX_BOARD_PINS 38

/**
 * @brief The ID of a project published in the Plynx catalog that comes with
 * this sketch.
 *
 * The board sends it at every connect and during setup, and the Plynx app
 * offers the ready-made dashboard of that project when the board is paired.
 *
 * Write it above the board header.
 *
 * @code
 * #define PLYNX_PROJECT "your-published-project-id"
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @warning Keep the ID within 32 characters. It shares a small buffer with
 * PLYNX_TEMPLATE_ID.
 */
#define PLYNX_PROJECT "your-published-project-id"

/**
 * @brief Leaves out the built-in update over the air on ESP32 and ESP8266.
 *
 * The board then cannot receive a new firmware from the server, and the
 * sketch saves about 7.5 KB of flash.
 *
 * Write it above the board header.
 *
 * @code
 * #define PLYNX_NO_OTA
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_NO_BOOTLOADER_ROLLBACK, PLYNX_OTA_GUARD_MS
 */
#define PLYNX_NO_OTA

/**
 * @brief Keeps the update over the air but leaves the ESP32 bootloader hook
 * alone, so a bad update is not undone by itself.
 *
 * It is required next to ESP RainMaker, which defines the same hook. Without
 * it the link fails with "multiple definition of verifyRollbackLater".
 *
 * Write it above `<PlynxSimpleEsp32.h>`.
 *
 * @code
 * #define PLYNX_NO_BOOTLOADER_ROLLBACK
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_NO_OTA, PLYNX_OTA_GUARD_MS
 */
#define PLYNX_NO_BOOTLOADER_ROLLBACK

/**
 * @brief Accepts firmware downloads only from this host name.
 *
 * By default the board downloads an update only from the server it is
 * configured to talk to. Set this when the firmware lives on another host.
 *
 * @code
 * #define PLYNX_OTA_HOST "fw.example.com"
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_OTA_NO_HOST_PIN
 */
#define PLYNX_OTA_HOST "fw.example.com"

/**
 * @brief Accepts firmware downloads from any host.
 *
 * @code
 * #define PLYNX_OTA_NO_HOST_PIN
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @warning Anyone who can forge the update message on a plain connection
 * can then point the board to a firmware of their choice.
 *
 * @see PLYNX_OTA_HOST
 */
#define PLYNX_OTA_NO_HOST_PIN

/**
 * @brief Leaves out the setup hotspot that lets the Plynx app configure the
 * board, on ESP32 and ESP8266.
 *
 * Plynx.begin() with no parameters then does not exist, and the sketch has to
 * pass the token and the WiFi to begin().
 *
 * @code
 * #define PLYNX_NO_PROVISION
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PlynxWifi::begin()
 */
#define PLYNX_NO_PROVISION

/**
 * @brief Leaves out the logo and the version the library prints at start on
 * the PLYNX_PRINT port.
 *
 * @code
 * #define PLYNX_PRINT Serial
 * #define PLYNX_NO_DEFAULT_BANNER
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_NO_FANCY_LOGO
 */
#define PLYNX_NO_DEFAULT_BANNER

/**
 * @brief Prints a single line with the version at start, instead of the
 * logo drawn with characters.
 *
 * @code
 * #define PLYNX_PRINT Serial
 * #define PLYNX_NO_FANCY_LOGO
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_NO_DEFAULT_BANNER
 */
#define PLYNX_NO_FANCY_LOGO

/**
 * @brief Turns off the handling of hardware pins written or read directly by
 * the app.
 *
 * Without it, a widget set to a digital pin (D) or an analog pin (A) works
 * with no code in the sketch. With it, only virtual pins work, and the
 * library is smaller.
 *
 * @code
 * #define PLYNX_NO_BUILTIN
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see V0
 */
#define PLYNX_NO_BUILTIN

/**
 * @brief Gives the board virtual pins up to V127 instead of V31.
 *
 * ESP32, ESP8266, SAMD boards and Linux have it already. On the other boards
 * it costs two tables of 128 entries in flash.
 *
 * @code
 * #define PLYNX_USE_128_VPINS
 * #include <PlynxSimpleStream.h>
 * @endcode
 *
 * @see V0
 */
#define PLYNX_USE_128_VPINS

/**
 * @brief Turns on the experimental functions, which may change in a future
 * version.
 *
 * They are Plynx.delay(), Plynx.refresh() and the PLYNX_EVERY_N_MILLIS()
 * family.
 *
 * @code
 * #define PLYNX_EXPERIMENTAL
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PlynxApi::delay, PLYNX_EVERY_N_MILLIS
 */
#define PLYNX_EXPERIMENTAL

/**
 * @brief Removes every use of `float` and `double` from the library, for very
 * small boards.
 *
 * asFloat(), asDouble(), PLYNX_VAR_DOUBLE() and GpsParam disappear, and
 * sending a `float` with virtualWrite() stops the build.
 *
 * @code
 * #define PLYNX_NO_FLOAT
 * #include <PlynxSimpleStream.h>
 * @endcode
 */
#define PLYNX_NO_FLOAT

/**
 * @brief Makes the board accept a login from whoever connects to it, instead
 * of logging in to a server.
 *
 * The board checks the token sent by the other side. The DirectConnect
 * examples use it with a transport that waits for connections.
 *
 * @code
 * #define PLYNX_USE_DIRECT_CONNECT
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 */
#define PLYNX_USE_DIRECT_CONNECT

/**
 * @brief Builds each message whole before writing it to the connection.
 *
 * It takes more RAM, as much as the message. ESP32, ESP8266 and Particle
 * boards always do it.
 *
 * @code
 * #define PLYNX_SEND_ATOMIC
 * #include <PlynxSimpleStream.h>
 * @endcode
 *
 * @see PLYNX_SEND_CHUNK
 */
#define PLYNX_SEND_ATOMIC

/**
 * @brief Leaves pinMode() to the sketch for the hardware pins the app writes
 * directly.
 *
 * By default the library sets a pin as OUTPUT before every write from the
 * app.
 *
 * @code
 * #define PLYNX_MINIMIZE_PINMODE_USAGE
 * #include <PlynxSimpleEsp32.h>
 *
 * void setup()
 * {
 *   pinMode(2, OUTPUT);
 * }
 * @endcode
 *
 * @see PLYNX_NO_BUILTIN
 */
#define PLYNX_MINIMIZE_PINMODE_USAGE

/**
 * @brief The name of the template of the device, for servers that organise
 * devices in templates.
 *
 * The board sends it at connect as "tmpl". It also turns notify(), email()
 * and tweet() into logEvent("notify", ...). A Plynx server does not need it.
 *
 * Write it above the board header.
 *
 * @code
 * #define PLYNX_TEMPLATE_ID "TMPL12345"
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_FIRMWARE_VERSION, PlynxApi::logEvent
 */
#define PLYNX_TEMPLATE_ID "TMPL12345"

/**
 * @brief The version of the sketch, as a text, that the board sends to the
 * server at every connect.
 *
 * The server keeps it with the information about the board, so you can tell
 * which firmware a board runs.
 *
 * Write it above the board header.
 *
 * @code
 * #define PLYNX_FIRMWARE_VERSION "1.0.3"
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_FIRMWARE_TYPE
 */
#define PLYNX_FIRMWARE_VERSION "1.0.3"

/**
 * @brief The kind of firmware, as a text, that the board sends at connect as
 * "fw-type".
 *
 * Without it, PLYNX_TEMPLATE_ID is used when present.
 *
 * @code
 * #define PLYNX_FIRMWARE_TYPE "greenhouse"
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_FIRMWARE_VERSION
 */
#define PLYNX_FIRMWARE_TYPE "greenhouse"

#endif // DOXYGEN

#endif // PlynxSettings_h
