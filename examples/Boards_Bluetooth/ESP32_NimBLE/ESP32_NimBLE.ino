/*************************************************************
  Download latest Plynx library here:
    https://github.com/NickP005/plynx-library/releases/latest

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
  This example shows how to use ESP32 NimBLE
  to connect your project to Plynx.

  NimBLE is the stack to prefer on ESP32. Measured on this very sketch with
  arduino-cli 1.5.1 and core esp32:esp32 3.3.10, with nothing in the library
  path but Plynx and NimBLE-Arduino: the same project built against the older
  Bluedroid shell takes 1.111.735 bytes of flash (84%); against NimBLE it
  takes 620.256 (47%), and 716.316 (54%) with the experimental over-the-air
  update receiver compiled in. The receiver costs 94 KB and you still end up
  30 points of flash better off.

  That "nothing else in the library path" is not pedantry: an unrelated
  Ethernet library merely SITTING in the search path pulls in 2.000 bytes
  here, and the first three times these numbers were measured it was there.

  Warning: Bluetooth support is in beta!
 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleEsp32_NimBLE.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

void setup()
{
  // Debug console
  Serial.begin(9600);

  Serial.println("Waiting for connections...");

  Plynx.setDeviceName("Plynx");

  Plynx.begin(auth);
}

void loop()
{
  Plynx.run();
}

