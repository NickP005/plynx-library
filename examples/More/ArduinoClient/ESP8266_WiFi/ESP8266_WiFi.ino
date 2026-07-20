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
  This sketch shows how to access WiFiClient directly in Plynx

  1. This gives you full control of the connection process.
  2. Shows a sensible way of integrating other connectivity hardware,
     that was not supported by Plynx out-of-the-box.

  NOTE: This requires ESP8266 support package:
       https://github.com/esp8266/Arduino

  Please be sure to select the right ESP8266 module
    in the Tools -> Board menu!

 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

/* Fill-in your Template ID (legacy: not needed for Plynx) */
//#define PLYNX_TEMPLATE_ID   "YourTemplateID"


#include <ESP8266WiFi.h>
#include <PlynxSimpleStream.h>

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = "YourNetworkName";
const char* pass = "YourPassword";

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

WiFiClient wifiClient;

// This function tries to connect to the cloud using TCP
bool connectPlynx()
{
  wifiClient.stop();
  return wifiClient.connect(PLYNX_DEFAULT_DOMAIN, PLYNX_DEFAULT_PORT);
}

// This function tries to connect to your WiFi network
void connectWiFi()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);

  if (pass && strlen(pass)) {
    WiFi.begin((char*)ssid, (char*)pass);
  } else {
    WiFi.begin((char*)ssid);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}


void setup()
{
  // Debug console
  Serial.begin(9600);

  connectWiFi();

  connectPlynx();

  Plynx.begin(wifiClient, auth);
}

void loop()
{
  // Reconnect WiFi
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
    return;
  }

  // Reconnect to Plynx Cloud
  if (!wifiClient.connected()) {
    connectPlynx();
    return;
  }

  Plynx.run();
}

