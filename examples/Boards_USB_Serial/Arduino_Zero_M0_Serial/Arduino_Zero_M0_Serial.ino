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
  =>
  =>          USB HOWTO: http://tiny.cc/PlynxUSB
  =>

  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT SerialUSB

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleStream.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";


void setup()
{
  // Debug console
  SerialUSB.begin(9600);

  // Plynx will work through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin(9600);
  Plynx.begin(Serial, auth);
}

void loop()
{
  Plynx.run();
}

