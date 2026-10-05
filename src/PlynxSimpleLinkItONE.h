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

    /**
     * @brief Joins a WiFi network. begin() calls it for you.
     *
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param wifi_auth The WiFi security, such as LWIFI_WPA.
     *
     * @see begin
     */
    void connectWiFi(const char* ssid, const char* pass, int wifi_auth)
    {
        PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
        LWiFi.begin();
        while(!LWiFi.connect(ssid, LWiFiLoginInfo((LWiFiEncryption)wifi_auth, pass))){
            PlynxDelay(1000);
        }
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ip        The IP address of the server.
     * @param port      The port of the server.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    /**
     * @brief Connects a LinkIt ONE to the WiFi and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param wifi_auth The WiFi security, such as LWIFI_WPA.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/LinkItONE.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
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

    /**
     * @brief Connects a LinkIt ONE to the WiFi and to a Plynx server given by its IP address, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param wifi_auth The WiFi security, such as LWIFI_WPA.
     * @param ip        The IP address of the server.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/LinkItONE.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
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
