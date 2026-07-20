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

  This example shows how to monitor a button state
  using polling mechanism.

  App project setup:
    LED widget on V1
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

int prevState = -1;
int currState = -1;
long lastChangeTime = 0;

void checkPin()
{
  // Invert state, since button is "Active LOW"
  int state = !digitalRead(2);

  // Debounce mechanism
  long t = millis();
  if (state != prevState) {
    lastChangeTime = t;
  }
  if (t - lastChangeTime > 50) {
    if (state != currState) {
      currState = state;
      Plynx.virtualWrite(V1, state);
    }
  }
  prevState = state;
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);

  // Make pin 2 default HIGH, and attach INT to our handler
  pinMode(2, INPUT_PULLUP);
}

void loop()
{
  Plynx.run();
  checkPin();
}

