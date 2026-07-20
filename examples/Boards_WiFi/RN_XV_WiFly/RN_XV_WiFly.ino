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
  This example shows how to use WiFly RN-XV
  to connect your project to Plynx.

  For this example you need WiFlyHQ library:
    https://github.com/harlequin-tech/WiFlyHQ

  Note: Ensure a stable serial connection!
        Hardware serial is preferred.
        Firmware version 4.41 or later is preferred.

  Change WiFi ssid, pass, and Plynx auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <WiFlyHQ.h>
#include <PlynxSimpleWiFly.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

#define WiFlySerial Serial1
// This can be a SoftwareSerial object:
//#include <SoftwareSerial.h>
//SoftwareSerial WiFlySerial(2, 3); // RX, TX

WiFly wifly;

void setup()
{
  // Debug console
  Serial.begin(9600);

  delay(10);

  WiFlySerial.begin(9600);  // Set your RN-XV baud rate
  delay(10);

  // Bind WiFly driver to the serial
  if (!wifly.begin(&WiFlySerial)) {
    PLYNX_FATAL("Failed to start wifly");
  }

  // You can try increasing baud rate:
  //wifly.setBaud(115200);
  //WiFlySerial.begin(115200);

  Plynx.begin(auth, wifly, ssid, pass);
}

void loop()
{
  Plynx.run();
}

