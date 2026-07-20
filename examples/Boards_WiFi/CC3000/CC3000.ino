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
  This example shows how to use Adafruit CC3000 breakout
  to connect your project to Plynx.

  For this example you need Adafruit_CC3000_Library library:
    https://github.com/adafruit/Adafruit_CC3000_Library

  Note: Firmware version 1.14 or later is preferred.

  1. Update pin definitions according to your setup.
  2. Change WiFi ssid, pass, and Plynx auth token
  3. Run :)

  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


// These are the interrupt and control pins for СС3000
#define ADAFRUIT_CC3000_IRQ   3
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10

#include <SPI.h>
#include <Adafruit_CC3000.h>
#include <PlynxSimpleCC3000.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Your WiFi credentials.
// Choose wifi_sec from WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";
int wifi_sec = WLAN_SEC_WPA2;

void setup()
{
  // Debug console
  Serial.begin(9600);

  Plynx.begin(auth, ssid, pass, wifi_sec);
}

void loop()
{
  Plynx.run();
}

