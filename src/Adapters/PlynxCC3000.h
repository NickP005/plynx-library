/**
 * @file       PlynxCC3000.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef PlynxCC3000_h
#define PlynxCC3000_h

#define PLYNX_INFO_CONNECTION "CC3000"

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adafruit_CC3000.h>
#include <utility/netapp.h>

class PlynxTransportCC3000
{
public:
    PlynxTransportCC3000(Adafruit_CC3000& cc3000)
        : cc3000(cc3000)
        , host(NULL)
        , addr(0)
        , port(0)
    {}

    void begin(uint32_t a, uint16_t p) {
        host = NULL;
        port = p;
        addr = a;
    }

    void begin(const char* h, uint16_t p) {
        host = h;
        port = p;
    }

    bool connect() {
        if (host) {
            PLYNX_LOG2(PLYNX_F("Looking for "), host);
            while (addr == 0) {
                if (!cc3000.getHostByName((char*)host, &addr)) {
                    PLYNX_LOG1(PLYNX_F("Couldn't locate server"));
                    PlynxDelay(500);
                }
            }
        }
        uint8_t* a = (uint8_t*)&addr;
        PLYNX_LOG_IP_REV("Connecting to ", a);
        client = cc3000.connectTCP(addr, port);
        return client.connected();
    }

    void disconnect() { client.stop(); }

    size_t read(void* buf, size_t len) {
        return client.readBytes((char*)buf, len);
    }
    size_t write(const void* buf, size_t len) {
        return client.write((const uint8_t*)buf, len);
    }

    bool connected() { return client.connected(); }
    int available() { return client.available(); }

private:
    Adafruit_CC3000& cc3000;
    Adafruit_CC3000_Client client;
    const char* host;
    uint32_t    addr;
    uint16_t    port;
};

class PlynxCC3000
    : public PlynxProtocol<PlynxTransportCC3000>
{
    typedef PlynxProtocol<PlynxTransportCC3000> Base;
public:
    PlynxCC3000(Adafruit_CC3000& cc3000, PlynxTransportCC3000& transp)
        : Base(transp), cc3000(cc3000)
    {}

    /**
     * @brief Joins a WiFi network. begin() calls it for you.
     *
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param secmode   The WiFi security, such as WLAN_SEC_WPA2.
     *
     * @see begin
     */
    void connectWiFi(const char* ssid,
                     const char* pass,
                     uint8_t     secmode)
    {
        if (!cc3000.begin())
        {
            PLYNX_FATAL("Couldn't begin()! Check your wiring?");
        }

#if !defined(CC3000_TINY_DRIVER) && defined(PLYNX_DEBUG)
        uint8_t major, minor;
        if(!cc3000.getFirmwareVersion(&major, &minor))
        {
            if(major != 0x1 || minor < 0x13) {
                PLYNX_LOG1(PLYNX_F("CC3000 upgrade needed?"));
            }
        }
#endif

        // Remove socket inactivity timeout
        /*unsigned long aucDHCP       = 14400;
        unsigned long aucARP        = 3600;
        unsigned long aucKeepalive  = 30;
        unsigned long aucInactivity = 0;
        int iRet = netapp_timeout_values(&aucDHCP, &aucARP, &aucKeepalive, &aucInactivity);
        if (iRet != 0) {
            PLYNX_FATAL("Cannot set netapp timeout!");
        }*/

        /*if (!cc3000.deleteProfiles())
        {
            PLYNX_FATAL("Fail deleting old profiles");
        }*/
        PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
        if (!cc3000.connectToAP(ssid, pass, secmode))
        {
            PLYNX_FATAL("Failed to connect to AP");
        }
        PLYNX_LOG1(PLYNX_F("Getting IP..."));
        while (!cc3000.checkDHCP())
        {
            PlynxDelay(100);
        }

        uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
        if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
        {
            PLYNX_FATAL("DHCP failed.");
        }
#ifdef PLYNX_PRINT
        uint8_t* addr = (uint8_t*)&ipAddress;
        PLYNX_LOG_IP_REV("IP: ", addr);
        addr = (uint8_t*)&gateway;
        PLYNX_LOG_IP_REV("GW: ", addr);
        addr = (uint8_t*)&dnsserv;
        PLYNX_LOG_IP_REV("DNS: ", addr);
#endif
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
        this->conn.begin(cc3000.IP2U32(ip[0],ip[1],ip[2],ip[3]), port);
    }

    /**
     * @brief Connects a board with the CC3000 WiFi chip to the WiFi and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param secmode   The WiFi security, such as WLAN_SEC_WPA2.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/CC3000 and examples/Boards_WiFi/TinyDuino_WiFi.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin( const char* auth,
                const char* ssid,
                const char* pass,
                uint8_t     secmode,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass, secmode);
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    /**
     * @brief Connects a board with the CC3000 WiFi chip to the WiFi and to a Plynx server given by its IP address, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param secmode   The WiFi security, such as WLAN_SEC_WPA2.
     * @param ip        The IP address of the server.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/CC3000 and examples/Boards_WiFi/TinyDuino_WiFi.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin( const char* auth,
                const char* ssid,
                const char* pass,
                uint8_t     secmode,
                IPAddress   ip,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass, secmode);
        config(auth, ip, port);
        while(this->connect() != true) {}
    }
private:
    Adafruit_CC3000& cc3000;
};

#endif
