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

  Output any data on LCD widget!

  App project setup:
    LCD widget, SIMPLE mode, in widget settings :
    - Select pin V0 for zero pin
    - Select pin V1 for first pin
    - Leave "Reading Frequency" on "1 sec" interval
    - Type into first edit field "/pin0/ seconds"
    - Type into second edit field "/pin1/ millis"
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

PLYNX_READ(V0) {
  Plynx.virtualWrite(V0, millis() / 1000);
}

PLYNX_READ(V1) {
  Plynx.virtualWrite(V1, millis());
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

