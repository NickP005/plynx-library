/*************************************************************
  Plynx is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.plynx.cc
    Sketch generator:           http://examples.plynx.cc
    Plynx community:            http://community.plynx.cc
    Social networks:            http://www.fb.com/plynxapp
                                http://twitter.com/plynx_app

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows how to use Energia BLE
  to connect your project to Plynx.

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

//#define PLYNX_DEBUG
#define PLYNX_PRINT Serial
#define PLYNX_RUN_YIELD() { ble.handleEvents(); }

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <BLE.h>
#include <PlynxSimpleSerialBLE.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

void setup() {
  Serial.begin(115200);

  ble.setLogLevel(BLE_LOG_ERRORS);
  ble.begin();
  ble.serial();
  ble.setAdvertName("Plynx BLE");
  ble.startAdvert();

  Serial.println("Waiting for connections...");

  Plynx.begin(ble, auth);
}

void loop() {
  Plynx.run();
}

