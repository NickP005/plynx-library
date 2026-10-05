/**
 * @file       PlynxImageUpload.h
 * @author     Plynx
 * @license    This project is released under the MIT License (MIT)
 * @date       Aug 2026
 * @brief      Publishes a picture as the value of a virtual pin.
 *
 * The picture is the value of the pin. The pin carries a reference to the
 * bytes, and a new value written to the same pin, another picture, a number
 * or a word, replaces the picture, as for any value.
 *
 * The bytes travel over HTTPS, outside the Plynx connection, which carries
 * short messages. The server receives them and writes the value of the pin
 * itself, so the phone sees a pin value like any other.
 *
 * The token is the same key that writes the pins, and with this function it
 * also opens the pictures of the account. Keep it out of sketches you publish.
 * The function asks for it at every call, so whoever writes the call sees it.
 */

#ifndef PlynxImageUpload_h
#define PlynxImageUpload_h

#if defined(ESP32) || defined(ESP8266)

#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiClientSecure.h>
#else
  #include <ESP8266WiFi.h>
  #include <WiFiClientSecure.h>
#endif

#ifndef PLYNX_IMAGE_HOST
/**
 * @brief The server that receives the pictures, "plynx.cc".
 *
 * Change it, above the include, when the board talks to your own server.
 *
 * @code
 * #define PLYNX_IMAGE_HOST "my.server.com"
 * #include <PlynxSimpleEsp32.h>
 * #include <PlynxImageUpload.h>
 * @endcode
 *
 * @see PLYNX_IMAGE_PORT
 */
#define PLYNX_IMAGE_HOST "plynx.cc"
#endif

#ifndef PLYNX_IMAGE_PORT
/**
 * @brief The encrypted port that receives the pictures, 9443.
 *
 * The token and the bytes of the picture travel encrypted. On a plain port
 * anyone on the same network, such as the WiFi of a cafe, could read the
 * token, and the token writes every pin of the project.
 *
 * @code
 * #define PLYNX_IMAGE_PORT 9443
 * #include <PlynxSimpleEsp32.h>
 * #include <PlynxImageUpload.h>
 * @endcode
 *
 * @see PLYNX_IMAGE_HOST
 */
#define PLYNX_IMAGE_PORT 9443
#endif

#ifndef PLYNX_IMAGE_TIMEOUT_MS
/**
 * @brief How long, in milliseconds, plynxUploadImage() waits for the server.
 * The default is 15000.
 *
 * @code
 * #define PLYNX_IMAGE_TIMEOUT_MS 30000
 * #include <PlynxSimpleEsp32.h>
 * #include <PlynxImageUpload.h>
 * @endcode
 */
#define PLYNX_IMAGE_TIMEOUT_MS 15000
#endif

/**
 * @brief Sends a JPEG or PNG picture to the server, which shows it on a
 * virtual pin.
 *
 * Only on ESP32 and ESP8266. The call waits until the server answers.
 *
 * @param auth  The auth token of the board, the one of the sketch.
 * @param vpin  The virtual pin, 7 for V7.
 * @param data  The bytes of the picture.
 * @param len   How many bytes.
 * @return The answer of the server. 200 means published. 401 means a wrong
 *         token. 413 means a picture larger than 1 MB. 415 means the bytes
 *         are not a JPEG or a PNG. 429 means too soon, since the server takes
 *         at most one picture every half second from a board. 507 means the
 *         space of the account is full. A negative number means the board
 *         could not talk to the server at all.
 *
 * @par Where to call
 * Where a picture is ready, for example after a camera shot, never at every
 * turn of loop().
 *
 * @par App widget
 * Image, set to the same virtual pin.
 *
 * @code
 * #include <PlynxSimpleEsp32.h>
 * #include <PlynxImageUpload.h>
 *
 * char auth[] = "YourAuthToken";
 *
 * void publish(const uint8_t* jpeg, size_t length)
 * {
 *   int answer = plynxUploadImage(auth, 7, jpeg, length);
 *   if (answer != 200) {
 *     Serial.print("Picture refused, code ");
 *     Serial.println(answer);
 *   }
 * }
 * @endcode
 *
 * @warning Check the value it returns. A picture refused for size or for
 * space leaves no other trace on the board, and a camera that ignores the
 * answer keeps shooting while nothing is published.
 */
inline int plynxUploadImage(const char* auth, uint8_t vpin,
                            const uint8_t* data, size_t len) {
    if (auth == NULL || data == NULL || len == 0) {
        return -1;
    }

    WiFiClientSecure client;
    // A Plynx server often has a self-signed certificate (home installs, a
    // Raspberry Pi), and without this line the connection would fail exactly
    // there. The traffic is encrypted but the identity of the server is not
    // checked. For a real certificate, use client.setCACert(...) instead.
    client.setInsecure();
    client.setTimeout(PLYNX_IMAGE_TIMEOUT_MS / 1000);

    if (!client.connect(PLYNX_IMAGE_HOST, PLYNX_IMAGE_PORT)) {
        return -2;
    }

    // L'intestazione a mano invece di HTTPClient: su una ESP32-CAM la RAM è il
    // vincolo vero, e il buffer del frame è già grande. Così i byte vanno sul
    // socket senza una seconda copia.
    client.print(F("POST /v1/img/"));
    client.print(auth);
    client.print(F("/V"));
    client.print(vpin);
    client.print(F(" HTTP/1.1\r\nHost: "));
    client.print(F(PLYNX_IMAGE_HOST));
    client.print(F("\r\nContent-Type: image/jpeg\r\nContent-Length: "));
    client.print(len);
    client.print(F("\r\nConnection: close\r\n\r\n"));

    // A blocchi: una write unica da 300 KB fa fallire lo stack TLS su ESP8266
    // e mette in difficoltà anche l'ESP32.
    const size_t BLOCCO = 1024;
    size_t inviati = 0;
    while (inviati < len) {
        size_t quanti = (len - inviati) > BLOCCO ? BLOCCO : (len - inviati);
        size_t scritti = client.write(data + inviati, quanti);
        if (scritti == 0) {
            client.stop();
            return -3;
        }
        inviati += scritti;
        yield();
    }

    // La riga di stato basta: il corpo lo leggiamo solo per non lasciare
    // spazzatura nel socket prima di chiuderlo.
    unsigned long scadenza = millis() + PLYNX_IMAGE_TIMEOUT_MS;
    while (client.connected() && !client.available() && millis() < scadenza) {
        delay(10);
    }
    if (!client.available()) {
        client.stop();
        return -4;
    }

    String statusLine = client.readStringUntil('\n');
    int codice = -5;
    int spazio = statusLine.indexOf(' ');
    if (spazio > 0 && statusLine.length() > spazio + 4) {
        codice = statusLine.substring(spazio + 1, spazio + 4).toInt();
    }

    client.stop();
    return codice;
}

#endif // ESP32 || ESP8266
#endif // PlynxImageUpload_h
