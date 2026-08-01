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

  This example shows how to use LightBlue Bean / Bean+
  to connect your project to Plynx.

  Note: For this example you need to install Bean Loader:
    https://punchthrough.com/bean/guides/getting-started/intro/

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(2, 3); // RX, TX

#define PLYNX_PRINT DebugSerial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleSerialBLE.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

PlynxTimer timer;

void setup()
{
  DebugSerial.begin(9600);

  DebugSerial.println("Waiting for connections...");

  // Plynx will work through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin();
  Plynx.begin(Serial, auth);

  timer.setInterval(1000L, []() {
    Plynx.virtualWrite(V1, millis());
  });
}

void loop()
{
  Plynx.run();
  timer.run();
}

// Attach an RGB Picker widget to the Virtual pin 1
// to control the built-in RGB led!
PLYNX_WRITE(V0) {
  int r = param[0].asInt();
  int g = param[1].asInt();
  int b = param[2].asInt();
  Bean.setLed(r, g, b);
}

