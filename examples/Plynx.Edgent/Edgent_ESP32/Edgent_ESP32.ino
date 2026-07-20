
// Fill-in information from your Plynx Template here
//#define PLYNX_TEMPLATE_ID           "TMPLxxxxxx"
//#define PLYNX_DEVICE_NAME           "Device"

#define PLYNX_FIRMWARE_VERSION        "0.1.0"

#define PLYNX_PRINT Serial
//#define PLYNX_DEBUG

#define APP_DEBUG

// Uncomment your board, or configure a custom board in Settings.h
//#define USE_WROVER_BOARD
//#define USE_TTGO_T7

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

