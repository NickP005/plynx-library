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
  Attention! Please check out TinyGSM guide:
    https://tiny.cc/tinygsm-readme

  Change GPRS apm, user, pass, and Plynx auth token to run :)
  Feel free to apply it to any other example. It's simple!

 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


// Arduino MKR GSM 1400 uses U-blox modem
#define TINY_GSM_MODEM_UBLOX

// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
//#define PLYNX_HEARTBEAT 30

#include <TinyGsmClient.h>
#include <PlynxSimpleTinyGSM.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Your GPRS credentials
// Leave empty, if missing user or pass
char apn[]  = "YourAPN";
char user[] = "";
char pass[] = "";

TinyGsm modem(SerialGSM);

void setup()
{
  // Debug console
  Serial.begin(9600);

  delay(10);

  // Set GSM module baud rate
  SerialGSM.begin(115200);

  pinMode(GSM_DTR, OUTPUT);
  digitalWrite(GSM_DTR, LOW);
  delay(5);

  // Turn on the GSM module by triggering GSM_RESETN pin
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  delay(1000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");

  Plynx.begin(auth, modem, apn, user, pass);
}

void loop()
{
  Plynx.run();
}

