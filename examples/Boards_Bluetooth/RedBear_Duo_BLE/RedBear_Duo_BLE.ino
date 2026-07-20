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

  This example shows how to use RedBear Duo BLE
  to connect your project to Plynx.

  NOTE: BLE support is in beta!

 *************************************************************/

#if defined(ARDUINO)
  SYSTEM_MODE(MANUAL);    // If Arduino - do not connect to Particle cloud
#else
  SYSTEM_MODE(AUTOMATIC); // Otherwise, connect to Particle cloud
#endif

#define PLYNX_USE_DIRECT_CONNECT

#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleRedBear_Duo_BLE.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

void setup() {
  Serial.begin(9600);
  delay(5000);

  Serial.println("Waiting for connections...");

  Plynx.begin(auth);
}

void loop() {
  Plynx.run();
}

// Next functions are optional.
// Add ZeRGBa Widget on V0 to control onboard LED.
PLYNX_CONNECTED() {
  RGB.control(true);
}

PLYNX_WRITE(V0) {
  int r = param[0].asInt();
  int g = param[1].asInt();
  int b = param[2].asInt();
  RGB.color(r, g, b);
}

