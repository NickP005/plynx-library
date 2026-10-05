// qemu_testa_chip: due pezzi della 1.3.0 eseguiti sul chip vero, emulato in
// QEMU (lo lancia ../qemu_testa_chip.py, che legge le righe "BANCO").
//
//  1. La testa dell'immagine (Plynx/PlynxOta.h, plynxOtaHeadChipOk): con lo
//     stesso giro del download (controllo, poi Update.write), un'immagine di un
//     altro chip si ferma ai primi 14 byte, la propria passa fino a Update.end().
//     Accanto, il giro SENZA controllo: l'immagine straniera si scrive tutta e la
//     rifiuta il bootloader in fondo («mismatch chip ID»), che e' la 1.2.0.
//  2. "ota-err" nel blnkinf (PlynxApiArduino.h): c'e' una volta sola dopo un
//     errore, sparisce al collegamento dopo, e quando non ci sta intero non c'e'
//     per niente (una chiave senza valore sposterebbe tutte le coppie dopo).
//
// L'immagine straniera sta nella partizione spiffs: "ALIENIMG", uint32 della
// sua lunghezza e uint32 della lunghezza dell'immagine propria (LE), e
// l'immagine a 0x1000. La lunghezza propria la scrive chi prepara la flash:
// ESP.getSketchSize() la ricava verificando tutta l'immagine, e sulla S3 in
// QEMU non torna (05/10: fermo 10 minuti li', e di nuovo dentro sendInfo(),
// che la chiama per "sk-size"). Per questo anche "sk-size" qui la prende da
// quel numero: e' la stessa lunghezza, letta dall'intestazione.

// PLYNX_PROJECT e' letto a ogni sendInfo(): una variabile permette di riempire
// il buffer dinamico quanto serve alla prova del "non ci sta". Il provisioning
// SoftAP lo usa invece come letterale, quindi resta fuori (PLYNX_NO_PROVISION:
// toglie solo "cm" dalla parte fissa, il buffer dinamico e' lo stesso).
const char* progettoDiProva = NULL;
#define PLYNX_PROJECT progettoDiProva
#define PLYNX_NO_PROVISION
uint32_t dimensionePropria = 0;
#define PLYNX_INFO_FLASH_SKETCH (dimensionePropria)

#include <WiFi.h>
#include <WiFiClient.h>
#include <PlynxSimpleEsp32.h>
#include <Update.h>
#include <esp_partition.h>
#include <esp_ota_ops.h>

// I tipi stanno prima di ogni funzione: arduino-cli mette i prototipi delle
// funzioni davanti alla prima che trova, e un prototipo che nomina Sorgente
// prima del tipo non compila.

struct Sorgente {
    const esp_partition_t* part;
    uint32_t base;
    uint32_t size;
};

// Il trasporto spia: tiene quello che il protocollo scrive.
struct Spia {
    uint8_t out[2048];
    size_t n = 0;
    void begin(const char*, uint16_t) {}
    void begin(IPAddress, uint16_t) {}
    bool connect() { return true; }
    void disconnect() {}
    size_t read(void*, size_t) { return 0; }
    size_t write(const void* b, size_t l)
    {
        if (n + l <= sizeof(out)) {
            memcpy(out + n, b, l);
            n += l;
        }
        return l;
    }
    bool connected() { return true; }
    int available() { return 0; }
};

class Banco : public PlynxProtocol<Spia> {
public:
    Banco(Spia& s) : PlynxProtocol<Spia>(s), spia(s) {}
    // Il blocco blnkinf come lo manda sendInfo(): il corpo del frame INTERNAL
    // che comincia con "ver". Torna la lunghezza, 0 se non c'e'.
    size_t info(uint8_t* corpo, size_t max)
    {
        spia.n = 0;
        state = CONNECTED;
        sendInfo();
        size_t i = 0;
        while (i + 5 <= spia.n) {
            const uint8_t cmd = spia.out[i];
            const size_t len = ((size_t)spia.out[i + 3] << 8) | spia.out[i + 4];
            if (cmd == PLYNX_CMD_INTERNAL && len >= 4 && memcmp(spia.out + i + 5, "ver", 4) == 0) {
                const size_t l = len < max ? len : max;
                memcpy(corpo, spia.out + i + 5, l);
                return l;
            }
            i += 5 + len;
        }
        return 0;
    }
private:
    Spia& spia;
};

static int rotte = 0;

static void dico(bool bene, const char* cosa)
{
    if (!bene) {
        rotte++;
    }
    Serial.printf("BANCO %s %s\n", bene ? "ok" : "ko", cosa);
}

// ---- 1. la testa dell'immagine ------------------------------------------

// I pezzi del download hanno misure qualsiasi: i primi tre tagliano la testa
// di 14 byte in 5 + 9 e poi 1, come puo' fare un client.available() qualsiasi.
static size_t pezzo(int i, uint32_t resta)
{
    static const size_t primi[] = { 5, 9, 1 };
    size_t n = i < 3 ? primi[i] : 1024;
    return n < resta ? n : resta;
}

// Il giro del download di PlynxOta.h: controllo della testa, poi Update.write.
// Torna i byte passati a Update.write; *rifiutata dice se si e' fermato il
// controllo, *finita se Update.end() ha accettato.
static uint32_t scrivi(const Sorgente& s, bool conControllo, bool* rifiutata, bool* finita)
{
    *rifiutata = false;
    *finita = false;
    if (!Update.begin(s.size, U_FLASH)) {
        Serial.printf("BANCO nota Update.begin: %s\n", Update.errorString());
        return 0;
    }
    static uint8_t buf[1024];
    uint8_t head[14];
    uint32_t scritti = 0;
    for (int i = 0; scritti < s.size; i++) {
        const size_t n = pezzo(i, s.size - scritti);
        esp_partition_read(s.part, s.base + scritti, buf, n);
#ifdef PLYNX_OTA_CHECKS_CHIP
        if (conControllo && !plynxOtaHeadChipOk(head, scritti, buf, n)) {
            Update.abort();
            *rifiutata = true;
            return scritti;
        }
#else
        (void)head;
        (void)conControllo;
#endif
        if (Update.write(buf, n) != n) {
            Serial.printf("BANCO nota Update.write a %u: %s\n", (unsigned)scritti, Update.errorString());
            Update.abort();
            return scritti;
        }
        scritti += n;
    }
    *finita = Update.end();
    if (!*finita) {
        Serial.printf("BANCO nota Update.end: %s\n", Update.errorString());
    }
    return scritti;
}

static void provaLaTesta()
{
#ifdef PLYNX_OTA_CHECKS_CHIP
    dico(true, "PLYNX_OTA_CHECKS_CHIP e' acceso (CONFIG_IDF_FIRMWARE_CHIP_ID c'e')");
#else
    dico(false, "PLYNX_OTA_CHECKS_CHIP e' acceso (CONFIG_IDF_FIRMWARE_CHIP_ID c'e')");
#endif
    Serial.printf("BANCO nota CONFIG_IDF_FIRMWARE_CHIP_ID=%d\n", (int)CONFIG_IDF_FIRMWARE_CHIP_ID);

    const esp_partition_t* mia = esp_ota_get_running_partition();
    const esp_partition_t* sp = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, NULL);
    uint8_t h[16] = { 0 };
    if (sp) {
        esp_partition_read(sp, 0, h, sizeof(h));
    }
    if (!sp || memcmp(h, "ALIENIMG", 8) != 0) {
        dico(false, "l'immagine straniera e' nella spiffs");
        return;
    }
    Sorgente straniera = { sp, 0x1000, 0 };
    memcpy(&straniera.size, h + 8, 4);
    Sorgente propria = { mia, 0, 0 };
    memcpy(&propria.size, h + 12, 4);
    dimensionePropria = propria.size;
    uint8_t t[14];
    esp_partition_read(sp, 0x1000, t, sizeof(t));
    Serial.printf("BANCO nota straniera %u byte chip_id %u, propria %u byte\n",
                  (unsigned)straniera.size, (unsigned)(t[12] | (t[13] << 8)), (unsigned)propria.size);

    bool rif, fin;
    char riga[160];

    Serial.println("BANCO nota scrivo la straniera col controllo");
    uint32_t n = scrivi(straniera, true, &rif, &fin);
    snprintf(riga, sizeof(riga), "straniera COL controllo: rifiutata dalla testa dopo %u byte (attesi 5: il primo pezzo, prima dei 14)", (unsigned)n);
    dico(rif && !fin && n == 5, riga);

    Serial.println("BANCO nota scrivo la propria col controllo");
    n = scrivi(propria, true, &rif, &fin);
    snprintf(riga, sizeof(riga), "propria COL controllo: passa tutta (%u byte) e Update.end() accetta", (unsigned)n);
    dico(!rif && fin && n == propria.size, riga);

    Serial.println("BANCO nota scrivo la straniera senza controllo");
    n = scrivi(straniera, false, &rif, &fin);
    snprintf(riga, sizeof(riga), "straniera SENZA controllo (la 1.2.0): scritta tutta (%u byte), rifiutata solo da Update.end()", (unsigned)n);
    dico(!rif && !fin && n == straniera.size, riga);

    // Una testa che non e' un'immagine (magic diverso): il controllo non giudica,
    // il perche' lo dice Update.write. Qui solo la funzione, senza flash.
#ifdef PLYNX_OTA_CHECKS_CHIP
    uint8_t finta[16] = { 0 };
    uint8_t hh[14];
    dico(plynxOtaHeadChipOk(hh, 0, finta, sizeof(finta)), "una testa senza 0xE9 non e' giudicata dal controllo");
#endif
    // Il chip cambiato di un bit nell'intestazione propria: rifiutata.
#ifdef PLYNX_OTA_CHECKS_CHIP
    uint8_t vera[16];
    esp_partition_read(mia, 0, vera, sizeof(vera));
    vera[12] ^= 0x01;
    uint8_t h2[14];
    dico(!plynxOtaHeadChipOk(h2, 0, vera, sizeof(vera)), "l'intestazione propria con il chip_id cambiato si rifiuta");
#endif
}

// ---- 2. "ota-err" nel blnkinf ---------------------------------------------

// Le coppie del blocco: le chiavi sono nei posti pari. Torna il valore di
// `chiave` (NULL se manca) e dice se le coppie sono intere (numero pari di
// pezzi: una chiave senza valore lo rende dispari).
static const char* valore(const uint8_t* c, size_t l, const char* chiave, bool* pari)
{
    const char* trovato = NULL;
    size_t pezzi = 0;
    size_t i = 0;
    while (i < l) {
        const char* s = (const char*)c + i;
        const size_t sl = strnlen(s, l - i);
        if (pezzi % 2 == 0 && strcmp(s, chiave) == 0 && i + sl + 1 < l) {
            trovato = s + sl + 1;
        }
        pezzi++;
        i += sl + 1;
    }
    *pari = (pezzi % 2) == 0;
    return trovato;
}

static void provaOtaErr()
{
#ifndef PLYNX_INFO_OTA_ERR
    dico(false, "PLYNX_INFO_OTA_ERR e' definito con l'OTA di serie");
    return;
#else
    static Spia spia;
    static Banco banco(spia);
    uint8_t c[512];
    bool pari;
    char riga[200];

    size_t l = banco.info(c, sizeof(c));
    const char* cpu = valore(c, l, "cpu", &pari);
    snprintf(riga, sizeof(riga), "il blocco che parte dice cpu=%s", cpu ? cpu : "(niente)");
    dico(cpu && strcmp(cpu, CONFIG_IDF_TARGET) == 0, riga);
    dico(l > 0 && valore(c, l, "ota-err", &pari) == NULL && pari, "senza errori niente \"ota-err\"");

    plynxOtaLastError = "chip";
    l = banco.info(c, sizeof(c));
    const char* e = valore(c, l, "ota-err", &pari);
    snprintf(riga, sizeof(riga), "dopo un errore \"ota-err\" = %s, coppie intere", e ? e : "(niente)");
    dico(e && strcmp(e, "chip") == 0 && pari, riga);
    dico(valore(c, l, "ota-max", &pari) != NULL, "e le chiavi di prima ci sono ancora (ota-max)");

    l = banco.info(c, sizeof(c));
    dico(valore(c, l, "ota-err", &pari) == NULL && pari, "al collegamento dopo \"ota-err\" non c'e' piu'");

    // Il buffer dinamico (160 byte) quasi pieno: "proj" sempre piu' lungo, e a
    // ogni lunghezza un errore. Prima "ota-err" c'e'; dalla prima lunghezza in
    // cui non ci sta piu' (ota-max, flash e sk-size ancora si') NON deve
    // comparire, e le coppie devono restare intere.
    static char lungo[200];
    bool provato = false;
    bool primaCera = false;
    for (size_t k = 40; k < 150 && !provato; k++) {
        memset(lungo, 'p', k);
        lungo[k] = '\0';
        progettoDiProva = lungo;
        plynxOtaLastError = "chip";
        l = banco.info(c, sizeof(c));
        const char* e2 = valore(c, l, "ota-err", &pari);
        const bool coppie = pari;
        const bool sk = valore(c, l, "sk-size", &pari) != NULL;
        if (!sk) {
            break; // proj ha tolto il posto anche alle chiavi di prima: oltre non serve
        }
        if (e2) {
            primaCera = true;
            continue;
        }
        snprintf(riga, sizeof(riga),
                 "con proj di %u byte \"ota-err\" non ci sta: non c'e', coppie %s, l'errore e' consumato (%s)",
                 (unsigned)k, coppie ? "intere" : "ROTTE", plynxOtaLastError == NULL ? "si" : "no");
        dico(primaCera && coppie && plynxOtaLastError == NULL, riga);
        provato = true;
    }
    if (!provato) {
        dico(false, "trovata una lunghezza di proj che lascia fuori \"ota-err\"");
    }
    progettoDiProva = NULL;
#endif
}

void setup()
{
    Serial.begin(115200);
    delay(200);
#ifdef PLYNX_INFO_CPU
    Serial.printf("\nBANCO su %s, Plynx " PLYNX_VERSION ", PLYNX_INFO_CPU=%s\n", ESP.getChipModel(), PLYNX_INFO_CPU);
#else
    Serial.printf("\nBANCO su %s, Plynx " PLYNX_VERSION ", PLYNX_INFO_CPU assente\n", ESP.getChipModel());
#endif
    provaLaTesta();
    provaOtaErr();
    Serial.printf("BANCO fine rotte=%d\n", rotte);
}

void loop()
{
    delay(1000);
}
