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


// Select your modem:
#define TINY_GSM_MODEM_SIM800
//#define TINY_GSM_MODEM_SIM900
//#define TINY_GSM_MODEM_M590
//#define TINY_GSM_MODEM_A6
//#define TINY_GSM_MODEM_A7
//#define TINY_GSM_MODEM_BG96
//#define TINY_GSM_MODEM_XBEE

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

// Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

TinyGsm modem(SerialAT);

void setup()
{
  // Debug console
  Serial.begin(9600);

  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

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

