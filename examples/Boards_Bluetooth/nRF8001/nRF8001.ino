/*************************************************************
  Download latest Plynx library here:
    https://github.com/NickP005/plynx-library-arduino/releases/latest

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
  For this example you need BLEPeripheral library
    from http://librarymanager/all#BLEPeripheral
    or https://github.com/sandeepmistry/arduino-BLEPeripheral

  Warning: Bluetooth support is in beta!
 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleBLEPeripheral.h>
#include <BLEPeripheral.h>
#include <SPI.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// define pins (varies per shield/board)
#define BLE_REQ   10
#define BLE_RDY   2
#define BLE_RST   9

// create ble serial instance, see pinouts above
BLESerial SerialBLE(BLE_REQ, BLE_RDY, BLE_RST);

void setup()
{
  // Debug console
  Serial.begin(9600);

  SerialBLE.setLocalName("Plynx");
  SerialBLE.setDeviceName("Plynx");
  SerialBLE.setAppearance(0x0080);
  SerialBLE.begin();

  Serial.println("Waiting for connections...");

  Plynx.begin(SerialBLE, auth);
}

void loop()
{
  SerialBLE.poll();

  if (SerialBLE) {    // If BLE is connected...
    Plynx.run();
  }
}

