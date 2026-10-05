/**
 * @file       PlynxWiFly.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxWiFly_h
#define PlynxWiFly_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "RN-XV"
#endif

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <WiFlyHQ.h>

class PlynxTransportWiFly
{
public:
    PlynxTransportWiFly()
        : wifly(NULL)
        , domain(NULL)
        , port(0)
    {}

    void setWiFly(WiFly* rnxv) {
        wifly = rnxv;
    }

    void begin(const char* h,  uint16_t p) {
        domain = h;
        port = p;
    }

    bool connect() {
        return wifly->open(domain, port);
    }
    void disconnect() { wifly->close(); }

    size_t read(void* buf, size_t len) {
        return wifly->readBytes((char*)buf, len);
    }
    size_t write(const void* buf, size_t len) {
        wifly->write((const uint8_t*)buf, len);
        return len;
    }

    bool connected() { return wifly->isConnected(); }
    int available() { return wifly->available(); }

private:
    WiFly*      wifly;
    const char* domain;
    uint16_t    port;
};

class PlynxWiFly
    : public PlynxProtocol<PlynxTransportWiFly>
{
    typedef PlynxProtocol<PlynxTransportWiFly> Base;
public:
    PlynxWiFly(PlynxTransportWiFly& transp)
        : Base(transp)
        , wifly(NULL)
    {}

    /**
     * @brief Joins a WiFi network. begin() calls it for you.
     *
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     *
     * @see begin
     */
    void connectWiFi(const char* ssid, const char* pass) {
        if (!wifly->isAssociated()) {
            PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
            wifly->setSSID(ssid);
            if (pass && strlen(pass)) {
                 wifly->setPassphrase(pass);
            }
            wifly->enableDHCP();
            if (!wifly->join()) {
                PLYNX_FATAL("Failed.");
            }
        } else {
            PLYNX_LOG1(PLYNX_F("Already connected to WiFi"));
        }
        if (wifly->isConnected()) {
            wifly->close();
        }
        wifly->setIpFlags(1 << 1);
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param rnxv      The WiFly module object of the sketch.
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(WiFly&      rnxv,
                const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        wifly = &rnxv;
        this->conn.setWiFly(wifly);
        this->conn.begin(domain, port);
    }

    /**
     * @brief Connects a board with an RN-XV WiFly module to the WiFi and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param rnxv      The WiFly module object of the sketch.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/RN_XV_WiFly.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin( const char* auth,
                WiFly&      rnxv,
                const char* ssid,
                const char* pass,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        wifly = &rnxv;
        connectWiFi(ssid, pass);
        config(rnxv, auth, domain, port);
        while(this->connect() != true) {}
    }

private:
    WiFly* wifly;
};

#endif
