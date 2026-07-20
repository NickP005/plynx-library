/**
 * @file       PlynxSimpleEsp32_NimBLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Nov 2017
 * @brief
 *
 */

#ifndef PlynxSimpleEsp32_NimBLE_h
#define PlynxSimpleEsp32_NimBLE_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "Esp32_NimBLE"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 20
//#define PLYNX_SEND_THROTTLE 20

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>

#include <NimBLEDevice.h>
#include <NimBLEServer.h>
#include <NimBLEUtils.h>

#define SERVICE_UUID           "713D0000-503E-4C75-BA94-3148F18D941E"
#define CHARACTERISTIC_UUID_RX "713D0003-503E-4C75-BA94-3148F18D941E"
#define CHARACTERISTIC_UUID_TX "713D0002-503E-4C75-BA94-3148F18D941E"

class PlynxTransportEsp32_NimBLE :
    public NimBLEServerCallbacks,
    public BLECharacteristicCallbacks
{

public:
    PlynxTransportEsp32_NimBLE()
        : mConn (false)
        , mName ("Plynx")
    {}

    void setDeviceName(const char* name) {
        mName = name;
    }

    // IP redirect not available
    void begin(char PLYNX_UNUSED *h, uint16_t PLYNX_UNUSED p) {}

    void begin() {
        // Create the BLE Device
        NimBLEDevice::init(mName);

        // Create the BLE Server
        pServer = NimBLEDevice::createServer();
        pServer->setCallbacks(this);

        // Create the BLE Service
        pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic
        pCharacteristicTX = pService->createCharacteristic(
                            CHARACTERISTIC_UUID_TX,
                            NIMBLE_PROPERTY::NOTIFY
                          );

        pCharacteristicRX = pService->createCharacteristic(
                                              CHARACTERISTIC_UUID_RX,
                                              NIMBLE_PROPERTY::WRITE
                                            );

        pCharacteristicRX->setCallbacks(this);

        auto pSecurity = new NimBLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);

        // Start the service
        pService->start();

        // Start advertising
        pServer->getAdvertising()->addServiceUUID(pService->getUUID());
        pServer->getAdvertising()->setAppearance(0x00);
        pServer->getAdvertising()->setScanResponse(true);
        pServer->getAdvertising()->start();
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
                delay(1);
            } else {
                break;
            }
        }
        size_t res = mBuffRX.get((uint8_t*)buf, len);
        return res;
    }

    size_t write(const void* buf, size_t len) {
        pCharacteristicTX->setValue((uint8_t*)buf, len);
        pCharacteristicTX->notify();
        return len;
    }

    size_t available() {
        size_t rxSize = mBuffRX.size();
        return rxSize;
    }

private:

    void onConnect(NimBLEServer* pServer);
    void onDisconnect(NimBLEServer* pServer);

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        uint8_t* data = (uint8_t*)rxValue.data();
        size_t len = rxValue.length();

        PLYNX_DBG_DUMP(">> ", data, len);
        mBuffRX.put(data, len);
      }
    }

private:
    bool mConn;
    const char* mName;

    NimBLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristicTX;
    BLECharacteristic *pCharacteristicRX;

    PlynxFifo<uint8_t, PLYNX_MAX_READBYTES*2> mBuffRX;
};

class PlynxEsp32_NimBLE
    : public PlynxProtocol<PlynxTransportEsp32_NimBLE>
{
    typedef PlynxProtocol<PlynxTransportEsp32_NimBLE> Base;
public:
    PlynxEsp32_NimBLE(PlynxTransportEsp32_NimBLE& transp)
        : Base(transp)

    {}

    void begin(const char* auth)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin();
    }

    void setDeviceName(const char* name) {
        conn.setDeviceName(name);
    }

};


static PlynxTransportEsp32_NimBLE _plynxTransportBLE;
PlynxEsp32_NimBLE Plynx(_plynxTransportBLE);

inline
void PlynxTransportEsp32_NimBLE::onConnect(NimBLEServer* pServer) {
  PLYNX_LOG1(PLYNX_F("BLE connect"));
  connect();
  Plynx.startSession();
};

inline
void PlynxTransportEsp32_NimBLE::onDisconnect(NimBLEServer* pServer) {
  PLYNX_LOG1(PLYNX_F("BLE disconnect"));
  Plynx.disconnect();
  disconnect();
}


#include <PlynxWidgets.h>

#endif
