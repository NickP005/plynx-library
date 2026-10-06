/**
 * @file       PlynxSimpleWioTerminal_SSL.h
 * @author     Dmitry Maslov (Seeed Studio)
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2021 Dmitry Maslov
 * @date       June 2021
 * @brief
 *
 */

#ifndef PlynxSimpleWioTerminal_h
#define PlynxSimpleWioTerminal_h

#ifndef SEEED_WIO_TERMINAL
#error This code is intended to run on the Wio Terminal platform! Please check your Tools->Board setting.
#endif

static const char PLYNX_DEFAULT_ROOT_CA[] =
#include <certs/letsencrypt_pem.h>

#define PLYNX_SEND_ATOMIC

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <rpcWiFi.h>
#include <WiFiClientSecure.h>

template <typename Client>
class PlynxArduinoClientSecure
    : public PlynxArduinoClientGen<Client>
{
public:
    PlynxArduinoClientSecure(Client& client)
        : PlynxArduinoClientGen<Client>(client)
        , caCert(NULL)
    {}

    void setRootCA(const char* fp) { caCert = fp; }

    bool connect() {
        this->client->setCACert(caCert);
        if (PlynxArduinoClientGen<Client>::connect()) {
          PLYNX_LOG1(PLYNX_F("Certificate OK"));
          return true;
        } else {
          PLYNX_LOG1(PLYNX_F("Secure connection failed"));
        }
        return false;
    }

private:
    const char* caCert;
};


template <typename Transport>
class PlynxWifi
    : public PlynxProtocol<Transport>
{
    typedef PlynxProtocol<Transport> Base;
public:
    PlynxWifi(Transport& transp)
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
        WiFi.mode(WIFI_STA);
        if (pass && strlen(pass)) {
            WiFi.begin(ssid, pass);
        } else {
            WiFi.begin(ssid);
        }
        while (WiFi.status() != WL_CONNECTED) {
            PlynxDelay(500);
        }
        PLYNX_LOG1(PLYNX_F("Connected to WiFi"));

        IPAddress myip = WiFi.localIP();
        (void)myip; // Eliminate warnings about unused myip
        PLYNX_LOG_IP("IP: ", myip);
    }


    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     * @param root_ca   The certificate, in PEM text, that the server's certificate must come from.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT_SSL,
                const char* root_ca = PLYNX_DEFAULT_ROOT_CA)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
        this->conn.setRootCA(root_ca);
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ip        The IP address of the server.
     * @param port      The port of the server.
     * @param root_ca   The certificate, in PEM text, that the server's certificate must come from.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = PLYNX_DEFAULT_PORT_SSL,
                const char* root_ca = PLYNX_DEFAULT_ROOT_CA)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
        this->conn.setRootCA(root_ca);
    }

    /**
     * @brief Connects a Wio Terminal over an encrypted connection to the WiFi and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     * @param root_ca   The certificate, in PEM text, that the server's certificate must come from.
     *
     * Call it in setup(), once. The whole sketch is in examples/Plynx.Edgent/Edgent_Wio_Terminal.
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
               uint16_t    port   = PLYNX_DEFAULT_PORT_SSL,
               const char* root_ca = PLYNX_DEFAULT_ROOT_CA)
    {
        connectWiFi(ssid, pass);
        config(auth, domain, port, root_ca);
        while(this->connect() != true) {}
    }

    /**
     * @brief Connects a Wio Terminal over an encrypted connection to the WiFi and to a Plynx server given by its IP address, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param ip        The IP address of the server.
     * @param port      The port of the server.
     * @param root_ca   The certificate, in PEM text, that the server's certificate must come from.
     *
     * Call it in setup(), once. The whole sketch is in examples/Plynx.Edgent/Edgent_Wio_Terminal.
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
               uint16_t    port   = PLYNX_DEFAULT_PORT_SSL,
               const char* root_ca = PLYNX_DEFAULT_ROOT_CA)
    {
        connectWiFi(ssid, pass);
        config(auth, ip, port, root_ca);
        while(this->connect() != true) {}
    }

};

static WiFiClientSecure _plynxWifiClient;
static PlynxArduinoClientSecure<WiFiClientSecure> _plynxTransport(_plynxWifiClient);
PlynxWifi<PlynxArduinoClientSecure<WiFiClientSecure> > Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
