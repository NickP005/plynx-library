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

  Plynx using a LED widget on your phone!

  App project setup:
    LED widget on V3
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

// Select your pin with physical button
const int btnPin = 1;

WidgetLED led3(V3);

PlynxTimer timer;

// V3 LED Widget represents the physical button state
boolean btnState = false;
void buttonLedWidget()
{
  // Read button
  boolean isPressed = (digitalRead(btnPin) == LOW);

  // If state has changed...
  if (isPressed != btnState) {
    if (isPressed) {
      led3.on();
    } else {
      led3.off();
    }
    btnState = isPressed;
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);

  // Setup physical button pin (active low)
  pinMode(btnPin, INPUT_PULLUP);

  timer.setInterval(500L, buttonLedWidget);
}

void loop()
{
  Plynx.run();
  timer.run();
}

