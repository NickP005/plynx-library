/*************************************************************
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

  This example shows how to use AirBoard + BLE-Link
  to connect your project to Plynx.

  NOTE: BLE support is in beta!

 *************************************************************
             USB HOWTO: http://tiny.cc/PlynxUSB
 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(10, 11); // RX, TX

#define PLYNX_PRINT DebugSerial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleSerialBLE.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

PlynxTimer timer;

void setup()
{
  DebugSerial.begin(9600);

  DebugSerial.println("Waiting for connections...");

  // Plynx works through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin(115200);
  Plynx.begin(Serial, auth);

  setLED(0x04, 0x16, 0x10);

  timer.setInterval(1000L, []() {
    Plynx.virtualWrite(V1, millis());
  });
}

void loop()
{
  Plynx.run();
  timer.run();
}

#define LED_RED   9
#define LED_GREEN 5
#define LED_BLUE  6

void setLED(byte r, byte g, byte b) {
  analogWrite(LED_RED,   r);
  analogWrite(LED_GREEN, g);
  analogWrite(LED_BLUE,  b);
}

PLYNX_CONNECTED() {
  setLED(0x00, 0x00, 0x20);
}

PLYNX_WRITE(V0) {
  int r = param[0].asInt();
  int g = param[1].asInt();
  int b = param[2].asInt();
  setLED(r, g, b);
}

