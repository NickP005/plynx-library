/**
 * @file       PlynxSimpleEsp32_BLE.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Nov 2017
 * @brief
 *
 */

#ifndef PlynxSimpleEsp32_BLE_h
#define PlynxSimpleEsp32_BLE_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "Esp32_BLE"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 20
#define PLYNX_SEND_THROTTLE 20

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID           "713D0000-503E-4C75-BA94-3148F18D941E"
#define CHARACTERISTIC_UUID_RX "713D0003-503E-4C75-BA94-3148F18D941E"
#define CHARACTERISTIC_UUID_TX "713D0002-503E-4C75-BA94-3148F18D941E"

class PlynxTransportEsp32_BLE :
    public BLEServerCallbacks,
    public BLECharacteristicCallbacks
{

public:
    PlynxTransportEsp32_BLE()
        : mConn (false)
        , mDropped (false)
        , mName ("Plynx")
    {}

    void setDeviceName(const char* name) {
        mName = name;
    }

    // IP redirect not available
    void begin(char PLYNX_UNUSED *h, uint16_t PLYNX_UNUSED p) {}

    void begin() {
        // Create the BLE Device
        BLEDevice::init(mName);

        // Create the BLE Server
        pServer = BLEDevice::createServer();
        pServer->setCallbacks(this);

        // Create the BLE Service
        pService = pServer->createService(SERVICE_UUID);

        // Create a BLE Characteristic
        pCharacteristicTX = pService->createCharacteristic(
                            CHARACTERISTIC_UUID_TX,
                            BLECharacteristic::PROPERTY_NOTIFY
                          );

        pCharacteristicTX->addDescriptor(new BLE2902());

        pCharacteristicRX = pService->createCharacteristic(
                                              CHARACTERISTIC_UUID_RX,
                                              BLECharacteristic::PROPERTY_WRITE
                                            );

        pCharacteristicRX->setCallbacks(this);

        // Start the service
        pService->start();

        // Start advertising
        pServer->getAdvertising()->addServiceUUID(pService->getUUID());
        pServer->getAdvertising()->start();
    }

    bool connect() {
        mBuffRX.clear();
        return mConn = true;
    }

    void disconnect() {
        mConn = false;
    }

    // Il telefono e' andato via a sessione aperta. Lo segna il callback, che
    // gira nel task del Bluetooth; lo raccoglie Plynx.run(), nel task di loop().
    void markDropped() {
        mDropped = true;
    }

    bool takeDropped() {
        if (!mDropped) {
            return false;
        }
        mDropped = false;
        return true;
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

    void onConnect(BLEServer* pServer);
    void onDisconnect(BLEServer* pServer);

    void onWrite(BLECharacteristic *pCharacteristic) {
      // Dal core ESP32 3.x getValue() ritorna Arduino String, non std::string:
      // senza questa guardia lo sketch NON COMPILA sul core installato oggi di
      // default (String non converte a std::string e non ha .data()).
#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
      String rxValue = pCharacteristic->getValue();
      uint8_t* data = (uint8_t*)rxValue.c_str();
#else
      std::string rxValue = pCharacteristic->getValue();
      uint8_t* data = (uint8_t*)rxValue.data();
#endif

      if (rxValue.length() > 0) {
        size_t len = rxValue.length();

        PLYNX_DBG_DUMP(">> ", data, len);
        mBuffRX.put(data, len);
      }
    }

private:
    bool mConn;
    volatile bool mDropped;
    const char* mName;

    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristicTX;
    BLECharacteristic *pCharacteristicRX;

    PlynxFifo<uint8_t, PLYNX_MAX_READBYTES*2> mBuffRX;
};

/**
 * @brief The Plynx object of an ESP32 that talks to the Plynx app over
 * Bluetooth Low Energy, created by `<PlynxSimpleEsp32_BLE.h>` as the global `Plynx`.
 *
 * The phone connects to the board and carries the data to the server, so the
 * board needs no WiFi. begin() returns at once, and the board waits for the
 * phone.
 *
 * @code
 * #include <PlynxSimpleEsp32_BLE.h>
 *
 * void setup()
 * {
 *   Plynx.setDeviceName("Greenhouse");   // before begin()
 *   Plynx.begin("YourAuthToken");
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 * }
 * @endcode
 *
 * @see begin, setDeviceName
 */
class PlynxEsp32_BLE
    : public PlynxProtocol<PlynxTransportEsp32_BLE>
{
    typedef PlynxProtocol<PlynxTransportEsp32_BLE> Base;
public:
    PlynxEsp32_BLE(PlynxTransportEsp32_BLE& transp)
        : Base(transp)

    {}

    /**
     * @brief Starts Bluetooth Low Energy with the token of the board and waits for the
     * phone, without blocking.
     *
     * @param auth  The auth token of the board, from the Plynx app.
     *
     * Call it in setup(), once, after setDeviceName().
     *
     * @code
     * #include <PlynxSimpleEsp32_BLE.h>
     *
     * void setup()
     * {
     *   Plynx.begin("YourAuthToken");
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @see setDeviceName
     */
    void begin(const char* auth)
    {
        Base::begin(auth);
        state = DISCONNECTED;
        conn.begin();
    }

    /**
     * @brief Sets the name the phone shows when it looks for the board. The
     * default is "Plynx".
     *
     * @param name  The name, such as "Greenhouse".
     *
     * Call it in setup(), before begin(). The name is used when Bluetooth Low Energy starts.
     *
     * @code
     * #include <PlynxSimpleEsp32_BLE.h>
     *
     * void setup()
     * {
     *   Plynx.setDeviceName("Greenhouse");
     *   Plynx.begin("YourAuthToken");
     * }
     * @endcode
     *
     * @warning Called after begin(), it changes nothing.
     */
    void setDeviceName(const char* name) {
        conn.setDeviceName(name);
    }

    // PLYNX_DISCONNECTED() quando il telefono se ne va. Prima non scattava mai:
    // Plynx.disconnect() mette lo stato a DISCONNECTED senza chiamarlo, e da li'
    // run() esce subito. Si chiama qui e non nel callback, perche' il codice
    // dell'utente (fermare un motore) deve girare dove gira loop().
    bool run(bool avail = false) {
        if (conn.takeDropped()) {
            PlynxOnDisconnected();
        }
        return Base::run(avail);
    }

};


static PlynxTransportEsp32_BLE _plynxTransportBLE;
PlynxEsp32_BLE Plynx(_plynxTransportBLE);

inline
void PlynxTransportEsp32_BLE::onConnect(BLEServer* pServer) {
  PLYNX_LOG1(PLYNX_F("BLE connect"));
  connect();
  Plynx.startSession();
};

inline
void PlynxTransportEsp32_BLE::onDisconnect(BLEServer* pServer) {
  PLYNX_LOG1(PLYNX_F("BLE disconnect"));
  if (Plynx.connected()) {
    markDropped();
  }
  Plynx.disconnect();
  disconnect();
  // Ricomincia ad annunciarsi, altrimenti la scheda e collegabile UNA SOLA
  // volta per accensione: il core non riparte da solo (m_advertiseOnDisconnect
  // e false) e dopo un blocco schermo o un allontanamento la scheda spariva
  // dalla scansione finche non la si resettava a mano.
  if (pServer) {
    pServer->startAdvertising();
  }
}


#include <PlynxWidgets.h>

#endif
