/*************************************************************
  Download latest Plynx library here:
    https://github.com/NickP005/plynx-library-arduino/releases/latest

  Plynx lets you create beautiful drag-and-drop visual interfaces
  for your projects in 5 minutes. And it works with almost every
  hardware out there.

    Downloads, docs, tutorials: http://www.plynx.cc
    Sketch generator:           http://examples.plynx.cc
    Plynx community:            http://community.plynx.cc
    Follow us:                  http://www.fb.com/plynxapp
                                http://twitter.com/plynx_app

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  You’ll need:
   - Plynx App (download from AppStore or Google Play)
   - Arduino UNO or similar microcontroller board
   - Decide how to connect to Plynx
     (USB, Ethernet, Wi-Fi, Bluetooth, ...)

  There is a bunch of great example sketches included to show you how to get
  started. Think of them as LEGO bricks  and combine them as you wish.
  For example, take the Ethernet Shield sketch and combine it with the
  Servo example, or choose a USB sketch and add a code from SendData
  example.
 *************************************************************

 Let’s turn ON your LED with Plynx!
 In this example we'll use Arduino UNO + Ethernet Shield

 5 Steps guide:

  1. Connect LED to Pin 9
  ( http://arduino.cc/en/uploads/Tutorial/simplefade_bb.png )

 In the Plynx App:
  2. Create New Project
  3. Email yourself Auth Token. You can do it later at any time
  4. Add a Button Widget. Select Pin D9 in Widget's Settings
  5. Press Play icon. Enjoy Plynxing!

  You can find a QR code for easy setup of this project here:
  https://github.com/NickP005/plynx-library-arduino/tree/master/examples/GettingStarted/PlynxBlink

 *************************************************************/

#define PLYNX_PRINT Serial // Enables Serial Monitor

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


// Following includes are for Arduino Ethernet Shield (W5100)
// If you're using another shield, see Boards_* examples
#include <SPI.h>
#include <Ethernet.h>
#include <PlynxSimpleEthernet.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";


void setup()
{
  // See the connection status in Serial Monitor
  Serial.begin(9600);

  // Here your Arduino connects to the Plynx Cloud.
  Plynx.begin(auth);
}

void loop()
{
  // All the Plynx Magic happens here...
  Plynx.run();

  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Plynx. Remember
  // to avoid delay() function!
}

