/**
 * @file       PlynxSimpleRFduinoBLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       May 2016
 * @brief
 *
 */

#ifndef PlynxSimpleRFduinoBLE_h
#define PlynxSimpleRFduinoBLE_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "RFduinoBLE"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 20
//#define PLYNX_SEND_THROTTLE 20

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>
#include <RFduinoBLE.h>

class PlynxTransportRFduinoBLE
{
public:
    PlynxTransportRFduinoBLE()
        : mConn (false)
    {}

    // IP redirect not available
    void begin(char PLYNX_UNUSED *h, uint16_t PLYNX_UNUSED p) {}

    void begin() {
        instance = this;
    }

    bool connect() {
        mBuffRX.clear();
        return mConn = true;
    }

    void disconnect() {
        mConn = false;
    }

    bool connected() {
        return mConn;
    }

    size_t read(void* buf, size_t len) {
        millis_time_t start = PlynxMillis();
        while (PlynxMillis() - start < PLYNX_TIMEOUT_MS) {
            if (available() < len) {
                PlynxDelay(1);
            } else {
                break;
            }
        }
        noInterrupts();
        size_t res = mBuffRX.get((uint8_t*)buf, len);
        interrupts();
        return res;
    }

    size_t write(const void* buf, size_t len) {
        RFduinoBLE.send((const char*)buf, len);
        return len;
    }

    size_t available() {
        noInterrupts();
        size_t rxSize = mBuffRX.size();
        interrupts();
        return rxSize;
    }

    static
    int putData(uint8_t* data, uint16_t len) {
        if (!instance)
            return 0;
        noInterrupts();
        //PLYNX_DBG_DUMP(">> ", data, len);
        instance->mBuffRX.put(data, len);
        interrupts();
        return 0;
    }

private:
    static PlynxTransportRFduinoBLE* instance;

private:
    bool mConn;

    PlynxFifo<uint8_t, PLYNX_MAX_READBYTES*2> mBuffRX;
};

class PlynxSimpleRFduinoBLE
    : public PlynxProtocol<PlynxTransportRFduinoBLE>
{
    typedef PlynxProtocol<PlynxTransportRFduinoBLE> Base;
public:
    PlynxSimpleRFduinoBLE(PlynxTransportRFduinoBLE& transp)
        : Base(transp)
    {}

    void begin(const char* auth)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin();
    }
};

PlynxTransportRFduinoBLE* PlynxTransportRFduinoBLE::instance = NULL;

static PlynxTransportRFduinoBLE _plynxTransport;
PlynxSimpleRFduinoBLE Plynx(_plynxTransport);


void RFduinoBLE_onConnect()
{
  PLYNX_LOG1("Device connected");
  Plynx.startSession();
}

void RFduinoBLE_onDisconnect()
{
  PLYNX_LOG1("Device disconnected");
  Plynx.disconnect();
}

void RFduinoBLE_onReceive(char* data, int len)
{
  _plynxTransport.putData((uint8_t*)data, len);
}

#include <PlynxWidgets.h>

#endif
