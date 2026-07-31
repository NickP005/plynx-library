/**
 * @file       PlynxSimpleEsp32.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Oct 2016
 * @brief
 *
 */

#ifndef PlynxSimpleEsp32_h
#define PlynxSimpleEsp32_h

#ifndef ESP32
#error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

#define PLYNX_SEND_ATOMIC

#ifndef PLYNX_NO_PROVISION
// blnkinf "cm" flag: the connection manager (SoftAP provisioning) is
// compiled in. Must be defined BEFORE PlynxApiArduino.h, where the
// preprocessor builds the blnkinf profile string.
#define PLYNX_HAS_CONNECTION_MANAGER
#endif

#ifndef PLYNX_NO_OTA
// Built-in OTA handler (Plynx/PlynxOta.h, included at the bottom).
// Must be defined BEFORE PlynxProtocol.h: it enables the OTA dispatch and
// the pending-confirm/rollback hooks. Opt out with #define PLYNX_NO_OTA.
#define PLYNX_OTA_HAS_DEFAULT
#endif

// blnkinf "ota-max"/"flash"/"sk-size": how big an OTA image this board can
// take. Must come after the PLYNX_OTA_HAS_DEFAULT decision above and before
// PlynxApiArduino.h, which assembles the profile.
#include <Plynx/PlynxFlashInfo.h>

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <WiFi.h>
#ifndef PLYNX_NO_PROVISION
#include <Plynx/PlynxProvision.h>
#endif

class PlynxWifi
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxWifi(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    void connectWiFi(const char* ssid, const char* pass)
    {
        PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
        WiFi.mode(WIFI_STA);
        if (pass && strlen(pass)) {
            WiFi.begin(ssid, pass);
        } else {
            WiFi.begin(ssid);
        }
        while (WiFi.status() != WL_CONNECTED) {
#ifdef PLYNX_OTA_HAS_DEFAULT
            // Rollback watchdog must run even if the freshly-flashed
            // firmware cannot join WiFi (e.g. wrong compiled-in credentials).
            PlynxOtaTick();
#endif
            PlynxDelay(500);
        }
        PLYNX_LOG1(PLYNX_F("Connected to WiFi"));

        IPAddress myip = WiFi.localIP();
        (void)myip; // Eliminate warnings about unused myip
        PLYNX_LOG_IP("IP: ", myip);
    }

    void config(const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
    }

    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

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
    /**
     * Token-less begin: "one flash fits all".
     *  - saved config valid -> connect as usual;
     *  - none (or WiFi/token turn out bad) -> SoftAP "Plynx-XXXX" that the
     *    Plynx app finds and configures. See Plynx/PlynxProvision.h.
     * Classic begin(auth, ssid, pass) users pay zero bytes for this: the
     * linker drops everything below when begin() is never called.
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

    /// Factory reset for the auto mode (wire to a long-press button).
    void resetProvisioning()
    {
        PlynxProvisioner::clear();
        ESP.restart();
    }
#endif // PLYNX_NO_PROVISION

};

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifi Plynx(_plynxTransport);

#ifdef PLYNX_OTA_HAS_DEFAULT
// Needs the global Plynx object above (disconnect/startSession).
#include <Plynx/PlynxOta.h>
#endif

#include <PlynxWidgets.h>

#endif
