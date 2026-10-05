/**
 * @file       PlynxArduinoNB.h
 * @author     Riccardo Rizzo
 * @license    This project is released under the MIT License (MIT)
 * @date       Dec 2018
 * @brief
 *
 */

#ifndef PlynxArduinoNB_h
#define PlynxArduinoNB_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "MKRNB"
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
#include <MKRNB.h>

class PlynxSIM
    : public PlynxProtocol<PlynxArduinoClient>
{
    typedef PlynxProtocol<PlynxArduinoClient> Base;
public:
    PlynxSIM(PlynxArduinoClient& transp)
        : Base(transp)
    {}

    bool connectNetwork(NB& modem, GPRS& gprs, const char* pin)
    {
        PLYNX_LOG1(PLYNX_F("Modem init..."));
        if (!modem.begin(pin)) {
           PLYNX_FATAL(PLYNX_F("Cannot init"));
           return false;
        }

        PLYNX_LOG3(PLYNX_F("Connecting to "),"", PLYNX_F(" ..."));
        if (!gprs.attachGPRS()) {
            PLYNX_FATAL(PLYNX_F("Connect GPRS failed"));
            return false;
        }

        PLYNX_LOG1(PLYNX_F("Connected to GPRS"));
        return true;
    }

    /**
     * @brief Sets the token and the server, without connecting yet.
     *
     * @param nb        The NB object of the sketch.
     * @param gprs      The GPRS object of the sketch.
     * @param nbclient  The NB client object of the sketch.
     * @param auth      The auth token of the board, from the Plynx app.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void config(NB&        nb,
                GPRS&       gprs,
                NBClient&  nbclient,
                const char* auth,
                const char* domain = PLYNX_DEFAULT_DOMAIN,
                uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        Base::begin(auth);
        modem = &nb;
        modemGPRS = &gprs;

        this->conn.setClient(&nbclient);
        this->conn.begin(domain, port);
    }

    /**
     * @brief Connects an Arduino MKR NB 1500 to the mobile network and to the Plynx server, and waits until both are done.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     * @param nb        The NB object of the sketch.
     * @param gprs      The GPRS object of the sketch.
     * @param nbclient  The NB client object of the sketch.
     * @param pin       The PIN of the SIM card, or an empty text.
     * @param domain    The address of the server. The default is PLYNX_DEFAULT_DOMAIN.
     * @param port      The port of the server.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_GSM/Arduino_MKRNB.
     *
     * @warning It waits without a time limit. Turn on PLYNX_PRINT to read
     * why when setup() does not end.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin(const char* auth,
               NB&        nb,
               GPRS&       gprs,
               NBClient&  nbclient,
               const char* pin,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        config(nb, gprs, nbclient, auth, domain, port);
        connectNetwork(nb, gprs, pin);
        while(this->connect() != true) {}
    }

private:
    NB*       modem;
    GPRS*     modemGPRS;
    NBClient* nbclient;
};

#endif
