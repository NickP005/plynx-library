/**
 * @file       PlynxOta.h
 * @license    This project is released under the MIT License (MIT)
 * @brief      Built-in OTA handler for ESP32 / ESP8266.
 *
 * The server pushes an internal frame "ota\0<url>[\0<sha256hex>\0<size>]".
 * Legacy servers send only the url: we proceed without hash verification
 * (and log it). Download is a plain HTTP GET streamed straight into the
 * flash updater, hashing incrementally chunk by chunk.
 *
 * ESP32:   Plynx.disconnect() first (Edgent-style), Update API, SHA256 via
 *          mbedtls. On success a "pending-confirm" flag is stored in NVS and
 *          the board reboots; if the first authenticated Plynx handshake
 *          does not happen within PLYNX_OTA_CONFIRM_MS the library rolls
 *          back to the previous app (Update.rollBack) and reboots again.
 * ESP8266: same flow with BearSSL SHA256 and the core Updater. NO automatic
 *          rollback: the 8266 Updater is single-slot (eboot copies the new
 *          image over the old one at boot), so once a broken-but-booting
 *          firmware is flashed it cannot auto-revert. The free-sketch-space
 *          check runs before anything is touched.
 *
 * Security model (read this before shipping a fleet):
 *   the sha256 in the frame protects DOWNLOAD INTEGRITY only, not
 *   authenticity -- url and hash travel together in the same frame, so
 *   whoever can forge the frame can also forge the hash. On a plain
 *   (security level 0) link the device does not authenticate the server:
 *   an on-path attacker able to spoof it could push its own url+hash.
 *   Two built-in mitigations raise the bar at near-zero cost:
 *     1. the frame is ignored until the session handshake has completed
 *        (Plynx.connected()), so nothing pre-auth is ever flashed;
 *     2. the download host is pinned to the server endpoint the sketch is
 *        configured against (exact, case-insensitive). Pin a different
 *        host with #define PLYNX_OTA_HOST "fw.example.com", or drop the
 *        check with #define PLYNX_OTA_NO_HOST_PIN.
 *   A full MITM on an sl=0 link can still defeat both. Fleets that care
 *   should use an authenticated link (sl>=1), define PLYNX_NO_OTA, or
 *   override PLYNX_WRITE(InternalPinOTA) with a handler that verifies a
 *   firmware signature.
 *
 * Crash-loop guard (ESP32): the confirm deadline is millis-based, so a new
 * firmware that panics and reboots before PLYNX_OTA_CONFIRM_MS would re-arm
 * the timer forever and never roll back. A boot counter in the same NVS
 * namespace closes the gap: after PLYNX_OTA_MAX_BOOT_TRIES boots armed with
 * pending-confirm and still unconfirmed, the library rolls back immediately.
 *
 * Opt-out: #define PLYNX_NO_OTA before including PlynxSimpleEsp32/Esp8266.h
 * (drops all of this code from the build). A sketch-level
 * PLYNX_WRITE(InternalPinOTA) override always wins over the built-in
 * handler: the dispatch checks at runtime whether the weak alias was
 * replaced by a strong user symbol, so the classic priority mechanism is
 * fully preserved.
 *
 * This header must be included AFTER the global `Plynx` object exists
 * (the ESP shells include it at the bottom).
 */

#ifndef PlynxOta_h
#define PlynxOta_h

#if !defined(ESP32) && !defined(ESP8266)
#error PlynxOta.h supports only ESP32 and ESP8266
#endif

#if defined(PLYNX_NO_OTA)
#error PlynxOta.h should not be included when PLYNX_NO_OTA is defined
#endif

#include <Plynx/PlynxHandlers.h>
#include <Plynx/PlynxParam.h>
#include <Plynx/PlynxDebug.h>

#if defined(ESP32)
    #include <WiFi.h>
    #include <Update.h>
    #include <Preferences.h>
    #include <mbedtls/version.h>
    #include <mbedtls/sha256.h>
#else
    #include <ESP8266WiFi.h>
    #include <Updater.h>
    #include <bearssl/bearssl_hash.h>
#endif

// Window after an OTA reboot for a successful authenticated Plynx handshake.
// Expired: rollback (ESP32) or clear-and-continue (ESP8266 has no rollback).
#ifndef PLYNX_OTA_CONFIRM_MS
#define PLYNX_OTA_CONFIRM_MS        60000UL
#endif

// Crash-loop guard (ESP32): boots spent armed with pending-confirm before
// forcing an immediate rollback. Boots 1..N-1 each get the full confirm
// window; boot N rolls back at the first tick.
#ifndef PLYNX_OTA_MAX_BOOT_TRIES
#define PLYNX_OTA_MAX_BOOT_TRIES    3
#endif

// Download chunk size (bytes) and per-chunk network stall timeout.
#ifndef PLYNX_OTA_CHUNK
#define PLYNX_OTA_CHUNK             512
#endif
#ifndef PLYNX_OTA_STREAM_TIMEOUT_MS
#define PLYNX_OTA_STREAM_TIMEOUT_MS 10000UL
#endif

/*
 * ---- pending-confirm state (ESP32 only: 8266 cannot roll back) ----
 */

#if defined(ESP32)

struct PlynxOtaState {
    bool          checked;    // NVS flag already read this boot
    bool          armed;      // pending-confirm active, waiting for handshake
    bool          confirmed;  // an authenticated handshake happened
    millis_time_t deadline;   // when to give up and roll back
};

static PlynxOtaState& plynxOtaState()
{
    static PlynxOtaState s = { false, false, false, 0 };
    return s;
}

static bool plynxOtaFlagRead()
{
    Preferences p;
    if (!p.begin("plynxota", true)) {
        return false;
    }
    const bool v = (p.getUChar("pend", 0) == 1);
    p.end();
    return v;
}

static void plynxOtaFlagWrite(bool on)
{
    Preferences p;
    if (!p.begin("plynxota", false)) {
        return;
    }
    if (on) {
        p.putUChar("pend", 1);
    } else {
        p.remove("pend");
    }
    // Either way the boot counter restarts: a fresh OTA begins at zero and
    // a cleared flag needs no counter.
    p.remove("tries");
    p.end();
}

// Crash-loop guard: increments the pending-confirm boot counter and returns
// the new value (1 on the first boot after flashing).
static uint8_t plynxOtaTriesBump()
{
    Preferences p;
    if (!p.begin("plynxota", false)) {
        return 1; // NVS unavailable: behave like a first try
    }
    uint8_t t = p.getUChar("tries", 0);
    if (t < 0xFF) {
        t++;
    }
    p.putUChar("tries", t);
    p.end();
    return t;
}

#endif // ESP32

/*
 * Called from PlynxProtocol::run() (and from the WiFi connect loop, so the
 * watchdog also fires when the new firmware cannot even join WiFi).
 */
void PlynxOtaTick()
{
#if defined(ESP32)
    PlynxOtaState& s = plynxOtaState();
    if (!s.checked) {
        s.checked = true;
        if (!s.confirmed && plynxOtaFlagRead()) {
            // A firmware that crashes and reboots before the deadline would
            // re-arm the millis-based timer on every boot and never roll
            // back: count the boots and give up after the Nth.
            if (plynxOtaTriesBump() >= PLYNX_OTA_MAX_BOOT_TRIES) {
                plynxOtaFlagWrite(false);
                if (Update.canRollBack()) {
                    PLYNX_LOG1(PLYNX_F("OTA: boot loop detected, rolling back"));
                    Update.rollBack();
                    ESP.restart();
                }
                PLYNX_LOG1(PLYNX_F("OTA: boot loop detected, no rollback available"));
                return;
            }
            s.armed    = true;
            s.deadline = PlynxMillis() + PLYNX_OTA_CONFIRM_MS;
            PLYNX_LOG1(PLYNX_F("OTA: new firmware, waiting for confirm handshake"));
        }
    }
    if (s.armed && !s.confirmed &&
        (int32_t)(PlynxMillis() - s.deadline) >= 0)
    {
        s.armed = false;
        // Clear BEFORE rolling back: the previous firmware must boot with a
        // clean flag, or the two images would ping-pong forever.
        plynxOtaFlagWrite(false);
        if (Update.canRollBack()) {
            PLYNX_LOG1(PLYNX_F("OTA: confirm timeout, rolling back"));
            Update.rollBack();
            ESP.restart();
        }
        PLYNX_LOG1(PLYNX_F("OTA: confirm timeout, no rollback available"));
    }
#endif
}

/*
 * Called by the protocol right after an authenticated handshake succeeds:
 * the new firmware works, keep it.
 */
void PlynxOtaAuthOk()
{
#if defined(ESP32)
    PlynxOtaState& s = plynxOtaState();
    s.confirmed = true;
    if (!s.checked) {
        s.checked = true;
        s.armed   = plynxOtaFlagRead();
    }
    if (s.armed) {
        s.armed = false;
        plynxOtaFlagWrite(false);
        PLYNX_LOG1(PLYNX_F("OTA: update confirmed"));
    }
#endif
}

/*
 * ---- helpers ----
 */

static char plynxOtaLower(char c)
{
    return (c >= 'A' && c <= 'Z') ? (char)(c + 32) : c;
}

// "http://host[:port]/path" -> host copied out, path points into url.
// https is NOT supported by the built-in handler (keeps the flash diet;
// override PLYNX_WRITE(InternalPinOTA) if you need it).
static bool plynxOtaParseUrl(const char* url, char* host, size_t hostLen,
                             uint16_t& port, const char*& path)
{
    if (strncmp(url, "http://", 7) != 0) {
        return false;
    }
    const char* h     = url + 7;
    const char* slash = strchr(h, '/');
    const char* colon = strchr(h, ':');
    if (colon && slash && colon > slash) {
        colon = NULL; // ':' belongs to the path
    }
    const size_t hl = colon ? (size_t)(colon - h)
                    : slash ? (size_t)(slash - h)
                    : strlen(h);
    if (hl == 0 || hl >= hostLen) {
        return false;
    }
    memcpy(host, h, hl);
    host[hl] = '\0';
    port = colon ? (uint16_t)atoi(colon + 1) : 80;
    path = slash ? slash : "/";
    return (port != 0);
}

#if !defined(PLYNX_OTA_NO_HOST_PIN)

// Hostnames are case-insensitive: exact match, no wildcards.
static bool plynxOtaHostEq(const char* a, const char* b)
{
    while (*a && *b) {
        if (plynxOtaLower(*a) != plynxOtaLower(*b)) {
            return false;
        }
        a++; b++;
    }
    return *a == '\0' && *b == '\0';
}

// The built-in handler only downloads from the pinned host: PLYNX_OTA_HOST
// when defined, otherwise the server endpoint the sketch is configured
// against (domain, or the literal IP when config() got an IPAddress). The
// url+sha256 arrive in one unauthenticated frame, so the sha256 proves
// integrity, not authenticity: the pin keeps a forged frame from pointing
// the download at an attacker's server. See the security model on top.
static bool plynxOtaHostAllowed(const char* host)
{
#if defined(PLYNX_OTA_HOST)
    return plynxOtaHostEq(host, PLYNX_OTA_HOST);
#else
    const char* dom = Plynx.transportConn().getDomain();
    if (dom && dom[0]) {
        return plynxOtaHostEq(host, dom);
    }
    IPAddress ip;
    return ip.fromString(host) && ip == Plynx.transportConn().getIP();
#endif
}

#endif // !PLYNX_OTA_NO_HOST_PIN

// Reads one CRLF-terminated line (without CRLF). Returns length or -1.
static int plynxOtaReadLine(WiFiClient& client, char* buf, size_t len)
{
    size_t n = 0;
    const millis_time_t deadline = PlynxMillis() + PLYNX_OTA_STREAM_TIMEOUT_MS;
    while ((int32_t)(PlynxMillis() - deadline) < 0) {
        if (!client.available()) {
            if (!client.connected()) {
                return -1;
            }
            PlynxDelay(1);
            continue;
        }
        const int c = client.read();
        if (c < 0) {
            continue;
        }
        if (c == '\n') {
            buf[n] = '\0';
            return (int)n;
        }
        if (c != '\r' && n < len - 1) {
            buf[n++] = (char)c;
        }
    }
    return -1;
}

// Case-insensitive "does line start with name" (name must be lowercase).
static bool plynxOtaHeaderIs(const char* line, const char* name)
{
    while (*name) {
        if (plynxOtaLower(*line) != *name) {
            return false;
        }
        line++; name++;
    }
    return true;
}

static bool plynxOtaShaMatches(const uint8_t digest[32], const char* hex)
{
    static const char lut[] = "0123456789abcdef";
    for (int i = 0; i < 32; i++) {
        if (lut[digest[i] >> 4]  != plynxOtaLower(hex[2*i]) ||
            lut[digest[i] & 0xF] != plynxOtaLower(hex[2*i + 1]))
        {
            return false;
        }
    }
    return true;
}

/*
 * ---- incremental SHA256 (mbedtls on ESP32, BearSSL on ESP8266) ----
 */

struct PlynxOtaSha256 {
#if defined(ESP32)
    mbedtls_sha256_context ctx;
    void init() {
        mbedtls_sha256_init(&ctx);
#if defined(MBEDTLS_VERSION_MAJOR) && (MBEDTLS_VERSION_MAJOR >= 3)
        mbedtls_sha256_starts(&ctx, 0);
#else
        mbedtls_sha256_starts_ret(&ctx, 0);
#endif
    }
    void update(const uint8_t* d, size_t n) {
#if defined(MBEDTLS_VERSION_MAJOR) && (MBEDTLS_VERSION_MAJOR >= 3)
        mbedtls_sha256_update(&ctx, d, n);
#else
        mbedtls_sha256_update_ret(&ctx, d, n);
#endif
    }
    void final(uint8_t out[32]) {
#if defined(MBEDTLS_VERSION_MAJOR) && (MBEDTLS_VERSION_MAJOR >= 3)
        mbedtls_sha256_finish(&ctx, out);
#else
        mbedtls_sha256_finish_ret(&ctx, out);
#endif
        mbedtls_sha256_free(&ctx);
    }
#else
    br_sha256_context ctx;
    void init()                            { br_sha256_init(&ctx); }
    void update(const uint8_t* d, size_t n){ br_sha256_update(&ctx, d, n); }
    void final(uint8_t out[32])            { br_sha256_out(&ctx, out); }
#endif
};

/*
 * ---- flash updater wrappers ----
 */

static bool plynxOtaFlashBegin(uint32_t size)
{
#if defined(ESP32)
    return Update.begin(size ? size : UPDATE_SIZE_UNKNOWN);
#else
    // Spec: verify free sketch space BEFORE starting.
    const uint32_t space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (size > space) {
        return false;
    }
    return Update.begin(size ? size : space, U_FLASH);
#endif
}

static void plynxOtaFlashAbort()
{
#if defined(ESP32)
    Update.abort();
#else
    // The 8266 core has no abort(): a premature end() flags the error and
    // resets the updater state, which is all we need.
    Update.end();
#endif
}

/*
 * ---- the download itself ----
 */

// Failure after the cloud link was dropped: clean up and arm a reconnect.
// We may be nested inside processInput() here (msgIdOutOverride is set, so
// a blocking Plynx.connect() would spin on the nesting guard): startSession()
// re-opens the socket and lets the sketch's normal Plynx.run() loop redo the
// login instead.
static void plynxOtaFail(WiFiClient& client, const char* msg, bool abortFlash)
{
    PLYNX_LOG2(PLYNX_F("OTA: failed: "), msg);
    if (abortFlash) {
        plynxOtaFlashAbort();
    }
    client.stop();
    Plynx.startSession();
}

static void plynxOtaPerform(const char* url, const char* shaHex, uint32_t declaredSize)
{
    char host[64];
    uint16_t port;
    const char* path;
    if (!plynxOtaParseUrl(url, host, sizeof(host), port, path)) {
        // Session untouched: just refuse.
        PLYNX_LOG2(PLYNX_F("OTA: bad/unsupported url: "), url);
        return;
    }

#if !defined(PLYNX_OTA_NO_HOST_PIN)
    if (!plynxOtaHostAllowed(host)) {
        // Session untouched here too. If your firmware lives on another
        // host, pin it with PLYNX_OTA_HOST (see the header comment).
        PLYNX_LOG2(PLYNX_F("OTA: host not pinned, refusing: "), host);
        return;
    }
#endif

#if defined(ESP8266)
    if (declaredSize && ESP.getFreeSketchSpace() < declaredSize) {
        // Fail fast, before dropping the cloud link.
        PLYNX_LOG1(PLYNX_F("OTA: image larger than free sketch space"));
        return;
    }
#endif

    PLYNX_LOG2(PLYNX_F("OTA: updating from "), url);
    if (!shaHex) {
        PLYNX_LOG1(PLYNX_F("OTA: legacy frame without sha256, skipping verification"));
    }

    // Edgent-style: drop the cloud connection before the download.
    Plynx.disconnect();

    WiFiClient client;
    if (!client.connect(host, port)) {
        plynxOtaFail(client, "connect", false);
        return;
    }

    client.print("GET ");
    client.print(path);
    // HTTP/1.0: no chunked transfer-encoding, server closes after the body.
    client.print(" HTTP/1.0\r\nHost: ");
    client.print(host);
    client.print("\r\nConnection: close\r\n\r\n");

    char line[160];
    if (plynxOtaReadLine(client, line, sizeof(line)) < 0) {
        plynxOtaFail(client, "no response", false);
        return;
    }
    const char* sp = strchr(line, ' ');
    if (!sp || atoi(sp + 1) != 200) {
        plynxOtaFail(client, "http status", false);
        return;
    }

    uint32_t contentLength = 0;
    for (;;) {
        const int n = plynxOtaReadLine(client, line, sizeof(line));
        if (n < 0) {
            plynxOtaFail(client, "headers", false);
            return;
        }
        if (n == 0) {
            break; // end of headers
        }
        if (plynxOtaHeaderIs(line, "content-length:")) {
            const char* v = line + 15;
            while (*v == ' ') v++;
            contentLength = (uint32_t)atol(v);
        }
    }

    if (declaredSize && contentLength && declaredSize != contentLength) {
        plynxOtaFail(client, "size mismatch", false);
        return;
    }
    const uint32_t expected = declaredSize ? declaredSize : contentLength;

    if (!plynxOtaFlashBegin(expected)) {
        plynxOtaFail(client, "flash begin (space?)", false);
        return;
    }

    PlynxOtaSha256 sha;
    sha.init();

    uint8_t  buf[PLYNX_OTA_CHUNK];
    uint32_t written  = 0;
    millis_time_t deadline = PlynxMillis() + PLYNX_OTA_STREAM_TIMEOUT_MS;

    while (expected == 0 || written < expected) {
        size_t avail = client.available();
        if (!avail) {
            if (!client.connected()) {
                break; // server closed: end of body
            }
            if ((int32_t)(PlynxMillis() - deadline) >= 0) {
                plynxOtaFail(client, "stream timeout", true);
                return;
            }
            PlynxDelay(1);
            continue;
        }
        size_t toRead = avail;
        if (toRead > sizeof(buf)) {
            toRead = sizeof(buf);
        }
        if (expected && toRead > expected - written) {
            toRead = expected - written;
        }
        const int n = client.read(buf, toRead);
        if (n <= 0) {
            continue;
        }
        sha.update(buf, (size_t)n);
        if (Update.write(buf, (size_t)n) != (size_t)n) {
            plynxOtaFail(client, "flash write", true);
            return;
        }
        written += (uint32_t)n;
        deadline = PlynxMillis() + PLYNX_OTA_STREAM_TIMEOUT_MS;
        PlynxDelay(0); // yield / feed WDT during the long download
    }
    client.stop();

    if ((expected && written != expected) || written == 0) {
        plynxOtaFail(client, "truncated download", true);
        return;
    }

    uint8_t digest[32];
    sha.final(digest);
    if (shaHex && !plynxOtaShaMatches(digest, shaHex)) {
        plynxOtaFail(client, "sha256 mismatch", true);
        return;
    }

    if (!Update.end(expected == 0)) {
        plynxOtaFail(client, "flash end", false);
        return;
    }

#if defined(ESP32)
    // Arm pending-confirm: the next boot must complete an authenticated
    // handshake within PLYNX_OTA_CONFIRM_MS or we roll back.
    plynxOtaFlagWrite(true);
#endif
    PLYNX_LOG1(PLYNX_F("OTA: success, rebooting"));
    PlynxDelay(100);
    ESP.restart();
}

/*
 * Dispatch entry, wired into PlynxProtocol's PLYNX_INT_OTA case.
 */
void PlynxOtaDispatch(PlynxReq& req, const PlynxParam& param)
{
    // A sketch-level PLYNX_WRITE(InternalPinOTA) is a strong symbol that
    // replaces the weak alias (which points at PlynxWidgetWrite): if the two
    // addresses differ, the user override exists and takes priority. The
    // volatile reads stop the compiler from constant-folding a comparison of
    // two "distinct" functions.
    WidgetWriteHandler volatile userH = &PlynxWidgetWriteInternalPinOTA;
    WidgetWriteHandler volatile defH  = &PlynxWidgetWrite;
    if (userH != defH) {
        ((WidgetWriteHandler)userH)(req, param);
        return;
    }
    (void)req;

    // PLYNX_CMD_INTERNAL frames are processed even while the session is
    // still CONNECTING: never flash anything before the handshake completed.
    if (!Plynx.connected()) {
        PLYNX_LOG1(PLYNX_F("OTA: ignored, session not authenticated"));
        return;
    }

    // param layout (NUL-separated, via iterators -- never through a single
    // C-string): [0]=url, [1]=sha256 hex (optional), [2]=size (optional).
    PlynxParam::iterator it = param.begin();
    if (it >= param.end() || it.isEmpty()) {
        PLYNX_LOG1(PLYNX_F("OTA: missing url"));
        return;
    }
    const char* url    = it.asStr();
    const char* shaHex = NULL;
    uint32_t    size   = 0;
    if (++it < param.end() && !it.isEmpty()) {
        const char* s = it.asStr();
        if (strlen(s) == 64) {
            shaHex = s;
        } else {
            PLYNX_LOG1(PLYNX_F("OTA: malformed sha256, ignoring it"));
        }
        if (++it < param.end() && !it.isEmpty()) {
            const long sz = it.asLong();
            if (sz > 0) {
                size = (uint32_t)sz;
            }
        }
    }
    plynxOtaPerform(url, shaHex, size);
}

#endif // PlynxOta_h
