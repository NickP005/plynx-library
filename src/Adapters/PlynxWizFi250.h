/**
 * @file       PlynxWiFiCommon.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxWizFi250_h
#define PlynxWizFi250_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "WiFi"
#endif

#ifdef PLYNX_USE_SSL
  #define PLYNX_SERVER_PORT PLYNX_DEFAULT_PORT_SSL
#else
  #define PLYNX_SERVER_PORT PLYNX_DEFAULT_PORT
#endif

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>

class PlynxWifiCommon
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxWifiCommon(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    void connectWiFi(const char* ssid, const char* pass)
    {
        int status = WiFi.status();
        // check for the presence of the shield:
        if (status == WL_NO_SHIELD) {
            PLYNX_FATAL("WiFi module not found");
        }

#ifdef PLYNX_DEBUG
        PLYNX_LOG2(PLYNX_F("WiFi firmware: "), WiFi.firmwareVersion());
#endif

        // attempt to connect to Wifi network:
        while (status != WL_CONNECTED) {
            PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
            status = WiFi.begin((char*)ssid, (const char*)pass);

            millis_time_t started = PlynxMillis();
            while ((status != WL_CONNECTED) &&
                  (PlynxMillis() - started < 10000))
            {
                PlynxDelay(100);
                status = WiFi.status();
            }
        }

        IPAddress myip = WiFi.localIP();
        PLYNX_LOG_IP("IP: ", myip);
    }

    void config(const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_SERVER_PORT)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
    }

    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = PLYNX_SERVER_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_SERVER_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               IPAddress   ip,
               uint16_t    port   = PLYNX_SERVER_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, ip, port);
        while(this->connect() != true) {}
    }

};

#endif
