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

  This example shows how to use Arduino + RedBearLab BLE Mini
  to connect your project to Plynx.

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleSerialBLE.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

#define SerialBLE  Serial1      // Set Serial object

void setup()
{
  // This is for debug prints
  Serial.begin(9600);

  Serial.println("Waiting for connections...");

  SerialBLE.begin(57600);       // BLE Mini uses baud 57600
  Plynx.begin(SerialBLE, auth);
}

void loop()
{
  Plynx.run();
}

