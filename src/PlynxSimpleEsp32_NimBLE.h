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
#define PLYNX_SEND_THROTTLE 20

/* ---- OTA over Bluetooth (EXPERIMENTAL) ----
 *
 * A board on Bluetooth has no way to FETCH anything: no route, no DNS, no
 * WiFiClient. So the built-in handler here is the push receiver, not the http
 * one: whoever is connected hands us the bytes. Both live in Plynx/PlynxOta.h
 * and share the rollback rules; only the transport differs.
 *
 * Opt out with #define PLYNX_NO_OTA, which costs nothing but also makes the
 * board impossible to update without a cable. */
#ifndef PLYNX_NO_OTA
// Must be defined BEFORE PlynxProtocol.h: together they enable the OTA
// dispatch and the pending-confirm/rollback hooks.
#define PLYNX_OTA_HAS_DEFAULT
#define PLYNX_OTA_TRANSPORT_PUSH

/* 🔴 The two rollback windows are longer here than on WiFi, and they have to
 * be. After the reboot the board must see an AUTHENTICATED session again or it
 * goes back to the old firmware, and on this transport the only thing in the
 * world that can produce that session is the phone that just sent the bytes:
 * it has to notice the disconnect, rediscover the peripheral, reconnect,
 * re-pair and log in. That is a slower road than a WiFi association with DHCP.
 *
 * The library's own comment on PLYNX_OTA_GUARD_MS says why erring long is the
 * safe direction: a window that is too long only postpones the rollback to the
 * next reset, while one that is too short throws away a firmware that worked. */
#ifndef PLYNX_OTA_CONFIRM_MS
#define PLYNX_OTA_CONFIRM_MS 180000UL
#endif
#ifndef PLYNX_OTA_GUARD_MS
#define PLYNX_OTA_GUARD_MS   300000UL
#endif
#endif // !PLYNX_NO_OTA

// blnkinf "ota-max"/"flash"/"sk-size": how big an image this board can really
// take. Must come after the decision above and before PlynxApiArduino.h, which
// assembles the profile. Until now no Bluetooth shell included this, so a BLE
// board declared no size at all and the server's "unknown never blocks" rule
// let an oversized image through to a board that would discard it.
#include <Plynx/PlynxFlashInfo.h>

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>

/* NimBLE-Arduino is a third party library, not part of the ESP32 core.
 * Without this guard the compiler says only "NimBLEDevice.h: No such file or
 * directory", which tells a first-time user nothing about what to install.
 * The depends= line in library.properties makes the Arduino IDE offer it;
 * this message covers arduino-cli and PlatformIO, where nothing offers it. */
#if defined(__has_include)
#  if !__has_include(<NimBLEDevice.h>)
#    error "Plynx: NimBLE-Arduino is not installed. Arduino IDE: Tools > Manage Libraries, search for NimBLE-Arduino. Command line: arduino-cli lib install NimBLE-Arduino"
#  endif
#endif

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

        // Ask for the biggest ATT MTU the peer will grant. The default is 23,
        // which leaves 20 usable bytes per packet: on a 600 KB firmware image
        // that is 30.000 round trips. The peer decides the real value (iOS
        // settles around 185), and nothing here depends on a particular one --
        // the protocol keeps framing by length, exactly as before.
        NimBLEDevice::setMTU(517);

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

        // WRITE_NR is offered ALONGSIDE WRITE, never instead of it: the app
        // that exists today writes with a response and must keep working.
        // ⚠️ Write-without-response is the fast lane and the dangerous one at
        // the same time: the peer stops waiting for us, so a full FIFO drops
        // bytes with no error anywhere. That is survivable only because the
        // OTA receiver checks the offset of every chunk (Plynx/PlynxOta.h).
        pCharacteristicRX = pService->createCharacteristic(
                                              CHARACTERISTIC_UUID_RX,
                                              NIMBLE_PROPERTY::WRITE |
                                              NIMBLE_PROPERTY::WRITE_NR
                                            );

        pCharacteristicRX->setCallbacks(this);

        // NimBLE-Arduino 2.x ha rinominato meta' dell'API (security, scan
        // response, firme dei callback): senza queste guardie l'header non
        // compila con la 2.x, che e' quella che installa oggi il Library
        // Manager. NIMBLE_CPP_VERSION_MAJOR esiste solo dalla 2.0 in poi.
#if defined(NIMBLE_CPP_VERSION_MAJOR) && (NIMBLE_CPP_VERSION_MAJOR >= 2)
        NimBLEDevice::setSecurityAuth(true, false, false); // bonding, come ESP_LE_AUTH_BOND
#else
        auto pSecurity = new NimBLESecurity();
        pSecurity->setAuthenticationMode(ESP_LE_AUTH_BOND);
#endif

        // Start the service
        pService->start();

        // Start advertising
        pServer->getAdvertising()->addServiceUUID(pService->getUUID());
        pServer->getAdvertising()->setAppearance(0x00);
#if defined(NIMBLE_CPP_VERSION_MAJOR) && (NIMBLE_CPP_VERSION_MAJOR >= 2)
        pServer->getAdvertising()->enableScanResponse(true);
#else
        pServer->getAdvertising()->setScanResponse(true);
#endif
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

#if defined(NIMBLE_CPP_VERSION_MAJOR) && (NIMBLE_CPP_VERSION_MAJOR >= 2)
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
      handleRx(pCharacteristic);
    }
#else
    void onConnect(NimBLEServer* pServer);
    void onDisconnect(NimBLEServer* pServer);

    void onWrite(BLECharacteristic *pCharacteristic) {
      handleRx(pCharacteristic);
    }
#endif

    void handleRx(NimBLECharacteristic* pCharacteristic) {
      // NimBLEAttValue porta puntatore e LUNGHEZZA: niente conversioni via
      // stringa sul payload binario (il protocollo ha byte 0x00 nell'header,
      // qualsiasi giro da C-string tronca il frame al primo NUL).
      NimBLEAttValue rxValue = pCharacteristic->getValue();
      const uint8_t* data = rxValue.data();
      size_t len = rxValue.length();

      if (len > 0) {
        PLYNX_DBG_DUMP(">> ", data, len);
        // 🔴 Il valore di ritorno NON si butta. `put` su un buffer pieno
        // accetta quello che entra e torna quanto ne ha preso: da qui in giù
        // nessuno può più accorgersi della differenza, perché il parser vede
        // un flusso di byte e un frame tagliato a metà somiglia a un frame
        // corto. Questa riga è l'ultimo punto del programma in cui la perdita
        // è ancora un fatto.
        const int accepted = mBuffRX.put(data, (int)len);
        if (accepted >= 0 && (size_t)accepted < len) {
          PlynxOtaRxLost(len - (size_t)accepted);
        }
      }
    }

private:
    bool mConn;
    const char* mName;

    NimBLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristicTX;
    BLECharacteristic *pCharacteristicRX;
    // 🪤 `PlynxFifo::put` is NOT blocking: when it is full it returns short and
    // every caller here ignores the return value, so bytes vanish with no
    // error. While the board erases a flash sector it does not run loop() and
    // nothing drains this, so during an OTA the queue is the thing most likely
    // to overflow -- exactly when the damage is a corrupt image.
    //
    // 🔴 The size is NOT "a few frames": it is whatever the sender is allowed to
    // have in flight, plus framing and a frame of slack (PLYNX_OTA_RX_BUFFER).
    // Counting in frames is how this ended up at four of them, 1 KB, while the
    // ack window was telling the sender to keep going for 4 KB without hearing
    // back: a queue promised four times the room it had. And the loss that
    // causes is the one thing the offset check cannot catch -- it counts bytes,
    // so a frame cut in half by a full queue goes straight past it.
#if defined(PLYNX_OTA_TRANSPORT_PUSH)
    PlynxFifo<uint8_t, PLYNX_OTA_RX_BUFFER> mBuffRX;
#else
    PlynxFifo<uint8_t, PLYNX_MAX_READBYTES*2> mBuffRX;
#endif
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

#if defined(NIMBLE_CPP_VERSION_MAJOR) && (NIMBLE_CPP_VERSION_MAJOR >= 2)

inline
void PlynxTransportEsp32_NimBLE::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
  PLYNX_LOG1(PLYNX_F("BLE connect"));
  connect();
  Plynx.startSession();
}

inline
void PlynxTransportEsp32_NimBLE::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
  PLYNX_LOG1(PLYNX_F("BLE disconnect"));
  Plynx.disconnect();
  disconnect();
  // Senza questo la scheda e collegabile una sola volta per accensione
  // (vedi PlynxSimpleEsp32_BLE.h).
  if (pServer) {
    pServer->startAdvertising();
  }
}

#else

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
  // Senza questo la scheda e collegabile una sola volta per accensione
  // (vedi PlynxSimpleEsp32_BLE.h).
  if (pServer) {
    pServer->startAdvertising();
  }
}

#endif

#ifdef PLYNX_OTA_HAS_DEFAULT
// Needs the global Plynx object above (sendInternal/connected).
#include <Plynx/PlynxOta.h>
#endif

#include <PlynxWidgets.h>

#endif
