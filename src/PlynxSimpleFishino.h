/**
 * @file       PlynxSimpleFishino.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jun 2016
 * @brief
 *
 */

#ifndef PlynxSimpleFishino_h
#define PlynxSimpleFishino_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "Fishino"
#endif

#ifdef ESP8266
#error This code is not intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <Fishino.h>

class PlynxFishino
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxFishino(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    /**
     * @brief Joins a WiFi network. begin() calls it for you.
     *
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     *
     * @see begin
     */
    void connectWiFi(const char* ssid, const char* pass)
    {
        PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
        SPI.begin();
        SPI.setClockDivider(SPI_CLOCK_DIV2);

        while(!Fishino.reset()) {
            PLYNX_LOG1(PLYNX_F("Fishino RESET FAILED"));
        }

        Fishino.setMode(STATION_MODE);
        if (pass && strlen(pass)) {
            while (!Fishino.begin(ssid, pass)) {
                PlynxDelay(500);
            }
        } else {
            while (!Fishino.begin(ssid)) {
                PlynxDelay(500);
            }
        }

        PLYNX_LOG1(PLYNX_F("Connected to WiFi"));

        Fishino.staStartDHCP();
        while(Fishino.status() != STATION_GOT_IP)
        {
            PlynxDelay(500);
        }

        IPAddress myip = Fishino.localIP();
        PLYNX_LOG_IP("IP: ", myip);
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
                uint16_t    port = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    /**
     * @brief Connects a Fishino to the WiFi and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * Call it in setup(), once. The whole sketch is in examples/Boards_WiFi/Fishino.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
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
     * @brief Connects a Fishino to the WiFi and to a Plynx server given by its IP address, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param ip        The IP address of the server.
     * @param port      The port of the server.
     *
     * Call it in setup(), once. The whole sketch is in examples/Boards_WiFi/Fishino.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
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

};

static FishinoClient _plynxFishinoClient;
static PlynxArduinoClient _plynxTransport(_plynxFishinoClient);
PlynxFishino Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
