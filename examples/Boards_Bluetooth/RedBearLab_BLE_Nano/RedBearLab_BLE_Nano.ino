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

  This example shows how to use RedBearLab BLE Nano
  to connect your project to Plynx.

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleRedBearLab_BLE_Nano.h>
#include <BLE_API.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

uint8_t device_name[] = "Plynx";

void setup() {
  Serial.begin(9600);
  delay(100);

  ble.init();

  ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
  ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                         device_name, sizeof(device_name) - 1);

  ble.gap().setDeviceName(device_name);
  ble.gap().setTxPower(4);

  Serial.println("Waiting for connections...");

  // Add Plynx service...
  Plynx.begin(auth);

  ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  ble.gap().setAdvertisingInterval(Gap::MSEC_TO_GAP_DURATION_UNITS(1000));
  ble.gap().setAdvertisingTimeout(0);
  ble.startAdvertising();
}

void loop() {
  Plynx.run();
}

