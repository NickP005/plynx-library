/**
 * @file       PlynxSimpleEsp8266_SSL.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2016
 * @brief
 *
 */

#ifndef PlynxSimpleEsp8266_SSL_h
#define PlynxSimpleEsp8266_SSL_h

#ifndef ESP8266
#error This code is intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif

#include <version.h>

#if ESP_SDK_VERSION_NUMBER < 0x020200
#error Please update your ESP8266 Arduino Core
#endif

static const char PLYNX_DEFAULT_ROOT_CA[] PROGMEM =
#include <certs/letsencrypt_pem.h>

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

#ifndef wificlientbearssl_h
#error BearSSL is needed, please update your ESP8266 Arduino Core
#endif
#ifdef USING_AXTLS
#error BearSSL is needed, but USING_AXTLS is defined
#endif

#ifndef PLYNX_SSL_RX_BUF_SIZE
#define PLYNX_SSL_RX_BUF_SIZE 2048
#endif

#ifndef PLYNX_SSL_TX_BUF_SIZE
#define PLYNX_SSL_TX_BUF_SIZE 512
#endif


static X509List PlynxCert(PLYNX_DEFAULT_ROOT_CA);

template <typename Client>
class PlynxArduinoClientSecure
    : public PlynxArduinoClientGen<Client>
{
public:
    PlynxArduinoClientSecure(Client& client)
        : PlynxArduinoClientGen<Client>(client)
    {
        this->client->setBufferSizes(PLYNX_SSL_RX_BUF_SIZE, PLYNX_SSL_TX_BUF_SIZE);
    }

    void setFingerprint(const char* fp) {
        this->client->setFingerprint(fp);
    }

    void setTrustAnchors(X509List* certs) {
        this->client->setTrustAnchors(certs);
    }

    bool connect() {

        time_t now = time(nullptr);
        if (now < 100000) {
            // Synchronize time useing SNTP. This is necessary to verify that
            // the TLS certificates offered by the server are currently valid.
            configTime(0, 0, "pool.ntp.org", "time.nist.gov");

            while (now < 100000) {
                delay(100);
                now = time(nullptr);
            }
        }
        struct tm timeinfo;
        gmtime_r(&now, &timeinfo);
        String ntpTime = asctime(&timeinfo);
        ntpTime.trim();
        PLYNX_LOG2("Current time: ", ntpTime);

        if (!PlynxArduinoClientGen<Client>::connect()) {
            int err = this->client->getLastSSLError();
            if (err == 0) {
                PLYNX_LOG1("Connection failed");
            } else {
                PLYNX_LOG2(PLYNX_F("SSL error: "), err);
            }
            return false;
        }
        return true;
    }

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
        if (WiFi.status() != WL_CONNECTED) {
            if (pass && strlen(pass)) {
                WiFi.begin(ssid, pass);
            } else {
                WiFi.begin(ssid);
            }
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
     * @param fingerprint  The SHA-1 fingerprint of the server certificate. Without it the
     *                     board checks the certificate against the root the library
     *                     carries.
     *
     * @see begin, PlynxProtocol::connect
     */
    void config(const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT_SSL,
                const char* fingerprint = NULL)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);

        if (fingerprint) {
          this->conn.setFingerprint(fingerprint);
        } else {
          this->conn.setTrustAnchors(&PlynxCert);
        }
    }

    /**
     * @brief Sets the token and the server by its IP address for an encrypted
     * connection, without connecting yet.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ip    The IP address of the server.
     * @param port  The port of the server. The default is PLYNX_DEFAULT_PORT_SSL.
     * @param fingerprint  The SHA-1 fingerprint of the server certificate. Without it the
     *                     board checks the certificate against the root the library
     *                     carries.
     *
     * @see config
     */
    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = PLYNX_DEFAULT_PORT_SSL,
                const char* fingerprint = NULL)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);

        if (fingerprint) {
          this->conn.setFingerprint(fingerprint);
        } else {
          this->conn.setTrustAnchors(&PlynxCert);
        }
    }

    /**
     * @brief Connects the ESP8266 to the WiFi and to the Plynx server over an
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
     * @param fingerprint  The SHA-1 fingerprint of the server certificate. Without it the
     *                     board checks the certificate against the root the library
     *                     carries.
     *
     * @par Where to call
     * In setup(), once.
     *
     * @code
     * #define PLYNX_PRINT Serial
     * #include <PlynxSimpleEsp8266_SSL.h>
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
               const char* fingerprint = NULL)
    {
        connectWiFi(ssid, pass);
        config(auth, domain, port, fingerprint);
        while(this->connect() != true) {}
    }

    /**
     * @brief Connects the ESP8266 to the WiFi and to a Plynx server given by its IP
     * address, over an encrypted connection.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     * @param ssid  The name of the WiFi network.
     * @param pass  The password of the WiFi network.
     * @param ip    The IP address of the server.
     * @param port  The port of the server. The default is PLYNX_DEFAULT_PORT_SSL.
     * @param fingerprint  The SHA-1 fingerprint of the server certificate. Without it the
     *                     board checks the certificate against the root the library
     *                     carries.
     *
     * @see begin
     */
    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               IPAddress   ip,
               uint16_t    port   = PLYNX_DEFAULT_PORT_SSL,
               const char* fingerprint = NULL)
    {
        connectWiFi(ssid, pass);
        config(auth, ip, port, fingerprint);
        while(this->connect() != true) {}
    }

};

static WiFiClientSecure _plynxWifiClient;
static PlynxArduinoClientSecure<WiFiClientSecure> _plynxTransport(_plynxWifiClient);
PlynxWifi<PlynxArduinoClientSecure<WiFiClientSecure> > Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
