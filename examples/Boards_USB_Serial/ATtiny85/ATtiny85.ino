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
  =>
  =>          USB HOWTO: http://tiny.cc/PlynxUSB
  =>

  Note: This requires ATtiny support package:
    https://github.com/damellis/attiny

  Be sure to check ordinary Serial example first!!!

                   +-\/-+
  A0|6 (D 5) PB5  1|    |8  Vcc
  A3|9 (D 3) PB3  2|    |7  PB2 (D 2) A1|7
  A2|8 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
             GND  4|    |5  PB0 (D 0) pwm0
                   +----+

 *************************************************************/

#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(1, 2); // RX, TX
SoftwareSerial SwSerial(3, 4);

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT DebugSerial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <PlynxSimpleStream.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

//#include <SoftwareServo.h>
//SoftwareServo servo1;

void setup()
{
  // Debug console
  DebugSerial.begin(9600);

  // Plynx will work through SoftwareSerial
  // Do not read or write this serial manually in your sketch
  SwSerial.begin(9600);
  Plynx.begin(SwSerial, AUTH_TOKEN);

  // Sometimes you need to calibrate your ATtiny timer
  //OSCCAL = 175;
}

void loop()
{
  Plynx.run();
}

