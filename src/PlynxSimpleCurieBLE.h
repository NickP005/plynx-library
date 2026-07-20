/**
 * @file       PlynxSimpleCurieBLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       May 2016
 * @brief
 *
 */

#ifndef PlynxSimpleCurieBLE_h
#define PlynxSimpleCurieBLE_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "CurieBLE"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 20
//#define PLYNX_SEND_THROTTLE 20

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>
#include <CurieBLE.h>

class PlynxTransportCurieBLE
{
public:
    PlynxTransportCurieBLE()
        : mConn (false)
    {}

    // IP redirect not available
    void begin(char PLYNX_UNUSED *h, uint16_t PLYNX_UNUSED p) {}

    void begin(BLEPeripheral& per) {
        instance = this;

        blePeripheral = &per;
        // Add service and characteristic
        blePeripheral->setAdvertisedServiceUuid(bleService.uuid());
        blePeripheral->addAttribute(bleService);
        blePeripheral->addAttribute(rxChar);
        blePeripheral->addAttribute(txChar);

        unsigned char empty[0] = {};
        rxChar.setValue(empty, 0);
        txChar.setValue(empty, 0);

        // Assign event handlers for connected, disconnected to peripheral
        blePeripheral->setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
        rxChar.setEventHandler(BLEWritten,    rxCharWritten);
        txChar.setEventHandler(BLESubscribed, txCharSubscribed);
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
        uint32_t start = millis();
        while (millis() - start < PLYNX_TIMEOUT_MS) {
            if (available() < len) {
                blePeripheral->poll();
            } else {
                break;
            }
        }
        uint32_t key = interrupt_lock();
        size_t res = mBuffRX.get((uint8_t*)buf, len);
        interrupt_unlock(key);
        return res;
    }

    size_t write(const void* buf, size_t len) {
        txChar.setValue((uint8_t*)buf, len);
        return len;
    }

    size_t available() {
        uint32_t key = interrupt_lock();
        size_t rxSize = mBuffRX.size();
        interrupt_unlock(key);
        return rxSize;
    }

private:
    static PlynxTransportCurieBLE* instance;

    static
    void rxCharWritten(BLECentral& central, BLECharacteristic& ch) {
        if (!instance)
            return;
        uint32_t key = interrupt_lock();
        const uint8_t* data = ch.value();
        uint32_t len = ch.valueLength();
        //PLYNX_DBG_DUMP(">> ", data, len);
        instance->mBuffRX.put(data, len);
        interrupt_unlock(key);
    }

    static
    void txCharSubscribed(BLECentral& central, BLECharacteristic& ch);

    static
    void blePeripheralDisconnectHandler(BLECentral& central);

private:
    bool mConn;
    BLEPeripheral*    blePeripheral = NULL;
    BLEService        bleService = BLEService       ("713D0000-503E-4C75-BA94-3148F18D941E");
    BLECharacteristic rxChar     = BLECharacteristic("713D0003-503E-4C75-BA94-3148F18D941E", BLEWrite | BLEWriteWithoutResponse, BLE_MAX_ATTR_DATA_LEN);
    BLECharacteristic txChar     = BLECharacteristic("713D0002-503E-4C75-BA94-3148F18D941E", BLERead  | BLENotify,               BLE_MAX_ATTR_DATA_LEN);

    PlynxFifo<uint8_t, PLYNX_MAX_READBYTES*2> mBuffRX;
};

class PlynxCurieBLE
    : public PlynxProtocol<PlynxTransportCurieBLE>
{
    typedef PlynxProtocol<PlynxTransportCurieBLE> Base;
public:
    PlynxCurieBLE(PlynxTransportCurieBLE& transp)
        : Base(transp)
    {}

    void begin(BLEPeripheral& per, const char* auth)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin(per);
    }

    // Please use Plynx.begin(BLEPeripheral, "auth")
    PLYNX_DEPRECATED
    void begin(const char* auth, BLEPeripheral& per)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin(per);
    }
};

PlynxTransportCurieBLE* PlynxTransportCurieBLE::instance = NULL;

static PlynxTransportCurieBLE _plynxTransport;
PlynxCurieBLE Plynx(_plynxTransport);

inline
void PlynxTransportCurieBLE::txCharSubscribed(BLECentral& central, BLECharacteristic& ch) {
    Plynx.startSession();
}

inline
void PlynxTransportCurieBLE::blePeripheralDisconnectHandler(BLECentral& central) {
    Plynx.disconnect();
}

#include <PlynxWidgets.h>

#endif
