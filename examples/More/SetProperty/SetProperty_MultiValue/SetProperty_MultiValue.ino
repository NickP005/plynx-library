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
    Menu Widget on V1 with 2 items
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

PLYNX_WRITE(V1) {
  int value = param.asInt();
  if (value == 1) {
    Serial.println("Item 1 selected");
  } else if (value == 2) {
    // If item 2 is selected, change menu items...
    PlynxParamAllocated items(128); // list length, in bytes
    items.add("New item 1");
    items.add("New item 2");
    items.add("New item 3");
    Plynx.setProperty(V1, "labels", items);

    // You can also use it like this:
    //Plynx.setProperty(V1, "labels", "item 1", "item 2", "item 3");

  } else {
    Serial.println("Unknown item selected");
  }
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

