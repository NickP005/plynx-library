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

  This is a simple stroboscope.
  You can turn it on and of using a button,
  and control frequency with a slider.

  App project setup:
    Button widget (Switch) on V1
    Slider widget (100...1000) on V2
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

#define LED_PIN 9

PlynxTimer timer;
int t1;

// Toggle LED
void ledPlynx()
{
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

// Enable/disable blinking using virtual pin 1
PLYNX_WRITE(V1)
{
  if (param.asInt()) {
    timer.enable(t1);
  } else {
    timer.disable(t1);
    digitalWrite(LED_PIN, LOW);
  }
}

// Change blink interval using virtual pin 2
PLYNX_WRITE(V2)
{
  long interval = param.asLong();
  boolean wasEnabled = timer.isEnabled(t1);
  timer.deleteTimer(t1);
  t1 = timer.setInterval(interval, ledPlynx);
  if (!wasEnabled) {
    timer.disable(t1);
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);

  // Configure LED and timer
  pinMode(LED_PIN, OUTPUT);
  t1 = timer.setInterval(500L, ledPlynx);
  timer.disable(t1);
}

void loop()
{
  Plynx.run();
  timer.run();
}

