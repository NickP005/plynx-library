/**
 * @file       PlynxWiFiCommon.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxWiFiCommon_h
#define PlynxWiFiCommon_h

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

/**
 * @brief The Plynx object of the boards that use the Arduino WiFi libraries,
 * such as the MKR 1010, the Nano 33 IoT and the Uno WiFi Rev2 (WiFiNINA), the
 * MKR 1000 and the WiFi Shield 101 (WiFi101).
 *
 * The board header, such as `<PlynxSimpleWiFiNINA.h>`, creates it as the
 * global `Plynx`. The examples in examples/Boards_WiFi show the whole sketch
 * for each board.
 *
 * @see begin, PlynxProtocol::run
 */
class PlynxWifiCommon
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxWifiCommon(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    /**
     * @brief Joins a WiFi network, trying again every 10 seconds until it
     * works.
     *
     * begin() calls it for you. When the WiFi module does not answer, the
     * board stops with "WiFi module not found" on the PLYNX_PRINT port.
     *
     * @param ssid  The name of the WiFi network.
     * @param pass  The password, or an empty text for an open network.
     *
     * @warning With a wrong name or password it never returns.
     *
     * @see begin
     */
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
            if (pass && strlen(pass)) {
                status = WiFi.begin((char*)ssid, (char*)pass);
            } else {
                status = WiFi.begin((char*)ssid);
            }

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

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server, PLYNX_DEFAULT_PORT by default.
     *
     * Call it in setup(), before Plynx.connect(), when the sketch joins the WiFi by
     * itself.
     *
     * @see PlynxProtocol::connect, begin
     */
    void config(const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_SERVER_PORT)
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
     * @param port  The port of the server, PLYNX_DEFAULT_PORT by default.
     *
     * @see config
     */
    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = PLYNX_SERVER_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    /**
     * @brief Connects the board to the WiFi and to the Plynx server, and
     * waits until both are done.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param ssid    The name of the WiFi network.
     * @param pass    The password of the WiFi network.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server, PLYNX_DEFAULT_PORT by default.
     *
     * Call it in setup(), once. The sketch for each board is in
     * examples/Boards_WiFi, for example Arduino_MKR1010.
     *
     * @warning It waits without a time limit. With a wrong WiFi password or a
     * wrong token, setup() never ends. Turn on PLYNX_PRINT to read why.
     *
     * @see config, PlynxProtocol::run
     */
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

    /**
     * @brief Connects the board to the WiFi and to a Plynx server given by its
     * IP address, and waits until both are done.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ssid  The name of the WiFi network.
     * @param pass  The password of the WiFi network.
     * @param ip    The IP address of the server.
     * @param port  The port of the server, PLYNX_DEFAULT_PORT by default.
     *
     * @see begin
     */
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
