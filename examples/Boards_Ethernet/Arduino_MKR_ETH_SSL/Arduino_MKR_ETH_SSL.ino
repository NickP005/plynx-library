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
    Social networks:            http://www.fb.com/plynxapp
                                http://twitter.com/plynx_app

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example shows how to use Arduino MKR ETH shield
  to connect your project to Plynx.

  Note: This requires the latest Ethernet library (2.0.0+)
    from http://librarymanager/all#Ethernet

  WARNING: If you have an SD card, you may need to disable it
        by setting pin 4 to HIGH. Read more here:
        https://www.arduino.cc/en/Main/ArduinoEthernetShield

  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoECCX08.h>
#include <ArduinoBearSSL.h>
#include <PlynxSimpleEthernetSSL.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

#define MKRETH_CS  5
#define SDCARD_CS  4

void setup()
{
  // Debug console
  Serial.begin(9600);

  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH); // Deselect the SD card

  Ethernet.init(MKRETH_CS);      // Init MKR ETH shield

  // Enable NTP time helper (needed for SSL authentiction)
  ArduinoBearSSL.onGetTime(ntpGetTime);

  Plynx.begin(auth);
  // You can also specify server:
  //Plynx.begin(auth, "yes.plynx.cc", 80);
  //Plynx.begin(auth, IPAddress(192,168,1,100), 8080);
  // For more options, see Boards_Ethernet/Arduino_Ethernet_Manual example
}

void loop()
{
  Plynx.run();
}

