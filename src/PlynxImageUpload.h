/**
 * @file       PlynxImageUpload.h
 * @author     Plynx
 * @license    This project is released under the MIT License (MIT)
 * @date       Aug 2026
 * @brief      Pubblica un'immagine come VALORE di un pin virtuale.
 *
 * L'idea, in una riga: **l'immagine è il valore del pin**. Il pin non porta i
 * byte, porta il loro riferimento. Chi scrive un altro valore su quel pin —
 * un'altra foto, un numero, una parola — fa sparire l'immagine, perché è la
 * stessa regola di sempre: l'ultimo scritto vince.
 *
 * Uso, dentro uno sketch che ha già `auth[]`:
 *
 *     #include <PlynxImageUpload.h>
 *     ...
 *     int esito = plynxUploadImage(auth, 7, fb->buf, fb->len);
 *     if (esito == 200) { ... }
 *
 * ⚠️ **Non passa dal protocollo Plynx**, e non può: quel filo trasporta
 * messaggi corti, non trecento kilobyte di JPEG. I byte viaggiano su HTTP, e
 * il server — ricevendoli — scrive lui stesso il valore sul pin. Dal telefono
 * non cambia niente: arriva un valore di pin come tutti gli altri.
 *
 * ⚠️ **Il token è la chiave di casa.** È la stessa credenziale con cui si
 * scrivono i pin, e con questa funzione apre anche le foto dell'account: non
 * va pubblicato su GitHub insieme allo sketch. È il motivo per cui questa
 * funzione lo chiede ogni volta invece di tenerselo in una variabile globale —
 * chi lo scrive deve vederlo.
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
/** Lo stesso server a cui parla lo sketch. Cambialo se usi un server tuo. */
#define PLYNX_IMAGE_HOST "plynx.cc"
#endif

#ifndef PLYNX_IMAGE_PORT
/**
 * La 9443 e non la 8080: il token e i byte della foto viaggiano cifrati.
 * Sulla porta in chiaro chiunque sia sulla stessa rete — un wifi di un bar, un
 * access point di casa condiviso — leggerebbe il token, e col token si scrivono
 * i pin di tutto il progetto.
 */
#define PLYNX_IMAGE_PORT 9443
#endif

#ifndef PLYNX_IMAGE_TIMEOUT_MS
#define PLYNX_IMAGE_TIMEOUT_MS 15000
#endif

/**
 * Manda i byte e lascia che il server scriva il pin.
 *
 * @param auth   il token della scheda (quello dello sketch)
 * @param vpin   il pin virtuale, es. 7 per V7
 * @param data   i byte dell'immagine (JPEG o PNG)
 * @param len    quanti byte
 * @return       il codice HTTP: 200 fatto · 401 token sbagliato ·
 *               413 foto troppo grande (max 1 MB) · 507 spazio esaurito ·
 *               negativo se non si è nemmeno riusciti a parlare col server
 *
 * ⚠️ Il valore di ritorno **va guardato**. Una foto rifiutata per quota o per
 * dimensione non lascia nessun segno sulla scheda: senza questo controllo la
 * telecamera continuerebbe a scattare per giorni credendo di pubblicare.
 */
inline int plynxUploadImage(const char* auth, uint8_t vpin,
                            const uint8_t* data, size_t len) {
    if (auth == NULL || data == NULL || len == 0) {
        return -1;
    }

    WiFiClientSecure client;
    // Il server Plynx usa spesso un certificato autofirmato (installazioni di
    // casa, Raspberry). Senza questa riga la connessione fallirebbe proprio
    // dove serve. Cifra il traffico ma non verifica l'identità del server:
    // per un certificato vero, sostituisci con client.setCACert(...).
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
