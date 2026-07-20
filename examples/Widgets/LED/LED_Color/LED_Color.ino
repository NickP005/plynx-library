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

  Plynx using a LED widget on your phone!

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

WidgetLED led1(V1);

PlynxTimer timer;
bool ledStatus = false;

#define PLYNX_GREEN     "#23C48E"
#define PLYNX_BLUE      "#04C0F8"
#define PLYNX_YELLOW    "#ED9D00"
#define PLYNX_RED       "#D3435C"
#define PLYNX_DARK_BLUE "#5F7CD8"

// V1 LED Widget is blinking
void blinkLedWidget()
{
  if (ledStatus) {
    led1.setColor(PLYNX_RED);
    Serial.println("LED on V1: red");
    ledStatus = false;
  } else {
    led1.setColor(PLYNX_GREEN);
    Serial.println("LED on V1: green");
    ledStatus = true;
  }
}

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth);

  // Turn LED on, so colors are visible
  led1.on();
  // Setup periodic color change
  timer.setInterval(1000L, blinkLedWidget);
}

void loop()
{
  Plynx.run();
  timer.run();
}

