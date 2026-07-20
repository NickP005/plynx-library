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
