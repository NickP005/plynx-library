/**
 * @file       PlynxSimpleUserManaged.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jun 2015
 * @brief
 *
 */

#ifndef PlynxSimpleUserManaged_h
#define PlynxSimpleUserManaged_h

#warning "This user-defined connection is now deprecated."
#warning "Please use https://github.com/NickP005/plynx-library/tree/master/examples/More/ArduinoClient examples"

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "custom"
#endif

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>

extern size_t PlynxStreamRead(void* buf, size_t len);
extern size_t PlynxStreamWrite(const void* buf, size_t len);

class PlynxTransportUserDefined
{
public:
    PlynxTransportUserDefined()
        : mConn (false)
    {}

    // IP redirect not available
    void begin(char PLYNX_UNUSED *h, uint16_t PLYNX_UNUSED p) {}

    bool connect() {
        return mConn = true;
    }

    void disconnect() { mConn = false; }

    bool connected() {
        return mConn;
    }

    size_t read(void* buf, size_t len) {
        return PlynxStreamRead(buf, len);
    }

    size_t write(const void* buf, size_t len) {
        return PlynxStreamWrite(buf, len);
    }

    size_t available() {
        return 0;
    }

private:
    bool mConn;
};

class PlynxUserDefined
    : public PlynxProtocol<PlynxTransportUserDefined>
{
    typedef PlynxProtocol<PlynxTransportUserDefined> Base;
public:
    PlynxUserDefined(PlynxTransportUserDefined& transp)
        : Base(transp)
    {}

    void begin(const char* auth)
    {
        Base::begin(auth);
    }

private:
};

static PlynxTransportUserDefined _plynxTransport;
PlynxUserDefined Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
