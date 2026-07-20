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
    Follow us:                  http://www.fb.com/plynxapp
                                http://twitter.com/plynx_app

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows you how you can use server as storage for
  your data like EEPROM

  Project setup in the Plynx app (not necessary):
    Value display on V1 in PUSH mode.
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

PlynxTimer timer;
int uptimeCounter;
String someStaticData = "SomeStaticData";

// This function will run every time Plynx connection is established
PLYNX_CONNECTED() {
  //get data stored in virtual pin V0 from server
  Plynx.syncVirtual(V0);
}

// restoring counter from server
PLYNX_WRITE(V0)
{
  //restoring int value
  uptimeCounter = param[0].asInt();
  //restoring string value
  someStaticData = param[1].asString();
}

void increment() {
  uptimeCounter++;

  //storing int and string in V0 pin on server
  Plynx.virtualWrite(V0, uptimeCounter, someStaticData);

  //updating value display with uptimeCounter
  Plynx.virtualWrite(V1, uptimeCounter);
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);

  timer.setInterval(1000L, increment);
}

void loop()
{
  Plynx.run();
  timer.run();
}

