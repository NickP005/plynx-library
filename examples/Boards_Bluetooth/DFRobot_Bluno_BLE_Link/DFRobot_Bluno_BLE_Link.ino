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

  This example shows how to use DFRobot Bluno or BLE Link module
  to connect your project to Plynx.

  Please be sure to update your DFRobot firmware to at least V1.97:
    https://github.com/DFRobot/BLE_firmware_V1.9

  For Bluno, read instructions here:
    https://www.dfrobot.com/wiki/index.php/Bluno_SKU:DFR0267

  For BLE-Link, read instructions here:
    https://www.dfrobot.com/wiki/index.php/BLE-Link_(SKU:TEL0073)

  NOTE: BLE support is in beta!

 *************************************************************/

#define PLYNX_USE_DIRECT_CONNECT

// You could use a spare Hardware Serial on boards that have it (like Mega)
#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(2, 3); // RX, TX

#define PLYNX_PRINT DebugSerial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleSerialBLE.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

void setup()
{
  // Debug console
  DebugSerial.begin(9600);

  DebugSerial.println("Waiting for connections...");

  // Plynx will work through Serial
  // Do not read or write this serial manually in your sketch
  Serial.begin(115200);
  Plynx.begin(Serial, auth);
}

void loop()
{
  Plynx.run();
}

