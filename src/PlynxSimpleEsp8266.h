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

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <ESP8266WiFi.h>

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

};

static WiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxWifi Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
