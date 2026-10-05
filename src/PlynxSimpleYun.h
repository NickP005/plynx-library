/**
 * @file       PlynxSimpleYun.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef PlynxSimpleYun_h
#define PlynxSimpleYun_h

#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <BridgeClient.h>

typedef PlynxArduinoClient PlynxArduinoClientYun;

class PlynxYun
    : public PlynxProtocol< PlynxArduinoClientYun >
{
    typedef PlynxProtocol< PlynxArduinoClientYun > Base;
public:
    PlynxYun(PlynxArduinoClientYun& transp)
        : Base(transp)
    {}

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
     * @brief Connects an Arduino Yun to the Plynx server, and waits until it is done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/Arduino_Yun.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin(const char* auth,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        PLYNX_LOG1(PLYNX_F("Bridge init..."));
        Bridge.begin();
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    /**
     * @brief Connects an Arduino Yun to a Plynx server given by its IP address, and waits until it is done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ip        The IP address of the server.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/Arduino_Yun.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin(const char* auth,
               IPAddress   ip,
               uint16_t    port = PLYNX_DEFAULT_PORT)
    {
        PLYNX_LOG1(PLYNX_F("Bridge init..."));
        Bridge.begin();
        config(auth, ip, port);
        while(this->connect() != true) {}
    }

};

static BridgeClient _plynxYunClient;
static PlynxArduinoClient _plynxTransport(_plynxYunClient);
PlynxYun Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
