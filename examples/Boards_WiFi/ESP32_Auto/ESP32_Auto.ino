/*************************************************************
  Download latest Plynx library here:
    https://github.com/NickP005/plynx-library/releases/latest

  Plynx is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.

    Downloads, docs, tutorials: http://www.plynx.cc

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  One flash fits all: no WiFi credentials, no token in the code.

  On first boot the board opens a setup hotspot ("Plynx-XXXX")
  that the Plynx app finds and configures for you: pick the
  board in the app, enter your WiFi, done. The configuration
  is saved on the board; every next boot connects directly.

  To start over (new WiFi, new project):
  call Plynx.resetProvisioning() from your code, or reflash.
 *************************************************************/

#define PLYNX_PRINT Serial

#include <PlynxSimpleEsp32.h>

void setup()
{
  Serial.begin(115200);
  Plynx.begin();
}

void loop()
{
  Plynx.run();
}
