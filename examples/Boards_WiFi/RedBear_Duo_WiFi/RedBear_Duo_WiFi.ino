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
  This example shows how to use RedBear Duo
  to connect your project to Plynx.

  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleRedBear_Duo.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Your WiFi credentials.
// Choose wifi_sec from WPA2, WPA, or WEP
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";
int wifi_sec = WPA2;

//SYSTEM_MODE(AUTOMATIC);
SYSTEM_MODE(MANUAL);

void setup()
{
  // Debug console
  Serial.begin(9600);

  delay(5000);

  WiFi.on();
  WiFi.setCredentials(ssid, pass, wifi_sec);
  WiFi.connect();

  Plynx.begin(auth);
  // Or specify server using one of those commands:
  //Plynx.begin(auth, "yes.plynx.cc", 80);
  //Plynx.begin(auth, server_ip, port);
}

void loop()
{
  Plynx.run();
}

