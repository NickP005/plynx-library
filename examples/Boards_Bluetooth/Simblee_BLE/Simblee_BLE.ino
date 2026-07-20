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

  This example shows how to use Simblee BLE
  to connect your project to Plynx.

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleSimbleeBLE.h>
#include <SimbleeBLE.h>

char auth[] = "YourAuthToken";

void setup()
{
  Serial.begin(9600);

  SimbleeBLE.deviceName = "Simblee";
  SimbleeBLE.advertisementInterval = MILLISECONDS(300);
  SimbleeBLE.txPowerLevel = -20;  // (-20dbM to +4 dBm)

  // start the BLE stack
  SimbleeBLE.begin();

  Serial.println("Waiting for connections...");

  Plynx.begin(auth);
}


void loop()
{
  Plynx.run();
}

