/**
 * @file       PlynxSimpleSimbleeBLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       May 2016
 * @brief
 *
 */

#ifndef PlynxSimpleSimbleeBLE_h
#define PlynxSimpleSimbleeBLE_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "SimbleeBLE"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 20
//#define PLYNX_SEND_THROTTLE 20

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>
#include <SimbleeBLE.h>

class PlynxTransportSimbleeBLE
{
public:
    PlynxTransportSimbleeBLE()
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
        SimbleeBLE.send((const char*)buf, len);
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
    static PlynxTransportSimbleeBLE* instance;

private:
    bool mConn;

    PlynxFifo<uint8_t, PLYNX_MAX_READBYTES*2> mBuffRX;
};

class PlynxSimpleSimbleeBLE
    : public PlynxProtocol<PlynxTransportSimbleeBLE>
{
    typedef PlynxProtocol<PlynxTransportSimbleeBLE> Base;
public:
    PlynxSimpleSimbleeBLE(PlynxTransportSimbleeBLE& transp)
        : Base(transp)
    {}

    /**
     * @brief Starts the connection of a Simblee over Bluetooth Low Energy with the token of the board.
     *
     * @param auth      The auth token of the board, from the Plynx app.
     *
     * @par Where to call
     * In setup(), once. The whole sketch is in examples/Boards_Bluetooth/Simblee_BLE.
     *
     * @see PlynxProtocol::run, PlynxProtocol::connected
     */
    void begin(const char* auth)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin();
    }
};

PlynxTransportSimbleeBLE* PlynxTransportSimbleeBLE::instance = NULL;

static PlynxTransportSimbleeBLE _plynxTransport;
PlynxSimpleSimbleeBLE Plynx(_plynxTransport);


void SimbleeBLE_onConnect()
{
  PLYNX_LOG1("Device connected");
  Plynx.startSession();
}

void SimbleeBLE_onDisconnect()
{
  PLYNX_LOG1("Device disconnected");
  Plynx.disconnect();
}

void SimbleeBLE_onReceive(char* data, int len)
{
  _plynxTransport.putData((uint8_t*)data, len);
}

#include <PlynxWidgets.h>

#endif
