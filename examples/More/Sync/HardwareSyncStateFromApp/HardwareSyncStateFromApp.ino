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
    Follow us:                  http://www.fb.com/plynxapp
                                http://twitter.com/plynx_app

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  You can synchronize the state of widgets with hardware states,
  even if hardware resets or looses connection temporarily

  Project setup in the Plynx app:
    Slider widget (0...1024) on V0
    Value display (0...1024) on V2
    Button widget on digital pin (connected to an LED)
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <SPI.h>
#include <Ethernet.h>
#include <PlynxSimpleEthernet.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// This function will run every time Plynx connection is established
PLYNX_CONNECTED() {
  // Request Plynx server to re-send latest values for all pins
  Plynx.syncAll();

  // You can also update individual virtual pins like this:
  //Plynx.syncVirtual(V0, V2);

  // Let's write your hardware uptime to Virtual Pin 2
  int value = millis() / 1000;
  Plynx.virtualWrite(V2, value);
}

PLYNX_WRITE(V0)
{
  // Use of syncAll() will cause this function to be called
  // Parameter holds last slider value
  int sliderValue0 = param.asInt();
}

PLYNX_WRITE(V2)
{
  // You'll get uptime value here as result of syncAll()
  int uptime = param.asInt();
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);
}

void loop()
{
  Plynx.run();
}

