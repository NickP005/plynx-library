/**
 * @file       PlynxWildFire.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef PlynxWildFire_h
#define PlynxWildFire_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "CC3000"
#endif

#define PLYNX_SEND_ATOMIC

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <IPAddress.h>
#include <WildFire_CC3000.h>
#include <utility/netapp.h>

class PlynxTransportWildFire
{
public:
    PlynxTransportWildFire(WildFire_CC3000& cc3000)
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

    void disconnect() { client.close(); }

    size_t read(void* buf, size_t len) {
        char* beg = (char*)buf;
        char* end = beg + len;
        while (beg < end) {
            int c = client.read();
            if (c < 0)
                break;
            *beg++ = (char)c;
        }
        return len;
    }
    size_t write(const void* buf, size_t len) {
        return client.write((const uint8_t*)buf, len);
    }

    bool connected() { return client.connected(); }
    int available() { return client.available(); }

private:
    WildFire_CC3000& cc3000;
    WildFire_CC3000_Client client;
    const char* host;
    uint32_t    addr;
    uint16_t    port;
};

class PlynxWildFire
    : public PlynxProtocol<PlynxTransportWildFire>
{
    typedef PlynxProtocol<PlynxTransportWildFire> Base;
public:
    PlynxWildFire(WildFire_CC3000& cc3000, PlynxTransportWildFire& transp)
        : Base(transp), cc3000(cc3000)
    {}

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
        this->conn.begin(cc3000.IP2U32(ip[0],ip[1],ip[2],ip[3]), port);
    }

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
    WildFire_CC3000& cc3000;
};

#endif
