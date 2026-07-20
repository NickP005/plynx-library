
extern "C" {
  void app_loop();
  void restartMCU();
}

#include "Settings.h"
#include <PlynxSimpleWioTerminal_SSL.h>

#ifndef PLYNX_NEW_LIBRARY
#error "Old version of Plynx library is in use. Please replace it with the new one."
#endif

#if !defined(PLYNX_TEMPLATE_ID) || !defined(PLYNX_DEVICE_NAME)
#error "Please specify your PLYNX_TEMPLATE_ID and PLYNX_DEVICE_NAME"
#endif

#include "PlynxState.h"
#include "ConfigStore.h"
#include "ResetButton.h"
#include "ConfigMode.h"
#include "Indicator.h"
//#include "OTA.h"
#include "Console.h"

inline
void PlynxState::set(State m) {
  if (state != m && m < MODE_MAX_VALUE) {
    DEBUG_PRINT(String(StateStr[state]) + " => " + StateStr[m]);
    state = m;

    // You can put your state handling here,
    // i.e. implement custom indication
  }
}

void printDeviceBanner()
{
  Plynx.printBanner();
  DEBUG_PRINT("--------------------------");
  DEBUG_PRINT(String("Product:  ") + PLYNX_DEVICE_NAME);
  DEBUG_PRINT(String("Firmware: ") + PLYNX_FIRMWARE_VERSION " (build " __DATE__ " " __TIME__ ")");
  if (configStore.getFlag(CONFIG_FLAG_VALID)) {
    DEBUG_PRINT(String("Token:    ...") + (configStore.cloudToken+28));
  }
  DEBUG_PRINT(String("Device:   ") + PLYNX_INFO_DEVICE);
  DEBUG_PRINT(String("MAC:      ") + WiFi.macAddress());
  DEBUG_PRINT(String("WiFi FW:  ") + rpc_system_version());
  DEBUG_PRINT("--------------------------");
}

void runPlynxWithChecks() {
  Plynx.run();
  if (PlynxState::get() == MODE_RUNNING) {
    if (!Plynx.connected()) {
      if (WiFi.status() == WL_CONNECTED) {
        PlynxState::set(MODE_CONNECTING_CLOUD);
      } else {
        PlynxState::set(MODE_CONNECTING_NET);
      }
    }
  }
}

class Edgent {

public:
  void begin()
  {
    //indicator_init();
    button_init();
    config_init();
    console_init();

    printDeviceBanner();

    if (configStore.getFlag(CONFIG_FLAG_VALID)) {
      PlynxState::set(MODE_CONNECTING_NET);
    } else if (config_load_blnkopt()) {
      DEBUG_PRINT("Firmware is preprovisioned");
      PlynxState::set(MODE_CONNECTING_NET);
    } else {
      PlynxState::set(MODE_WAIT_CONFIG);
    }
  }

  void run() {
    app_loop();
    switch (PlynxState::get()) {
    case MODE_WAIT_CONFIG:       
    case MODE_CONFIGURING:       enterConfigMode();    break;
    case MODE_CONNECTING_NET:    enterConnectNet();    break;
    case MODE_CONNECTING_CLOUD:  enterConnectCloud();  break;
    case MODE_RUNNING:           runPlynxWithChecks(); break;
    case MODE_OTA_UPGRADE:       enterError();         break; // TODO: enterOTA
    case MODE_SWITCH_TO_STA:     enterSwitchToSTA();   break;
    case MODE_RESET_CONFIG:      enterResetConfig();   break;
    default:                     enterError();         break;
    }
  }

};

Edgent PlynxEdgent;
PlynxTimer edgentTimer;

void app_loop() {
    edgentTimer.run();
    edgentConsole.run();
}
