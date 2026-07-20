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

  This example shows how to keep WiFi connection on ESP8266.

 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <ESP8266WiFi.h>
#include <PlynxSimpleEsp8266.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";


int lastConnectionAttempt = millis();
int connectionDelay = 5000; // try to reconnect every 5 seconds

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth, ssid, pass);
}

void loop()
{
  // check WiFi connection:
  if (WiFi.status() != WL_CONNECTED)
  {
    // (optional) "offline" part of code

    // check delay:
    if (millis() - lastConnectionAttempt >= connectionDelay)
    {
      lastConnectionAttempt = millis();

      // attempt to connect to Wifi network:
      if (pass && strlen(pass))
      {
        WiFi.begin((char*)ssid, (char*)pass);
      }
      else
      {
        WiFi.begin((char*)ssid);
      }
    }
  }
  else
  {
    Plynx.run();
  }
}
