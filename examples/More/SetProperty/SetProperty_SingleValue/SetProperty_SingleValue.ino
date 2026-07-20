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

  You can set predefined properties of any widget. Like color, label

  Project setup in the Plynx app:
    Gauge widget (0...100) on V0 in PUSH mode
    Slider widget (0...100) on V1
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

#define PLYNX_GREEN     "#23C48E"
#define PLYNX_BLUE      "#04C0F8"
#define PLYNX_YELLOW    "#ED9D00"
#define PLYNX_RED       "#D3435C"
#define PLYNX_DARK_BLUE "#5F7CD8"

String gaugeColor;

PLYNX_WRITE(V1) {
  int gaugeValue = param.asInt();

  String newColor;
  if (gaugeValue > 80) {
    newColor = PLYNX_RED;
  } else if (gaugeValue > 50) {
    newColor = PLYNX_YELLOW;
  } else {
    newColor = PLYNX_GREEN;
  }

  // Send only if changed
  if (newColor != gaugeColor) {
    gaugeColor = newColor;
    Plynx.setProperty(V0, "color", gaugeColor);
  }

  Plynx.virtualWrite(V0, gaugeValue);
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

