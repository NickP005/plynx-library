/**
 * @file       PlynxProvision.h
 * @license    This project is released under the MIT License (MIT)
 * @brief      Token-less provisioning for ESP32/ESP8266 WiFi transports.
 *
 * Powers the no-argument Plynx.begin(): when the board has no saved
 * configuration it starts a WPA2 SoftAP ("Plynx-XXXX", PSK "plynxsetup")
 * with a tiny raw-HTTP endpoint the Plynx app talks to:
 *
 *   GET /config?ssid=..&pass=..&token=..[&host=..&port=..]  -> save + reboot
 *   GET /status  -> {"status":"waiting"|"error","err":"network"|"token",...}
 *
 * Deliberately minimal: no WebServer, no DNS captive portal, no String.
 * Everything lives in inline members so the linker drops the whole thing
 * when the sketch uses the classic begin(auth, ssid, pass) instead
 * (zero-cost rule: classic users pay zero bytes).
 *
 * Testable without the app:
 *   curl "http://192.168.4.1/config?ssid=X&pass=Y&token=Z"
 */

#ifndef PlynxProvision_h
#define PlynxProvision_h

#if !defined(ESP32) && !defined(ESP8266)
#error PlynxProvision.h supports only ESP32 and ESP8266
#endif

#if defined(ESP32)
  #include <Preferences.h>
#else
  #include <EEPROM.h>
#endif

// blnkinf "cm" flag: connection manager compiled in. NOTE: what actually
// puts "cm" in the profile is the define being visible BEFORE
// PlynxApiArduino.h is read; the ESP shells take care of that. This one only
// covers exotic sketches that include this header directly and early.
#ifndef PLYNX_HAS_CONNECTION_MANAGER
#define PLYNX_HAS_CONNECTION_MANAGER
#endif

// Well-known setup PSK: obfuscation + iOS auto-join, not secrecy. The
// provisioning window is short and the token only travels TO the board.
#ifndef PLYNX_PROV_AP_PASS
#define PLYNX_PROV_AP_PASS    "plynxsetup"
#endif
#ifndef PLYNX_PROV_PORT
#define PLYNX_PROV_PORT       80
#endif
// Provisioning window; on timeout the board reboots (and re-enters AP if
// still unconfigured).
#ifndef PLYNX_PROV_TIMEOUT_MS
#define PLYNX_PROV_TIMEOUT_MS 180000UL
#endif
// Bounded WiFi connect for the auto path (a typo'd password must fall back
// to the AP, never hang forever like the classic path is allowed to).
#ifndef PLYNX_PROV_WIFI_TIMEOUT_MS
#define PLYNX_PROV_WIFI_TIMEOUT_MS 30000UL
#endif

#define PLYNX_PROV_MAGIC 0x504C5958UL  // "PLYX"

// Last provisioning error, reported by GET /status so the app can explain
// a retry without the user reflashing.
enum PlynxProvError : uint8_t {
    PLYNX_PROV_ERR_NONE    = 0,
    PLYNX_PROV_ERR_NETWORK = 1,   // saved WiFi unreachable / wrong password
    PLYNX_PROV_ERR_TOKEN   = 2,   // server rejected the auth token
};

#ifdef PLYNX_OTA_HAS_DEFAULT
// Built-in OTA pending-confirm watchdog (see Plynx/PlynxOta.h): must also
// tick while the token-less path waits for WiFi, or a fresh firmware that
// cannot rejoin the network would never roll back.
void PlynxOtaTick();
#endif

struct PlynxProvConfig {
    uint32_t magic;
    char     ssid[33];
    char     pass[64];
    char     token[34];
    char     host[34];
    uint16_t port;
    uint8_t  lastError;
} __attribute__((packed));

class PlynxProvisioner {
public:

    static bool load(PlynxProvConfig& cfg)
    {
#if defined(ESP32)
        Preferences prefs;
        prefs.begin("plynx", true);
        size_t n = prefs.getBytes("cfg", &cfg, sizeof(cfg));
        prefs.end();
        if (n != sizeof(cfg)) return false;
#else
        EEPROM.begin(sizeof(PlynxProvConfig));
        EEPROM.get(0, cfg);
        EEPROM.end();
#endif
        return (cfg.magic == PLYNX_PROV_MAGIC) && cfg.ssid[0] && cfg.token[0];
    }

    static void save(PlynxProvConfig& cfg)
    {
        cfg.magic = PLYNX_PROV_MAGIC;
#if defined(ESP32)
        Preferences prefs;
        prefs.begin("plynx", false);
        prefs.putBytes("cfg", &cfg, sizeof(cfg));
        prefs.end();
#else
        EEPROM.begin(sizeof(PlynxProvConfig));
        EEPROM.put(0, cfg);
        EEPROM.commit();
        EEPROM.end();
#endif
    }

    static void clear()
    {
#if defined(ESP32)
        Preferences prefs;
        prefs.begin("plynx", false);
        prefs.remove("cfg");
        prefs.end();
#else
        PlynxProvConfig cfg;
        memset(&cfg, 0, sizeof(cfg));
        EEPROM.begin(sizeof(PlynxProvConfig));
        EEPROM.put(0, cfg);
        EEPROM.commit();
        EEPROM.end();
#endif
    }

    /// Stable per-chip AP name: "Plynx-XXXX"
    static void apName(char* out, size_t len)
    {
#if defined(ESP32)
        uint64_t mac = ESP.getEfuseMac();
        uint16_t uniq = (uint16_t)(mac >> 32) ^ (uint16_t)(mac >> 16) ^ (uint16_t)mac;
#else
        uint32_t id = ESP.getChipId();
        uint16_t uniq = (uint16_t)(id >> 16) ^ (uint16_t)id;
#endif
        snprintf(out, len, "Plynx-%04X", uniq);
    }

    /// Bounded STA connect for the auto path. Classic begin() keeps its own
    /// legacy behavior; this one must fail cleanly back into the AP.
    static bool connectBounded(const char* ssid, const char* pass, uint32_t timeoutMs)
    {
        WiFi.mode(WIFI_STA);
        if (pass && strlen(pass)) {
            WiFi.begin(ssid, pass);
        } else {
            WiFi.begin(ssid);
        }
        uint32_t started = millis();
        while (WiFi.status() != WL_CONNECTED) {
            if (millis() - started > timeoutMs) {
                return false;
            }
#ifdef PLYNX_OTA_HAS_DEFAULT
            PlynxOtaTick();
#endif
            PlynxDelay(250);
        }
        return true;
    }

    /// Blocks serving the SoftAP config endpoint. Reboots on success or when
    /// the provisioning window expires. cfg.lastError is reported by /status.
    static void run(PlynxProvConfig& cfg)
    {
        char ssidBuff[16];
        apName(ssidBuff, sizeof(ssidBuff));

        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssidBuff, PLYNX_PROV_AP_PASS);
        PLYNX_LOG2(PLYNX_F("Provisioning AP: "), ssidBuff);

        WiFiServer server(PLYNX_PROV_PORT);
        server.begin();

        uint8_t pendingError = cfg.lastError;

        uint32_t started = millis();
        while (millis() - started < PLYNX_PROV_TIMEOUT_MS) {
#if defined(ESP32) && defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
            WiFiClient client = server.accept();
#else
            WiFiClient client = server.available();
#endif
            if (!client) { PlynxDelay(10); continue; }

            // Read the request line only:
            // GET /config?ssid=..&pass=..&token=..&host=..&port=.. HTTP/1.1
            char line[512];
            size_t n = 0;
            uint32_t t0 = millis();
            while (client.connected() && (millis() - t0 < 3000)) {
                if (!client.available()) { PlynxDelay(1); continue; }
                char c = client.read();
                if (c == '\n') break;
                if (c != '\r' && n < sizeof(line)-1) line[n++] = c;
            }
            line[n] = 0;

            if (strstr(line, "GET /status") == line) {
                client.print(F("HTTP/1.1 200 OK\r\nConnection: close\r\n"
                               "Content-Type: application/json\r\n\r\n"
                               "{\"status\":\""));
                client.print(pendingError ? F("error") : F("waiting"));
                if (pendingError) {
                    client.print(F("\",\"err\":\""));
                    client.print(pendingError == PLYNX_PROV_ERR_TOKEN
                                 ? F("token") : F("network"));
                }
                client.print(F("\",\"board\":\""));
                client.print(ssidBuff);
#ifdef PLYNX_PROJECT
                // Slice 3 pairing: lo sketch dichiara l'ID del progetto
                // pubblicato (catalogo Plynx) da montare al pairing. Campo
                // opzionale: le app vecchie lo ignorano (Decodable), senza
                // define non viene emesso alcun byte.
                client.print(F("\",\"project\":\"" PLYNX_PROJECT));
#endif
                client.print(F("\",\"fw\":\"" PLYNX_VERSION "\"}\n"));
                client.stop();
                continue;
            }

            char* q  = strchr(line, '?');
            char* sp = q ? strchr(q, ' ') : NULL;
            if (sp) *sp = 0;

            memset(&cfg, 0, sizeof(cfg));
            strncpy(cfg.host, PLYNX_DEFAULT_DOMAIN, sizeof(cfg.host)-1);
            cfg.port = PLYNX_DEFAULT_PORT;

            bool ok = false;
            if (q) {
                q++;
                ok = queryParam(q, "ssid",  cfg.ssid,  sizeof(cfg.ssid))
                   & queryParam(q, "token", cfg.token, sizeof(cfg.token));
                queryParam(q, "pass", cfg.pass, sizeof(cfg.pass));
                queryParam(q, "host", cfg.host, sizeof(cfg.host));
                char tmp[8];
                if (queryParam(q, "port", tmp, sizeof(tmp))) {
                    cfg.port = atoi(tmp);
                }
            }

            if (ok) {
                client.print(F("HTTP/1.1 200 OK\r\nConnection: close\r\n"
                               "Content-Type: application/json\r\n\r\n"
                               "{\"status\":\"ok\",\"board\":\""));
                client.print(ssidBuff);
                client.print(F("\"}\n"));
                client.flush();
                client.stop();
                save(cfg);
                PlynxDelay(300);
                ESP.restart();
            } else {
                client.print(F("HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n"));
                client.stop();
            }
        }
        // Window expired: reboot (re-enters AP if still unconfigured)
        ESP.restart();
    }

private:

    static void urlDecode(char* s)
    {
        char* w = s;
        while (*s) {
            if (*s == '%' && s[1] && s[2]) {
                char hex[3] = { s[1], s[2], 0 };
                *w++ = (char)strtol(hex, NULL, 16);
                s += 3;
            } else if (*s == '+') {
                *w++ = ' '; s++;
            } else {
                *w++ = *s++;
            }
        }
        *w = 0;
    }

    static bool queryParam(const char* q, const char* key, char* out, size_t outLen)
    {
        size_t klen = strlen(key);
        const char* p = q;
        while (p && *p) {
            if (!strncmp(p, key, klen) && p[klen] == '=') {
                const char* v = p + klen + 1;
                const char* e = strchr(v, '&');
                size_t vlen = e ? (size_t)(e - v) : strlen(v);
                if (vlen >= outLen) vlen = outLen - 1;
                memcpy(out, v, vlen);
                out[vlen] = 0;
                urlDecode(out);
                return out[0] != 0;
            }
            p = strchr(p, '&');
            if (p) p++;
        }
        return false;
    }
};

#endif
