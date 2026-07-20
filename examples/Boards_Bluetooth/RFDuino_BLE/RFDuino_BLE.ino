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

  This example shows how to use RFduino BLE
  to connect your project to Plynx.

  Note: This requires RFduino support package:
    https://github.com/RFduino/RFduino

  And BLEPeripheral library
    from http://librarymanager/all#BLEPeripheral
    or https://github.com/sandeepmistry/arduino-BLEPeripheral

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleBLEPeripheral.h>
#include <BLEPeripheral.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Create ble serial instance, parameters are ignored for MicroBit
BLESerial SerialBLE(0, 0, 0);

void setup() {
  Serial.begin(9600);

  SerialBLE.setLocalName("Plynx");
  SerialBLE.setDeviceName("Plynx");
  SerialBLE.setAppearance(0x0080);
  SerialBLE.begin();

  Serial.println("Waiting for connections...");

  Plynx.begin(SerialBLE, auth);
}

void loop() {
  SerialBLE.poll();

  if (SerialBLE) {    // If BLE is connected...
    Plynx.run();
  }
}

