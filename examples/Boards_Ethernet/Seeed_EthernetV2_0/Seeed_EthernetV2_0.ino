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
    Social networks:            http://www.fb.com/plynxapp
                                http://twitter.com/plynx_app

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example shows how to use Seeed Ethernet Shield V2.0 (W5200)
  to connect your project to Plynx.

  More about the shield (and library download):
    http://www.seeedstudio.com/wiki/Ethernet_Shield_V2.0
  (It looks like this library for W5200 works only on Arduino IDE 1.0.*)

  Note: This Hardware is not tested.

  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <SPI.h>
#include <EthernetV2_0.h>
#include <PlynxSimpleEthernetV2_0.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

#define W5200_CS  10
#define SDCARD_CS 4

void setup()
{
  // Debug console
  Serial.begin(9600);

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH); // Deselect the SD card

  Plynx.begin(auth);
  // You can also specify server:
  //Plynx.begin(auth, "yes.plynx.cc", 80);
  //Plynx.begin(auth, IPAddress(192,168,1,100), 8080);
}

void loop()
{
  Plynx.run();
}

