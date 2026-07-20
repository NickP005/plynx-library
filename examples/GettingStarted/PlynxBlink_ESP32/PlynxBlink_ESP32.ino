/*
 * Plynx quickstart for ESP32.
 *
 * What this does: your phone talks to the ESP32 in real time.
 * Press a button on virtual pin V1 in the Plynx app and the LED
 * on GPIO 2 turns on or off instantly.
 *
 * Get the auth token from the Plynx app when you add a board.
 * Default server: yes.plynx.cc
 *
 * The Plynx wire protocol is compatible with legacy Blynk firmware,
 * so classic sketches port over by swapping the includes and the
 * global object name.
 */

// Board specific header. It defines the global Plynx object for you.
#include <PlynxSimpleEsp32.h>

// Auth token from the Plynx app (Board settings -> Auth token).
char auth[] = "YourAuthToken";

// Your WiFi credentials. Leave the password empty for open networks.
char ssid[] = "YOUR_WIFI";
char pass[] = "YOUR_PASSWORD";

// GPIO 2 drives the onboard LED on most ESP32 dev boards.
#define LED_PIN 2

// Called every time virtual pin V1 changes value from the app.
PLYNX_WRITE(V1)
{
  int value = param.asInt();          // 0 or 1 from a button widget
  digitalWrite(LED_PIN, value ? HIGH : LOW);
}

void setup()
{
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);         // start with the LED off

  // Connect to WiFi and to the default Plynx server (yes.plynx.cc).
  // For a private server use:
  //   Plynx.begin(auth, ssid, pass, "your.server", 8080);
  Plynx.begin(auth, ssid, pass);
}

void loop()
{
  Plynx.run();                        // keep the connection alive
}
