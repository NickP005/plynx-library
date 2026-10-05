/**
 * @file       PlynxEthernet.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxEthernet_h
#define PlynxEthernet_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "W5000"
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
 * @brief The Plynx object of a board on wired Ethernet, such as an Arduino
 * with the Ethernet Shield.
 *
 * The board header, such as `<PlynxSimpleEthernet.h>`, creates it as the
 * global `Plynx`. begin() gets an address from the router (DHCP) or uses the
 * one given, then connects to the server.
 *
 * @code
 * #include <SPI.h>
 * #include <Ethernet.h>
 * #include <PlynxSimpleEthernet.h>
 *
 * char auth[] = "YourAuthToken";
 *
 * void setup()
 * {
 *   Plynx.begin(auth);
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 * }
 * @endcode
 *
 * @see begin, PlynxProtocol::run
 */
class PlynxEthernet
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxEthernet(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    /**
     * @brief Sets the token and the server, without starting the Ethernet or
     * connecting.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server, PLYNX_DEFAULT_PORT by default.
     *
     * @par Where to call
     * In setup(), after Ethernet.begin() written by the sketch, and before
     * Plynx.connect().
     *
     * @code
     * #include <SPI.h>
     * #include <Ethernet.h>
     * #include <PlynxSimpleEthernet.h>
     *
     * byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
     *
     * void setup()
     * {
     *   Ethernet.begin(mac);
     *   Plynx.config("YourAuthToken");
     *   Plynx.connect();
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
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
     * connecting.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ip    The IP address of the server.
     * @param port  The port of the server, PLYNX_DEFAULT_PORT by default.
     *
     * @code
     * #include <SPI.h>
     * #include <Ethernet.h>
     * #include <PlynxSimpleEthernet.h>
     *
     * byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
     *
     * void setup()
     * {
     *   Ethernet.begin(mac);
     *   Plynx.config("YourAuthToken", IPAddress(192, 168, 1, 100), 8080);
     *   Plynx.connect();
     * }
     * @endcode
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

    // DHCP with domain
    /**
     * @brief Starts the Ethernet with an address from the router, connects to
     * the server, and waits until it is done.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server, PLYNX_DEFAULT_PORT by default.
     * @param mac     The MAC address of the shield. Without it the library
     *                makes one from the token.
     *
     * @par Where to call
     * In setup(), once.
     *
     * @code
     * #include <SPI.h>
     * #include <Ethernet.h>
     * #include <PlynxSimpleEthernet.h>
     *
     * char auth[] = "YourAuthToken";
     *
     * void setup()
     * {
     *   Plynx.begin(auth);
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @warning When the router gives no address, the board stops with "DHCP
     * Failed!" on the PLYNX_PRINT port. It waits without a time limit for
     * the server.
     *
     * @see config, PlynxProtocol::run
     */
    void begin( const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t port      = PLYNX_SERVER_PORT,
                const byte mac[]   = NULL)
    {
        PLYNX_LOG1(PLYNX_F("Getting IP..."));
        if (!Ethernet.begin(SelectMacAddress(auth, mac))) {
            PLYNX_FATAL(PLYNX_F("DHCP Failed!"));
        }
        // give the Ethernet shield a second to initialize:
        PlynxDelay(1000);
        IPAddress myip = Ethernet.localIP();
        PLYNX_LOG_IP("IP:", myip);

        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    // Static IP with domain
    /**
     * @brief Starts the Ethernet with a fixed address, connects to the server
     * by name, and waits until it is done.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param domain  The address of the server.
     * @param port    The port of the server.
     * @param local   The fixed IP address of the board.
     * @param dns     The DNS server that turns the name into an address.
     * @param mac     The MAC address of the shield, or NULL.
     *
     * @code
     * #include <SPI.h>
     * #include <Ethernet.h>
     * #include <PlynxSimpleEthernet.h>
     *
     * void setup()
     * {
     *   Plynx.begin("YourAuthToken", "yes.plynx.cc", 8080,
     *               IPAddress(192, 168, 1, 50), IPAddress(192, 168, 1, 1));
     * }
     * @endcode
     *
     * @see begin
     */
    void begin( const char* auth,
                const char* domain,
                uint16_t port,
                IPAddress local,
                IPAddress dns,
                const byte mac[] = NULL)
    {
        PLYNX_LOG1(PLYNX_F("Using static IP"));
        Ethernet.begin(SelectMacAddress(auth, mac), local, dns);
        // give the Ethernet shield a second to initialize:
        PlynxDelay(1000);
        IPAddress myip = Ethernet.localIP();
        PLYNX_LOG_IP("IP:", myip);

        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    // Static IP with domain, gateway, etc
    /**
     * @brief Starts the Ethernet with a fixed address, gateway and subnet,
     * connects to the server by name, and waits until it is done.
     *
     * @param auth     The auth token of the board, from the Plynx app.
     * @param domain   The address of the server.
     * @param port     The port of the server.
     * @param local    The fixed IP address of the board.
     * @param dns      The DNS server.
     * @param gateway  The router.
     * @param subnet   The subnet mask, usually 255.255.255.0.
     * @param mac      The MAC address of the shield, or NULL.
     *
     * @see begin
     */
    void begin( const char* auth,
                const char* domain,
                uint16_t port,
                IPAddress local,
                IPAddress dns,
                IPAddress gateway,
                IPAddress subnet,
                const byte mac[] = NULL)
    {
        PLYNX_LOG1(PLYNX_F("Using static IP"));
        Ethernet.begin(SelectMacAddress(auth, mac), local, dns, gateway, subnet);
        // give the Ethernet shield a second to initialize:
        PlynxDelay(1000);
        IPAddress myip = Ethernet.localIP();
        PLYNX_LOG_IP("IP:", myip);

        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    // DHCP with server IP
    /**
     * @brief Starts the Ethernet with an address from the router, connects to
     * a server given by its IP address, and waits until it is done.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param addr  The IP address of the server.
     * @param port  The port of the server, PLYNX_DEFAULT_PORT by default.
     * @param mac   The MAC address of the shield, or NULL.
     *
     * @code
     * #include <SPI.h>
     * #include <Ethernet.h>
     * #include <PlynxSimpleEthernet.h>
     *
     * void setup()
     * {
     *   Plynx.begin("YourAuthToken", IPAddress(192, 168, 1, 100), 8080);
     * }
     * @endcode
     *
     * @see begin
     */
    void begin( const char* auth,
                IPAddress addr,
                uint16_t port    = PLYNX_SERVER_PORT,
                const byte mac[] = NULL)
    {
        PLYNX_LOG1(PLYNX_F("Getting IP..."));
        if (!Ethernet.begin(SelectMacAddress(auth, mac))) {
            PLYNX_FATAL(PLYNX_F("DHCP Failed!"));
        }
        // give the Ethernet shield a second to initialize:
        PlynxDelay(1000);
        IPAddress myip = Ethernet.localIP();
        PLYNX_LOG_IP("IP:", myip);

        config(auth, addr, port);
        while(this->connect() != true) {}
    }

    // Static IP with server IP
    /**
     * @brief Starts the Ethernet with a fixed address, connects to a server
     * given by its IP address, and waits until it is done.
     *
     * @param auth   The auth token of the board, from the Plynx app.
     * @param addr   The IP address of the server.
     * @param port   The port of the server.
     * @param local  The fixed IP address of the board.
     * @param mac    The MAC address of the shield, or NULL.
     *
     * @see begin
     */
    void begin( const char* auth,
                IPAddress addr,
                uint16_t port,
                IPAddress local,
                const byte mac[] = NULL)
    {
        PLYNX_LOG1(PLYNX_F("Using static IP"));
        Ethernet.begin(SelectMacAddress(auth, mac), local);
        // give the Ethernet shield a second to initialize:
        PlynxDelay(1000);
        IPAddress myip = Ethernet.localIP();
        PLYNX_LOG_IP("IP:", myip);

        config(auth, addr, port);
        while(this->connect() != true) {}
    }

    // Static IP with server IP, DNS, gateway, etc
    /**
     * @brief Starts the Ethernet with a fixed address, gateway and subnet,
     * connects to a server given by its IP address, and waits until it is
     * done.
     *
     * @param auth     The auth token of the board, from the Plynx app.
     * @param addr     The IP address of the server.
     * @param port     The port of the server.
     * @param local    The fixed IP address of the board.
     * @param dns      The DNS server.
     * @param gateway  The router.
     * @param subnet   The subnet mask, usually 255.255.255.0.
     * @param mac      The MAC address of the shield, or NULL.
     *
     * @see begin
     */
    void begin( const char* auth,
                IPAddress addr,
                uint16_t port,
                IPAddress local,
                IPAddress dns,
                IPAddress gateway,
                IPAddress subnet,
                const byte mac[] = NULL)
    {
        PLYNX_LOG1(PLYNX_F("Using static IP"));
        Ethernet.begin(SelectMacAddress(auth, mac), local, dns, gateway, subnet);
        // give the Ethernet shield a second to initialize:
        PlynxDelay(1000);
        IPAddress myip = Ethernet.localIP();
        PLYNX_LOG_IP("IP:", myip);

        config(auth, addr, port);
        while(this->connect() != true) {}
    }

private:

    byte* SelectMacAddress(const char* token, const byte mac[])
    {
        if (mac != NULL) {
            return (byte*)mac;
        }

        macAddress[0] = 0xFE;
        macAddress[1] = 0xED;
        macAddress[2] = 0xBA;
        macAddress[3] = 0xFE;
        macAddress[4] = 0xFE;
        macAddress[5] = 0xED;

        int len = strlen(token);
        int mac_index = 1;
        for (int i=0; i<len; i++) {
            macAddress[mac_index++] ^= token[i];

            if (mac_index > 5) { mac_index = 1; }
        }
        /* PLYNX_LOG("MAC: %02X-%02X-%02X-%02X-%02X-%02X",
                  macAddress[0], macAddress[1],
                  macAddress[2], macAddress[3],
                  macAddress[4], macAddress[5]);
        */
        return macAddress;
    }

private:
    byte macAddress[6];

};

#endif
