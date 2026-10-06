/**
 * @file       PlynxSimpleEsp32_SSL.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Dec 2017
 * @brief
 *
 */

#ifndef PlynxSimpleEsp32_SSL_h
#define PlynxSimpleEsp32_SSL_h

#ifndef ESP32
#error This code is intended to run on the ESP32 platform! Please check your Tools->Board setting.
#endif

static const char PLYNX_DEFAULT_ROOT_CA[] =
#include <certs/letsencrypt_pem.h>

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
// connectWiFi() below uses WiFi.mode()/WiFi.begin(). WiFiClientSecure.h used to
// drag WiFi.h in on its own; on arduino-esp32 3.x it does not, and this shell
// stopped compiling at all -- which is why Plynx.Edgent, the only example built
// on it, could not be built either. The ESP8266 SSL shell always included it.
#include <WiFi.h>
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
     * @brief Sets the token and the server for an encrypted connection,
     * without connecting yet.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server. The default is PLYNX_DEFAULT_PORT_SSL.
     * @param root_ca  The certificate, in PEM text, that the server's certificate must come
     *                 from. The default is the Let's Encrypt root that the library
     *                 carries.
     *
     * @see begin, PlynxProtocol::connect
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
     * @brief Sets the token and the server by its IP address for an encrypted
     * connection, without connecting yet.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ip    The IP address of the server.
     * @param port  The port of the server. The default is PLYNX_DEFAULT_PORT_SSL.
     * @param root_ca  The certificate, in PEM text, that the server's certificate must come
     *                 from. The default is the Let's Encrypt root that the library
     *                 carries.
     *
     * @see config
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
     * @brief Connects the ESP32 to the WiFi and to the Plynx server over an
     * encrypted connection, and waits until both are done.
     *
     * The token and the values travel encrypted. The board checks the
     * certificate of the server before sending the token.
     *
     * @param auth    The auth token of the board, from the Plynx app.
     * @param ssid    The name of the WiFi network.
     * @param pass    The password of the WiFi network.
     * @param domain  The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port    The port of the server. The default is PLYNX_DEFAULT_PORT_SSL.
     * @param root_ca  The certificate, in PEM text, that the server's certificate must come
     *                 from. The default is the Let's Encrypt root that the library
     *                 carries.
     *
     * Call it in setup(), once.
     *
     * @code
     * #define PLYNX_PRINT Serial
     * #include <PlynxSimpleEsp32_SSL.h>
     *
     * void setup()
     * {
     *   Serial.begin(115200);
     *   Plynx.begin("YourAuthToken", "YourWiFiName", "YourWiFiPassword");
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @warning It waits without a time limit. When the certificate does not
     * match, the serial monitor shows "Secure connection failed" and the
     * board keeps trying.
     *
     * @see config, PlynxProtocol::run
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
     * @brief Connects the ESP32 to the WiFi and to a Plynx server given by its IP
     * address, over an encrypted connection.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ssid  The name of the WiFi network.
     * @param pass  The password of the WiFi network.
     * @param ip    The IP address of the server.
     * @param port  The port of the server. The default is PLYNX_DEFAULT_PORT_SSL.
     * @param root_ca  The certificate, in PEM text, that the server's certificate must come
     *                 from. The default is the Let's Encrypt root that the library
     *                 carries.
     *
     * @see begin
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
