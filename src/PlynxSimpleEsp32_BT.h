#ifndef PlynxSimpleEsp32_BT_h
#define PlynxSimpleEsp32_BT_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "ESP32_BT"
#endif

#define PLYNX_SEND_ATOMIC
#define PLYNX_SEND_CHUNK 40

#include "sdkconfig.h"

// Classic Bluetooth (SPP) lives only on the original ESP32. The C3, S3, C6
// and H2 have Bluetooth Low Energy and nothing else, and before this check a
// sketch for them failed far away with "'Plynx' was not declared in this
// scope", which says nothing about the real reason.
#if defined(__has_include)
#if __has_include("soc/soc_caps.h")
#include "soc/soc_caps.h"
#if !defined(SOC_BT_CLASSIC_SUPPORTED) || !SOC_BT_CLASSIC_SUPPORTED
#error "PlynxSimpleEsp32_BT.h needs Classic Bluetooth, which only the original ESP32 has. This chip (ESP32-C3, S3, C6...) speaks Bluetooth Low Energy, so include <PlynxSimpleEsp32_NimBLE.h> instead."
#endif
#endif
#endif

#if defined(CONFIG_BT_ENABLED) && defined(CONFIG_BLUEDROID_ENABLED)

#ifdef ARDUINO_ARCH_ESP32
#include "esp32-hal-log.h"
#endif

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>
#include <utility/PlynxFifo.h>

class PlynxTransportEsp32_BT
{
  public:
    PlynxTransportEsp32_BT()
      : mConn (false)
      , mName ("Plynx")
    {}

    void setDeviceName(const char* name) {
      mName = name;
    }

    // IP redirect not available
    void begin(char PLYNX_UNUSED *h, uint16_t PLYNX_UNUSED p) {}

    void begin() {
      instance = this;

      if (!btStarted() && !btStart()) {
        PLYNX_LOG1(PLYNX_F("btStart failed"));
        return;
      }

      esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
      if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED) {
        if (esp_bluedroid_init()) {
          PLYNX_LOG1(PLYNX_F("esp_bluedroid_init failed"));
          return;
        }
      }

      if (bt_state != ESP_BLUEDROID_STATUS_ENABLED) {
        if (esp_bluedroid_enable()) {
          PLYNX_LOG1(PLYNX_F("esp_bluedroid_enable failed"));
          return;
        }
      }

      if (esp_spp_register_callback(esp_spp_cb) != ESP_OK) {
        PLYNX_LOG1(PLYNX_F("esp_spp_register_callback failed"));
        return;
      }

      if (esp_spp_init(ESP_SPP_MODE_CB) != ESP_OK) {
        PLYNX_LOG1(PLYNX_F("esp_spp_init failed"));
        return;
      }

      if (esp_bredr_tx_power_set(ESP_PWR_LVL_N2, ESP_PWR_LVL_P7) != ESP_OK)
      {
        PLYNX_LOG1(PLYNX_F("esp_bredr_tx_power_set failed"));
      };

      if (esp_bt_dev_set_device_name(mName) != ESP_OK)
      {
        PLYNX_LOG1(PLYNX_F("esp_bt_dev_set_device_name failed"));
      }
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
      if (!spp_handle) {
        return 0;
      }

      esp_err_t err = esp_spp_write(spp_handle, len, (uint8_t *)buf);
      return (err == ESP_OK) ? len : 0;
    }

    size_t available() {
      size_t rxSize = mBuffRX.size();
      return rxSize;
    }


    static
    void putData(uint8_t* data, uint16_t len) {
      if (instance)
      {
        // PLYNX_DBG_DUMP(">> ", data, len);
        instance->mBuffRX.put(data, len);
      }
    }

  private:
    static PlynxTransportEsp32_BT* instance;
    static uint32_t spp_handle;

    static void onConnect();
    static void onDisconnect();

    bool mConn;
    const char* mName;

    PlynxFifo<uint8_t, PLYNX_MAX_READBYTES * 2> mBuffRX;

    static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
    {
      switch (event)
      {
        case ESP_SPP_INIT_EVT: // Once the SPP callback has been registered, ESP_SPP_INIT_EVT is triggered
#ifdef ESP_IDF_VERSION_MAJOR
          // IDF v4 split the single scan-mode enum into two arguments. Same
          // guard the core's own BluetoothSerial.cpp uses (:251 in esp32 3.3.10):
          // without it this header does not build on any current core.
          esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
#else
          esp_bt_gap_set_scan_mode(ESP_BT_SCAN_MODE_CONNECTABLE_DISCOVERABLE);
#endif
          esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
          break;

        case ESP_SPP_CLOSE_EVT:// After the SPP disconnection, ESP_SPP_CLOSE_EVT is triggered.
          spp_handle = 0;
          onDisconnect();
          break;

        case ESP_SPP_DATA_IND_EVT:// Data received
          if (param->data_ind.len > 0)
          {
            instance->putData((uint8_t*)param->data_ind.data, param->data_ind.len);
          }
          else
          {
            PLYNX_LOG1(PLYNX_F("ESP_SPP_DATA_IND_EVT ERROR"));
          }
          break;

        case ESP_SPP_CONG_EVT: // SPP connection congestion status changed
          PLYNX_LOG1(PLYNX_F("ESP_SPP_CONG_EVT"));
          break;

        case ESP_SPP_SRV_OPEN_EVT://Server connection open
          spp_handle = param->open.handle;
          onConnect();
          break;

        default:
          break;
      }
    }
};

class PlynxEsp32_BT
  : public PlynxProtocol<PlynxTransportEsp32_BT>
{
    typedef PlynxProtocol<PlynxTransportEsp32_BT> Base;
  public:
    PlynxEsp32_BT(PlynxTransportEsp32_BT& transp)
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

PlynxTransportEsp32_BT* PlynxTransportEsp32_BT::instance = NULL;
uint32_t PlynxTransportEsp32_BT::spp_handle = 0;

static PlynxTransportEsp32_BT _plynxTransport;
PlynxEsp32_BT Plynx(_plynxTransport);

void PlynxTransportEsp32_BT::onConnect() {
  PLYNX_LOG1(PLYNX_F("BT connect"));
  Plynx.startSession();
};

void PlynxTransportEsp32_BT::onDisconnect() {
  PLYNX_LOG1(PLYNX_F("BT disconnect"));
  Plynx.disconnect();
}

#include <PlynxWidgets.h>

#else
#error "PlynxSimpleEsp32_BT.h needs Bluetooth with the Bluedroid stack, and this core is built without it. ESP32 cores 3.x turn it on by default, so check the board settings or use <PlynxSimpleEsp32_NimBLE.h>."
#endif
#endif
