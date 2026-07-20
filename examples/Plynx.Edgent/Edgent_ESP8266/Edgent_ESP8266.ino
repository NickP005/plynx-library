
// Fill-in information from your Plynx Template here
//#define PLYNX_TEMPLATE_ID           "TMPLxxxxxx"
//#define PLYNX_DEVICE_NAME           "Device"

#define PLYNX_FIRMWARE_VERSION        "0.1.0"

#define PLYNX_PRINT Serial
//#define PLYNX_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_SPARKFUN_PLYNX_BOARD
//#define USE_NODE_MCU_BOARD
//#define USE_WITTY_CLOUD_BOARD
//#define USE_WEMOS_D1_MINI

#include "PlynxEdgent.h"

void setup()
{
  Serial.begin(115200);
  delay(100);

  PlynxEdgent.begin();
}

void loop() {
  PlynxEdgent.run();
}

