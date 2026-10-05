/**
 * @file       PlynxArduinoGSM.h
 * @author     Riccardo Rizzo
 * @license    This project is released under the MIT License (MIT)
 * @date       Dec 2018
 * @brief
 *
 */

#ifndef PlynxArduinoGSM_h
#define PlynxArduinoGSM_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "MKRGSM"
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
#include <MKRGSM.h>

class PlynxSIM
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxSIM(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    bool connectNetwork(GSM& modem, GPRS& gprs, const char* pin ,const char* apn, const char* user, const char* pass)
    {
        PLYNX_LOG1(PLYNX_F("Modem init..."));
        if (!modem.begin(pin)) {
           PLYNX_FATAL(PLYNX_F("Cannot init"));
           return false;
        }

        PLYNX_LOG3(PLYNX_F("Connecting to "), apn, PLYNX_F(" ..."));
        if (!gprs.attachGPRS(apn, user, pass)) {
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
     * @param gprs      The GPRS object of the sketch.
     * @param gsmclient The GSM client object of the sketch.
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(GSM&        gsm,
                GPRS&       gprs,
                GSMClient&  gsmclient,
                const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        modem = &gsm;
        modemGPRS = &gprs;

        this->conn.setClient(&gsmclient);
        this->conn.begin(domain, port);
    }

    /**
     * @brief Connects an Arduino MKR GSM 1400 to the mobile network and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param gsm       The modem object of the sketch.
     * @param gprs      The GPRS object of the sketch.
     * @param gsmclient The GSM client object of the sketch.
     * @param pin       The PIN of the SIM card, or an empty text.
     * @param apn       The APN of the mobile operator.
     * @param user      The user name for the APN, often empty.
     * @param pass      The password for the APN, often empty.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_GSM/Arduino_MKRGSM.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin(const char* auth,
               GSM&        gsm,
               GPRS&       gprs,
               GSMClient&  gsmclient,
               const char* pin,
               const char* apn,
               const char* user,
               const char* pass,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        config(gsm, gprs, gsmclient, auth, domain, port);
        connectNetwork(gsm, gprs, pin, apn, user, pass);
        while(this->connect() != true) {}
    }

private:
    GSM*       modem;
    GPRS*      modemGPRS;
    GSMClient* gsmclient;
};

#endif
