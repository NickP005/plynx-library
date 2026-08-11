# Plynx C++ Library

Control your Arduino, ESP32 or ESP8266 from the Plynx app. Your board and your phone talk to each other in real time over WiFi, BLE, Bluetooth, Ethernet, GSM, USB or Serial.

## Install

Arduino IDE:

- Download the ZIP from GitHub: [NickP005/plynx-library](https://github.com/NickP005/plynx-library), then in the IDE go to `Sketch > Include Library > Add .ZIP Library...` and select the file.
- Or install it from the Library Manager (search for "Plynx") when it becomes available there.

## Quick start

This sketch connects an ESP8266 to the default Plynx server (`yes.plynx.cc`) and turns an LED on or off from virtual pin V1 in the app.

The `auth` token comes from the Plynx app: create or open a board and add the device to get its token.

```cpp
#define PLYNX_PRINT Serial

#include <PlynxSimpleEsp8266.h>

char auth[] = "YourAuthToken";   // from the Plynx app
char ssid[] = "YourWiFiName";
char pass[] = "YourWiFiPassword";

const int ledPin = LED_BUILTIN;

// Called when V1 changes in the app
PLYNX_WRITE(V1)
{
  int value = param.asInt();
  digitalWrite(ledPin, value ? LOW : HIGH); // LED_BUILTIN is active low
}

PLYNX_CONNECTED()
{
  PLYNX_PRINT.println("Connected to Plynx");
}

void setup()
{
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);

  Plynx.begin(auth, ssid, pass); // default server: yes.plynx.cc
}

void loop()
{
  Plynx.run();
}
```

Send data the other way with `Plynx.virtualWrite(V1, value)`, for example to push a sensor reading to the app.

Each board has its own header. Include the one that matches your hardware:

- `<PlynxSimpleEsp8266.h>`
- `<PlynxSimpleEsp32.h>`
- `<PlynxSimpleEthernet.h>`

## Self-hosted / private server

The default `Plynx.begin(auth, ssid, pass)` connects to `yes.plynx.cc` on port `8080`. This is the legacy hardware endpoint and traffic is unencrypted, so treat it accordingly.

To point the board at your own server, pass the host and port:

```cpp
Plynx.begin(auth, ssid, pass, "your.server.com", 8080);
```

You can also use a raw IP address:

```cpp
IPAddress serverIp(192, 168, 1, 100);
Plynx.begin(auth, ssid, pass, serverIp, 8080);
```

## Updates over the air

Since 1.0.5 the ESP32 and ESP8266 shells carry the update handler by default:
the board can receive a new firmware from the server and reboot into it, with
no extra code in the sketch. It also reports how much space it has for an
update, so the server knows in advance whether a build fits.

Nothing to call — `Plynx.begin()` and `Plynx.run()` are enough. To leave the
handler out (a board that must never update itself, or a sketch that needs the
flash for something else) define this before the include:

```cpp
#define PLYNX_NO_OTA
#include <PlynxSimpleEsp32.h>
```

The handler costs about 7.5 KB of flash.

### A bad update undoes itself (`<PlynxSimpleEsp32.h>`)

An update is not final the moment it is written. The new firmware boots **on
probation**: it becomes permanent only once it has opened its first
authenticated session with the server. Until then the ESP32 bootloader is
holding the previous firmware, and it will bring it back — by itself, without
running a line of the new image. That is what saves a board whose new firmware
crashes before `setup()` even starts, which is exactly the case no software
watchdog can cover.

Read this part twice, because it is the part that surprises people:

> **Any reset before that first authenticated session undoes the update** —
> not only the window running out. A power cut, a watchdog, an `ESP.restart()`
> the sketch does itself, the setup hotspot rebooting after new WiFi
> credentials: each of them lands while the image is still on probation, and
> the board comes back on the old firmware. The server will simply push the
> update again.

For most fleets that is exactly the wanted behaviour: a board the server
cannot reach is a board nobody can fix remotely. For one that is offline for
long stretches by design, or for a sketch that spends minutes in `setup()`
before calling `Plynx.begin()`, widen the window:

```cpp
#define PLYNX_OTA_GUARD_MS 600000UL      // ten minutes instead of three
#include <PlynxSimpleEsp32.h>
```

To leave the bootloader hook alone entirely — you keep OTA, you lose the
automatic undo:

```cpp
#define PLYNX_NO_BOOTLOADER_ROLLBACK
#include <PlynxSimpleEsp32.h>
```

That one is **required** next to ESP RainMaker, which defines the same core
hook; without it the link fails with `multiple definition of
verifyRollbackLater`.

Two limits worth knowing. This lives in `<PlynxSimpleEsp32.h>` only: the SSL,
BT and BLE shells (and `Plynx.Edgent`, which is built on the SSL one) carry
neither the OTA handler nor this. And while the image is on probation Plynx is
the only thing that can confirm it, so an update written by ArduinoOTA or
`esp_https_ota` in the same sketch also waits for a Plynx session.

The ESP8266 has a single application slot — the new image is copied over the
old one at boot — so none of this applies there and no rollback is possible.

## Compatibility

Plynx speaks the classic Blynk Legacy protocol, so it also works with Blynk-legacy servers and existing sketches. You can point the same firmware at either a Plynx server or a Blynk-legacy server by changing the host in `begin()`.

Plynx is an independent project, not affiliated with, sponsored by, or endorsed by Blynk Inc. The Blynk name is used only to describe protocol compatibility. "Blynk" is a trademark of Blynk Inc.

## Credits

The Plynx C++ Library is a fork and derivative of the [Blynk library](https://github.com/blynkkk/blynk-library) by Volodymyr Shymanskyy, released under the MIT License. The original authorship is kept in the source files, and this library ships under the same MIT License. Thanks to the original author and contributors.

## Links

- Plynx on the App Store: https://apps.apple.com/us/app/plynx-arduino-esp32-control/id6756375448
- Website: https://www.plynx.cc
