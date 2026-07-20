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

  This example shows how to use Adafruit Feather 32u4 BLE
  to connect your project to Plynx.

  Note: This requires BluefruitLE nRF51 library
    from http://librarymanager/all#Adafruit_BluefruitLE_nRF51
    or https://github.com/adafruit/Adafruit_BluefruitLE_nRF51

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleSerialBLE.h>
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>
#include <SPI.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// SHARED SPI SETTINGS (see adafruit webpages for details)
#define BLUEFRUIT_SPI_CS               8
#define BLUEFRUIT_SPI_IRQ              7
#define BLUEFRUIT_SPI_RST              4    // Optional but recommended, set to -1 if unused
#define BLUEFRUIT_VERBOSE_MODE         true

// Create ble instance, see pinouts above
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

void setup() {
  Serial.begin(9600);

  ble.begin(BLUEFRUIT_VERBOSE_MODE);
  ble.factoryReset(); // Optional
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println("Waiting for connections...");

  Plynx.begin(ble, auth);
}

void loop() {
  Plynx.run();
}

