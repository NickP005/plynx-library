/**
 * @file       PlynxGsmClient.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Nov 2016
 * @brief
 *
 */

#ifndef PlynxGsmClient_h
#define PlynxGsmClient_h

#ifndef PLYNX_INFO_CONNECTION
  #if defined(TINY_GSM_MODEM_SIM800)
    #define PLYNX_INFO_CONNECTION  "SIM800"
  #elif defined(TINY_GSM_MODEM_SIM900)
    #define PLYNX_INFO_CONNECTION  "SIM900"
  #elif defined(TINY_GSM_MODEM_UBLOX)
    #define PLYNX_INFO_CONNECTION  "UBLOX"
  #elif defined(TINY_GSM_MODEM_BG96)
    #define PLYNX_INFO_CONNECTION  "BG96"
  #elif defined(TINY_GSM_MODEM_A6)
    #define PLYNX_INFO_CONNECTION  "A6"
  #elif defined(TINY_GSM_MODEM_A7)
    #define PLYNX_INFO_CONNECTION  "A7"
  #elif defined(TINY_GSM_MODEM_M590)
    #define PLYNX_INFO_CONNECTION  "M590"
  #elif defined(TINY_GSM_MODEM_XBEE)
    #define PLYNX_INFO_CONNECTION  "XBEE"
  #else
    #define PLYNX_INFO_CONNECTION  "TinyGSM"
  #endif
#endif

#ifndef PLYNX_HEARTBEAT
#define PLYNX_HEARTBEAT 60
#endif

#ifndef PLYNX_TIMEOUT_MS
#define PLYNX_TIMEOUT_MS 6000
#endif

#define PLYNX_SEND_ATOMIC

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <TinyGsmClient.h>

class PlynxSIM
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxSIM(PlynxArduinoClient& transp)
        : Base(transp)
        , modem(NULL)
    {}

    bool connectNetwork(const char* apn, const char* user, const char* pass)
    {
        PLYNX_LOG1(PLYNX_F("Modem init..."));
        if (!modem->begin()) {
           PLYNX_FATAL(PLYNX_F("Cannot init"));
        }

        switch (modem->getSimStatus()) {
        case SIM_ERROR:  PLYNX_FATAL(PLYNX_F("SIM is missing"));    break;
        case SIM_LOCKED: PLYNX_FATAL(PLYNX_F("SIM is PIN-locked")); break;
        default: break;
        }

        PLYNX_LOG1(PLYNX_F("Connecting to network..."));
        if (modem->waitForNetwork()) {
          String op = modem->getOperator();
          PLYNX_LOG2(PLYNX_F("Network: "), op);
        } else {
          PLYNX_FATAL(PLYNX_F("Register in network failed"));
        }

        PLYNX_LOG3(PLYNX_F("Connecting to "), apn, PLYNX_F(" ..."));
        if (!modem->gprsConnect(apn, user, pass)) {
            PLYNX_FATAL(PLYNX_F("Connect GPRS failed"));
            return false;
        }

        PLYNX_LOG1(PLYNX_F("Connected to GPRS"));
        return true;
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param gsm       The modem object of the sketch.
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(TinyGsm&    gsm,
                const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        modem = &gsm;
        client.init(modem);
        this->conn.setClient(&client);
        this->conn.begin(domain, port);
    }

    /**
     * @brief Connects a board with a GSM modem driven by TinyGSM to the mobile network and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param gsm       The modem object of the sketch.
     * @param apn       The APN of the mobile operator.
     * @param user      The user name for the APN, often empty.
     * @param pass      The password for the APN, often empty.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * Call it in setup(), once. The whole sketch is in examples/Boards_GSM/TinyGSM_SIM800_SIM900.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin(const char* auth,
               TinyGsm&    gsm,
               const char* apn,
               const char* user,
               const char* pass,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        config(gsm, auth, domain, port);
        connectNetwork(apn, user, pass);
        while(this->connect() != true) {}
    }

private:
    TinyGsm*      modem;
    TinyGsmClient client;
};

#endif
