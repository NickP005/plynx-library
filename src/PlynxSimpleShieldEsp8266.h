/**
 * @file       PlynxSimpleShieldEsp8266.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jun 2015
 * @brief
 *
 */

#ifndef PlynxSimpleShieldEsp8266_h
#define PlynxSimpleShieldEsp8266_h

#ifdef ESP8266
#error This code is not intended to run on the ESP8266 platform! Please check your Tools->Board setting.
#endif

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "ESP8266"
#endif

#ifndef PLYNX_ESP8266_MUX
#define PLYNX_ESP8266_MUX  1
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 40

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>
#include <ESP8266_Lib.h>

class PlynxTransportShieldEsp8266
{
    static void onData(uint8_t mux_id, uint32_t len, void* ptr) {
        ((PlynxTransportShieldEsp8266*)ptr)->onData(mux_id, len);
    }

    void onData(uint8_t mux_id, int32_t len) {
        if (mux_id != PLYNX_ESP8266_MUX) {
            return;
        }
        //PLYNX_LOG4("Got: ", len, ", Free: ", buffer.free());
        if (buffer.free() < len) {
          PLYNX_LOG1(PLYNX_F("Buffer overflow"));
          return;
        }
        while (len) {
            if (client->getUart()->available()) {
                uint8_t b = client->getUart()->read();
                buffer.put(b);
                len--;
            }
        }
    }

public:
    PlynxTransportShieldEsp8266()
        : client(NULL)
        , status(false)
        , domain(NULL)
        , port(0)
    {}

    void setEsp8266(ESP8266* esp8266) {
        client = esp8266;
        client->setOnData(onData, this);
    }

    //TODO: IPAddress

    void begin(const char* d,  uint16_t p) {
        domain = d;
        port = p;
    }

    bool connect() {
        if (!domain || !port)
            return false;
        status = client->createTCP(PLYNX_ESP8266_MUX, domain, port);
        return status;
    }

    void disconnect() {
        status = false;
        buffer.clear();
        client->releaseTCP(PLYNX_ESP8266_MUX);
    }

    size_t read(void* buf, size_t len) {
        millis_time_t start = PlynxMillis();
        //PLYNX_LOG4("Waiting: ", len, " Buffer: ", buffer.size());
        while ((buffer.size() < len) && (PlynxMillis() - start < 1500)) {
            client->run();
        }
        return buffer.get((uint8_t*)buf, len);
    }
    size_t write(const void* buf, size_t len) {
        if (client->send(PLYNX_ESP8266_MUX, (const uint8_t*)buf, len)) {
            return len;
        }
        return 0;
    }

    bool connected() { return status; }

    int available() {
        client->run();
        //PLYNX_LOG2("Still: ", buffer.size());
        return buffer.size();
    }

private:
    ESP8266* client;
    bool status;
    PlynxFifo<uint8_t,256> buffer;
    const char* domain;
    uint16_t    port;
};

class PlynxWifi
    : public PlynxProtocol<PlynxTransportShieldEsp8266>
{
    typedef PlynxProtocol<PlynxTransportShieldEsp8266> Base;
public:
    PlynxWifi(PlynxTransportShieldEsp8266& transp)
        : Base(transp)
        , wifi(NULL)
    {}

    bool connectWiFi(const char* ssid, const char* pass)
    {
        PlynxDelay(500);
        PLYNX_LOG2(PLYNX_F("Connecting to "), ssid);
        /*if (!wifi->restart()) {
            PLYNX_LOG1(PLYNX_F("Failed to restart"));
            return false;
        }*/
        if (!wifi->kick()) {
             PLYNX_LOG1(PLYNX_F("ESP is not responding"));
             //TODO: PLYNX_LOG_TROUBLE(PLYNX_F("esp8266-not-responding"));
             return false;
        }
        if (!wifi->setEcho(0)) {
            PLYNX_LOG1(PLYNX_F("Failed to disable Echo"));
            return false;
        }
        String ver = wifi->ESP8266::getVersion();
        PLYNX_LOG1(ver);
        if (!wifi->enableMUX()) {
            PLYNX_LOG1(PLYNX_F("Failed to enable MUX"));
        }
        if (!wifi->setOprToStation()) {
            PLYNX_LOG1(PLYNX_F("Failed to set STA mode"));
            return false;
        }
        if (wifi->joinAP(ssid, pass)) {
            String my_ip = wifi->getLocalIP();
            PLYNX_LOG1(my_ip);
        } else {
            PLYNX_LOG1(PLYNX_F("Failed to connect WiFi"));
            return false;
        }
        PLYNX_LOG1(PLYNX_F("Connected to WiFi"));
        return true;
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param esp8266   The ESP8266 object of the ESP8266_Lib library.
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(ESP8266&    esp8266,
                const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        wifi = &esp8266;
        this->conn.setEsp8266(wifi);
        this->conn.begin(domain, port);
    }

    /**
     * @brief Connects a board that uses an ESP8266 as a WiFi modem (AT firmware) to the WiFi and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param esp8266   The ESP8266 object of the ESP8266_Lib library.
     * @param ssid      The name of the WiFi network.
     * @param pass      The password of the WiFi network.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_WiFi/ESP8266_Shield.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin(const char* auth,
               ESP8266&    esp8266,
               const char* ssid,
               const char* pass,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        config(esp8266, auth, domain, port);
        connectWiFi(ssid, pass);
        while(this->connect() != true) {}
    }

private:
    ESP8266* wifi;
};

static PlynxTransportShieldEsp8266 _plynxTransport;
PlynxWifi Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
