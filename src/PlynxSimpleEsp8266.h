/**
 * @file       PlynxSimpleEsp8266.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxSimpleEsp8266_h
#define PlynxSimpleEsp8266_h

#ifndef ESP8266
#error This code is intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif

#include <version.h>

#if ESP_SDK_VERSION_NUMBER < 0x020200
#error Please update your ESP8266 Arduino Core
#endif

#ifndef PLYNX_NO_PROVISION
// blnkinf "cm" flag: the connection manager (SoftAP provisioning) is
// compiled in. Must be defined BEFORE PlynxApiArduino.h, where the
// preprocessor builds the blnkinf profile string.
#define PLYNX_HAS_CONNECTION_MANAGER
#endif

#ifndef PLYNX_NO_OTA
// Built-in OTA handler (Plynx/PlynxOta.h, included at the bottom).
// Must be defined BEFORE PlynxProtocol.h: it enables the OTA dispatch.
// No automatic rollback on ESP8266 (single-slot Updater); opt out with
// #define PLYNX_NO_OTA.
#define PLYNX_OTA_HAS_DEFAULT
#endif

// blnkinf "ota-max"/"flash"/"sk-size": how big an OTA image this board can
// take. Must come after the PLYNX_OTA_HAS_DEFAULT decision above and before
// PlynxApiArduino.h, which assembles the profile.
#include <Plynx/PlynxFlashInfo.h>

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <ESP8266WiFi.h>
#ifndef PLYNX_NO_PROVISION
#include <Plynx/PlynxProvision.h>
#endif

/**
 * @brief The Plynx object of an ESP8266 on WiFi, created by
 * `<PlynxSimpleEsp8266.h>` as the global `Plynx`.
 *
 * It joins the WiFi, connects to the server and keeps the connection alive.
 * With no token in the code, begin() opens a setup hotspot that the Plynx app
 * uses to configure the board. It also receives updates over the air.
 *
 * @code
 * #include <PlynxSimpleEsp8266.h>
 *
 * char auth[] = "YourAuthToken";   // from the Plynx app
 *
 * void setup()
 * {
 *   Plynx.begin(auth, "YourWiFiName", "YourWiFiPassword");
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 * }
 * @endcode
 *
 * @see begin, PlynxProtocol::run, PlynxApi::virtualWrite
 */
class PlynxWifi
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxWifi(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    /**
     * @brief Joins a WiFi network and waits until the board has an address.
     *
     * begin() calls it for you. It waits without a time limit.
     *
     * @param ssid  The name of the WiFi network.
     * @param pass  The password, or an empty text for an open network.
     *
     * @code
     * #include <PlynxSimpleEsp8266.h>
     *
     * void setup()
     * {
     *   Plynx.connectWiFi("YourWiFiName", "YourWiFiPassword");
     *   Plynx.config("YourAuthToken");
     *   Plynx.connect();
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @warning With a wrong name or password it never returns.
     *
     * @see begin, config
     */
    void connectWiFi(const char* ssid, const char* pass)
    {
        PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
        WiFi.mode(WIFI_STA);
        if (WiFi.status() != WL_CONNECTED) {
            if (pass && strlen(pass)) {
                WiFi.begin(ssid, pass);
            } else {
                WiFi.begin(ssid);
            }
        }
        while (WiFi.status() != WL_CONNECTED) {
            PlynxDelay(500);
        }
        PLYNX_LOG1(PLYNX_F("Connected to WiFi"));

        IPAddress myip = WiFi.localIP();
        (void)myip; // Eliminate warnings about unused myip
        PLYNX_LOG_IP("IP: ", myip);
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * Use it with connect() when the sketch has to start even without a
     * connection. The WiFi is up to the sketch.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server. The default is PLYNX_DEFAULT_PORT.
     *
     * @par Where to call
     * In setup(), before Plynx.connect().
     *
     * @code
     * #include <PlynxSimpleEsp8266.h>
     *
     * void setup()
     * {
     *   WiFi.begin("YourWiFiName", "YourWiFiPassword");
     *   Plynx.config("YourAuthToken");
     *   Plynx.connect(5000);   // at most 5 seconds, then go on
     * }
     *
     * void loop()
     * {
     *   Plynx.run();             // connects later if it could not yet
     * }
     * @endcode
     *
     * @warning Keep the token in a global variable or a literal. The library
     * keeps a pointer to it and does not copy it.
     *
     * @see PlynxProtocol::connect, begin
     */
    void config(const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
    }

    /**
     * @brief Sets the token and the server by its IP address, without
     * connecting yet.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ip    The IP address of the server.
     * @param port  The port of the server. The default is PLYNX_DEFAULT_PORT.
     *
     * @code
     * #include <PlynxSimpleEsp8266.h>
     *
     * void setup()
     * {
     *   WiFi.begin("YourWiFiName", "YourWiFiPassword");
     *   Plynx.config("YourAuthToken", IPAddress(192, 168, 1, 100), 8080);
     *   Plynx.connect();
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @see config, PlynxProtocol::connect
     */
    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    /**
     * @brief Connects the board to the WiFi and to the Plynx server, and
     * waits until both are done.
     *
     * It joins the WiFi, then logs in with the token and returns once the
     * server has accepted it. PLYNX_CONNECTED() has already run when begin()
     * returns.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param ssid    The name of the WiFi network.
     * @param pass    The password of the WiFi network.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server. The default is PLYNX_DEFAULT_PORT.
     *
     * @par Where to call
     * In setup(), once, after Serial.begin().
     *
     * @code
     * #define PLYNX_PRINT Serial
     * #include <PlynxSimpleEsp8266.h>
     *
     * char auth[] = "YourAuthToken";
     * char ssid[] = "YourWiFiName";
     * char pass[] = "YourWiFiPassword";
     *
     * void setup()
     * {
     *   Serial.begin(115200);
     *   Plynx.begin(auth, ssid, pass);
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @warning It waits without a time limit. With a wrong WiFi password, a
     * server that cannot be reached or a wrong token, setup() never ends.
     * Turn on PLYNX_PRINT to read why. To start without a connection, use
     * config() and connect() instead.
     *
     * @see config, PlynxProtocol::connect, PlynxProtocol::run, PLYNX_PRINT
     */
    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    /**
     * @brief Connects the board to the WiFi and to a Plynx server given by its
     * IP address, and waits until both are done.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ssid  The name of the WiFi network.
     * @param pass  The password of the WiFi network.
     * @param ip    The IP address of the server, such as a Plynx server on
     *              your network.
     * @param port  The port of the server. The default is PLYNX_DEFAULT_PORT.
     *
     * @code
     * #include <PlynxSimpleEsp8266.h>
     *
     * void setup()
     * {
     *   Plynx.begin("YourAuthToken", "YourWiFiName", "YourWiFiPassword",
     *               IPAddress(192, 168, 1, 100), 8080);
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @warning It waits without a time limit, like the other begin().
     *
     * @see begin
     */
    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               IPAddress   ip,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, ip, port);
        while(this->connect() != true) {}
    }

#ifndef PLYNX_NO_PROVISION
    // Token-less begin: "one flash fits all". Classic begin(auth, ssid, pass)
    // users pay zero bytes for this: the linker drops everything below when
    // begin() is never called. See Plynx/PlynxProvision.h.
    /**
     * @brief Connects with the WiFi and the token saved on the board, or opens
     * a setup hotspot when there are none.
     *
     * The first time, the board creates a WiFi network called Plynx-XXXX. The
     * Plynx app finds it, sends the WiFi and the token, and the board saves
     * them and restarts. From then on begin() connects directly. When the
     * saved WiFi does not answer within 30 seconds, or the server refuses the
     * token, the hotspot opens again. The same sketch works on every board,
     * with no token in the code.
     *
     * @par Where to call
     * In setup(), once.
     *
     * @par App widget
     * None. The Plynx app shows the board while adding a device.
     *
     * @code
     * #define PLYNX_PRINT Serial
     * #include <PlynxSimpleEsp8266.h>
     *
     * void setup()
     * {
     *   Serial.begin(115200);
     *   Plynx.begin();
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @note The hotspot stays open for 3 minutes (PLYNX_PROV_TIMEOUT_MS), then
     * the board restarts and tries again.
     *
     * @see resetProvisioning, PLYNX_NO_PROVISION, PLYNX_PROJECT
     */
    void begin()
    {
        // Function-local static: alive forever once used (the protocol keeps
        // a pointer to the token), zero RAM when the sketch never calls this.
        static PlynxProvConfig cfg;

        for (;;) {
            if (PlynxProvisioner::load(cfg)) {
                if (PlynxProvisioner::connectBounded(cfg.ssid, cfg.pass,
                                                     PLYNX_PROV_WIFI_TIMEOUT_MS)) {
                    config(cfg.token, cfg.host, cfg.port);
                    while (this->connect() != true) {
                        if (this->isTokenInvalid()) {
                            cfg.lastError = PLYNX_PROV_ERR_TOKEN;
                            break;
                        }
                    }
                    if (this->connected()) {
                        return;
                    }
                } else {
                    // Saved WiFi unreachable (moved house? typo?): back to AP,
                    // /status will say why.
                    cfg.lastError = PLYNX_PROV_ERR_NETWORK;
                }
            }
            PlynxProvisioner::run(cfg);   // blocks; reboots on save or timeout
        }
    }

    /**
     * @brief Forgets the saved WiFi and token and restarts the board, which
     * opens the setup hotspot again.
     *
     * @par Where to call
     * When the person asks for it, for example on a long press of a button.
     *
     * @code
     * #include <PlynxSimpleEsp8266.h>
     *
     * void checkResetButton()
     * {
     *   if (digitalRead(0) == LOW) {     // the BOOT button on many boards
     *     Plynx.resetProvisioning();
     *   }
     * }
     * @endcode
     *
     * @see begin()
     */
    void resetProvisioning()
    {
        PlynxProvisioner::clear();
        ESP.restart();
    }
#endif // PLYNX_NO_PROVISION

};

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
/**
 * @brief The Plynx object of the sketch, the one every `Plynx.` call talks
 * to.
 *
 * `<PlynxSimpleEsp8266.h>` creates it, so a sketch never declares it.
 *
 * @code
 * #include <PlynxSimpleEsp8266.h>
 *
 * void setup()
 * {
 *   Plynx.begin("YourAuthToken", "YourWiFiName", "YourWiFiPassword");
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 * }
 * @endcode
 */
PlynxWifi Plynx(_plynxTransport);

#ifdef PLYNX_OTA_HAS_DEFAULT
// Needs the global Plynx object above (disconnect/startSession).
#include <Plynx/PlynxOta.h>
#endif

#include <PlynxWidgets.h>

#endif
