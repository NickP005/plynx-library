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
  This example shows how to use Arduino MKRNB 1500
  to connect your project to Plynx.
  Note: This requires MKRGSM library
    from https://www.arduino.cc/en/Reference/MKRNB
  Feel free to apply it to any other example. It's simple!
 *************************************************************/
/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <MKRNB.h>
#include <PlynxSimpleMKRNB.h>

NBClient client;
GPRS gprs;
NB nbAccess;
// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";
// Your SIM credential
// Leave empty, if missing pin
char pin[]  = "";

void setup()
{
  // Debug console
  Serial.begin(9600);
  Plynx.begin(auth, nbAccess, gprs, client , pin);
}
void loop()
{
  Plynx.run();
}
