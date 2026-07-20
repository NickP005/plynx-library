/*************************************************************
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
  This sketch shows how to access EthernetClient directly in Plynx

  1. This gives you full control of the connection process.
  2. Shows a sensible way of integrating other connectivity hardware,
     that was not supported by Plynx out-of-the-box.

  NOTE: Pins 10, 11, 12 and 13 are reserved for Ethernet module.
        DON'T use them in your sketch directly!

  WARNING: If you have an SD card, you may need to disable it
        by setting pin 4 to HIGH. Read more here:
        https://www.arduino.cc/en/Main/ArduinoEthernetShield

 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <SPI.h>
#include <Ethernet.h>
#include <PlynxSimpleStream.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// You can specify your board mac adress
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Ethernet shield and SDcard pins
#define W5100_CS  10
#define SDCARD_CS 4

EthernetClient ethernetClient;

// This function tries to connect to the cloud using TCP
bool connectPlynx()
{
  ethernetClient.stop();
  return ethernetClient.connect(PLYNX_DEFAULT_DOMAIN, PLYNX_DEFAULT_PORT);
}


void setup()
{
  // Debug console
  Serial.begin(9600);

  // Deselect the SD card
  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH);

  // Initialize Ethernet shield
  Ethernet.begin(mac);

  delay(1000); // Give the Ethernet shield a second to initialize

  connectPlynx();

  Plynx.begin(ethernetClient, auth);
}

void loop()
{
  // Reconnect to Plynx Cloud
  if (!ethernetClient.connected()) {
    connectPlynx();
    return;
  }

  Plynx.run();
}

