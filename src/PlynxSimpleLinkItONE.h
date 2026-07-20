/**
 * @file       PlynxSimpleLinkItONE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jul 2015
 * @brief
 *
 */

#ifndef PlynxSimpleLinkItONE_h
#define PlynxSimpleLinkItONE_h

#ifndef PLYNX_INFO_DEVICE
#define PLYNX_INFO_DEVICE  "LinkIt ONE"
#endif

// cause this causes crashes...
#define PLYNX_NO_YIELD

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

class PlynxLinkItOneWifi
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxLinkItOneWifi(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    void connectWiFi(const char* ssid, const char* pass, int wifi_auth)
    {
        PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
        LWiFi.begin();
        while(!LWiFi.connect(ssid, LWiFiLoginInfo((LWiFiEncryption)wifi_auth, pass))){
            PlynxDelay(1000);
        }
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
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               int         wifi_auth,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t port      = PLYNX_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass, wifi_auth);
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               int         wifi_auth,
               IPAddress   ip,
               uint16_t    port = PLYNX_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass, wifi_auth);
        config(auth, ip, port);
        while(this->connect() != true) {}
    }

};


static LWiFiClient _plynxWifiClient;
static PlynxArduinoClient _plynxTransport(_plynxWifiClient);
PlynxLinkItOneWifi Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
