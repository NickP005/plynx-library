/**
 * @file       PlynxSimpleYun.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef PlynxSimpleYun_h
#define PlynxSimpleYun_h

#include <Plynx/PlynxProtocol.h>
#include <Adapters/PlynxArduinoClient.h>
#include <BridgeClient.h>

typedef PlynxArduinoClient PlynxArduinoClientYun;

class PlynxYun
    : public PlynxProtocol< PlynxArduinoClientYun >
{
    typedef PlynxProtocol< PlynxArduinoClientYun > Base;
public:
    PlynxYun(PlynxArduinoClientYun& transp)
        : Base(transp)
    {}

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
        this->conn.begin(ip, port);
    }

    void begin(const char* auth,
               const char* domain = PLYNX_DEFAULT_DOMAIN,
               uint16_t    port   = PLYNX_DEFAULT_PORT)
    {
        PLYNX_LOG1(PLYNX_F("Bridge init..."));
        Bridge.begin();
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    void begin(const char* auth,
               IPAddress   ip,
               uint16_t    port = PLYNX_DEFAULT_PORT)
    {
        PLYNX_LOG1(PLYNX_F("Bridge init..."));
        Bridge.begin();
        config(auth, ip, port);
        while(this->connect() != true) {}
    }

};

static BridgeClient _plynxYunClient;
static PlynxArduinoClient _plynxTransport(_plynxYunClient);
PlynxYun Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
