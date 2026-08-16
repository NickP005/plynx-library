/*************************************************************
  Download latest Plynx library here:
    https://github.com/NickP005/plynx-library/releases/latest

  Plynx is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.

    Downloads, docs, tutorials: https://www.plynx.cc
    Examples and widgets:       https://www.plynx.cc/docs/widgets/

  Plynx library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  ESP32-CAM: una foto sul telefono, come valore di un pin.

  L'idea in una riga: l'immagine E' il valore del pin V7. Il pin non
  porta i byte, porta il loro riferimento. Chi scrive un altro valore
  su V7 — un numero, una parola, un'altra foto — fa sparire l'immagine
  precedente. Non e' una regola in piu': e' la regola di sempre,
  l'ultimo scritto vince.

  Il caso d'uso: una ESP32-CAM sulla porta del pollaio che scatta
  quando il movimento scatta. Sul telefono, accanto al pulsante della
  luce, c'e' l'ultima foto. Nessuno stream, nessun NVR.

  Cosa NON e': non e' videosorveglianza, non e' uno storico, non e' un
  album. Le foto restano al massimo 5 giorni, e ogni scatto sostituisce
  il precedente.

  IN APP:
    Widget Immagine sul pin virtuale V7.
    Il widget compare nella libreria solo se il tuo server sa ospitare
    le immagini (capability "imageCache"): server vecchi non lo mostrano
    affatto, invece di mostrarlo vuoto per sempre.

  HARDWARE:
    ESP32-CAM (AI-Thinker) + un modulo FTDI per programmarla.
    Il sensore PIR (opzionale) sul GPIO 13.

  ⚠️ IL TOKEN E' LA CHIAVE DI CASA. Con quel token si scrivono i pin del
     progetto E si leggono le foto dell'account. Non pubblicare questo
     sketch compilato con dentro il tuo token vero.

  ⚠️ LIBRERIE NECESSARIE, oltre a Plynx:
     nessuna. La camera (esp_camera.h) arriva col core ESP32 di Espressif.

 *************************************************************/

/* Comment this out to disable prints and save space */
#define PLYNX_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <PlynxSimpleEsp32.h>
#include "esp_camera.h"

// Se usi un server tuo, scrivilo QUI PRIMA di includere l'header.
//#define PLYNX_IMAGE_HOST "192.168.1.60"
#include <PlynxImageUpload.h>

// You should get Auth Token in the Plynx App.
// Go to the Project Settings (nut icon).
char auth[] = "YourAuthToken";

// Your WiFi credentials.
char ssid[] = "YourNetworkName";
char pass[] = "YourPassword";

// Il pin virtuale su cui vive la foto: lo stesso del widget Immagine.
#define PIN_FOTO 7

// Sensore di movimento. Metti -1 se non ne hai uno: in quel caso lo
// sketch scatta a intervalli regolari.
#define PIN_PIR 13

// Quanto aspettare, come minimo, fra due foto. Non e' prudenza: e' la
// quota. Ogni account ha 20 MB di spazio foto, e un JPEG VGA pesa
// ~30 KB — a una foto al secondo lo spazio finisce in dieci minuti, e
// da li' in poi il server rifiuta con 507.
#define ATTESA_MINIMA_MS 30000

// ---- Pinout AI-Thinker ESP32-CAM ------------------------------------
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

unsigned long ultimoScatto = 0;

bool avviaCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // VGA e non UXGA, di proposito: il widget sul telefono e' grande come
  // una carta di credito, e una foto da 300 KB consuma la quota
  // dell'account dieci volte piu' in fretta senza che si veda meglio.
  config.frame_size   = FRAMESIZE_VGA;
  config.jpeg_quality = 12;
  config.fb_count     = 1;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera non partita: 0x%x\n", err);
    return false;
  }
  return true;
}

void scattaEPubblica() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Scatto fallito.");
    return;
  }

  Serial.printf("Scattata: %u byte. Mando...\n", fb->len);
  int esito = plynxUploadImage(auth, PIN_FOTO, fb->buf, fb->len);

  // ⚠️ L'esito si guarda SEMPRE. Una foto rifiutata non lascia nessun
  // segno sulla scheda: senza queste righe la telecamera continuerebbe
  // a scattare per giorni credendo di pubblicare.
  switch (esito) {
    case 200:
      Serial.println("Pubblicata: è sul telefono.");
      break;
    case 401:
      Serial.println("Token rifiutato: controlla auth[].");
      break;
    case 413:
      Serial.println("Foto troppo grande (max 1 MB): abbassa frame_size.");
      break;
    case 415:
      // Capita se il sensore restituisce un buffer che non è JPEG: succede
      // quando pixel_format non è PIXFORMAT_JPEG. Il server guarda i primi
      // byte e rifiuta, invece di tenersi in casa roba che nessuno sa aprire.
      Serial.println("Non è un JPEG: controlla pixel_format = PIXFORMAT_JPEG.");
      break;
    case 429:
      // Non è un errore da correggere, è un freno: fra due foto ci vuole
      // almeno mezzo secondo. Se lo vedi spesso, alza INTERVALLO_MINIMO.
      Serial.println("Troppo in fretta: aspetta di più fra una foto e l'altra.");
      break;
    case 507:
      Serial.println("Spazio esaurito sull'account (20 MB): le vecchie scadono da sole in 5 giorni.");
      break;
    default:
      Serial.printf("Non pubblicata, esito %d.\n", esito);
      break;
  }

  esp_camera_fb_return(fb);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  if (!avviaCamera()) {
    Serial.println("Mi fermo qui: senza camera non c'è niente da mandare.");
    return;
  }

  if (PIN_PIR >= 0) {
    pinMode(PIN_PIR, INPUT);
  }

  Plynx.begin(auth, ssid, pass);
}

void loop() {
  Plynx.run();

  bool movimento = (PIN_PIR >= 0) && digitalRead(PIN_PIR) == HIGH;
  bool aTempo    = (PIN_PIR < 0);

  if ((movimento || aTempo) && millis() - ultimoScatto > ATTESA_MINIMA_MS) {
    ultimoScatto = millis();
    scattaEPubblica();
  }
}
