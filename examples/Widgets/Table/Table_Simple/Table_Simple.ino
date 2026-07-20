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

  You can use Table widget for logging events

  App project setup:
    Default Table widget on V1
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
int rowIndex = 0;

void sendEvent() {
  // adding 1 row to table every second
  Plynx.virtualWrite(V1, "add", rowIndex, "My Event", millis() / 1000);

  //highlighting latest added row in table
  Plynx.virtualWrite(V1, "pick", rowIndex);

  rowIndex++;
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);

  //clean table at start
  Plynx.virtualWrite(V1, "clr");

  //run sendEvent method every second
  timer.setInterval(1000L, sendEvent);
}

void loop()
{
  Plynx.run();
  timer.run();
}

