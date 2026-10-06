/**
 * @file       PlynxOta.h
 * @license    This project is released under the MIT License (MIT)
 * @brief      Built-in OTA handler for ESP32 / ESP8266.
 *
 * TWO TRANSPORTS, ONE SET OF RULES. Which one is compiled is decided by the
 * shell, and only one of them is ever in a binary:
 *
 *   default (WiFi) -- the server pushes an internal frame
 *      "ota\0<url>[\0<sha256hex>\0<size>]" and the board FETCHES the image
 *      itself: a plain HTTP GET streamed straight into the flash updater,
 *      hashing incrementally chunk by chunk. Legacy servers send only the url:
 *      we proceed without hash verification (and log it).
 *
 *   PLYNX_OTA_TRANSPORT_PUSH (Bluetooth) -- the board cannot fetch anything,
 *      so whoever is connected hands it the bytes instead, chunk by chunk with
 *      an explicit offset. See the block above PlynxOtaPush further down. The
 *      HTTP client, the url parser and the host pin are not compiled at all.
 *
 * Everything else is deliberately common: the Update wrappers, the sha256, and
 * above all the rollback machinery. A second transport must never mean a
 * second set of safety rules.
 *
 * ESP32:   Plynx.disconnect() first (Edgent-style), Update API, SHA256 via
 *          mbedtls. The new image boots on probation: the bootloader itself
 *          undoes it unless the first authenticated Plynx handshake happens
 *          within PLYNX_OTA_CONFIRM_MS (see the block on verifyRollbackLater
 *          below). On cores too old for that hook a millis-based watchdog
 *          plus an NVS flag do the same job from software.
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
 *        host with `#define PLYNX_OTA_HOST "fw.example.com"`, or drop the
 *        check with `#define PLYNX_OTA_NO_HOST_PIN`.
 *   A full MITM on an sl=0 link can still defeat both. Fleets that care
 *   should use an authenticated link (sl>=1), define PLYNX_NO_OTA, or
 *   override PLYNX_WRITE(InternalPinOTA) with a handler that verifies a
 *   firmware signature.
 *
 *   On PLYNX_OTA_TRANSPORT_PUSH mitigation 2 does not exist and cannot: there
 *   is no host to pin, because there is no download. What stands in its place
 *   is the link itself -- the bytes can only come from whoever is already
 *   connected and authenticated, and on Bluetooth that means someone within a
 *   few metres who also holds the token. The sha256 still proves only that the
 *   image arrived intact. A signature over the image is the answer for a fleet
 *   that cares, and it is NOT implemented yet.
 *
 * Crash-loop guard: a millis-based deadline cannot see a firmware that panics
 * and reboots before it expires -- every boot would re-arm the timer. The
 * bootloader path has no such gap (one reset is enough for it to give up).
 * The software fallback closes it with a boot counter in the same NVS
 * namespace: after PLYNX_OTA_MAX_BOOT_TRIES boots armed with pending-confirm
 * and still unconfirmed, the library rolls back immediately.
 *
 * Opt-out: `#define PLYNX_NO_OTA` before including PlynxSimpleEsp32/Esp8266.h
 * (drops all of this code from the build), or
 * `#define PLYNX_NO_BOOTLOADER_ROLLBACK` to keep OTA but leave the bootloader hook
 * alone -- REQUIRED alongside ESP RainMaker, which defines the same symbol.
 * A sketch-level
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

// PlynxProtocol.h decides its OTA branch at its FIRST inclusion in the
// translation unit. If something pulled it in before PLYNX_OTA_HAS_DEFAULT was
// defined, this file would compile a download handler nobody dispatches to and
// an update path nobody reaches -- with a perfectly green build. Refuse.
#if !defined(PLYNX_OTA_PROTOCOL_WIRED)
#error Plynx/PlynxProtocol.h was included before PLYNX_OTA_HAS_DEFAULT was \
defined, so the OTA dispatch is missing from this build. Include \
PlynxSimpleEsp32.h / PlynxSimpleEsp8266.h FIRST, or define PLYNX_NO_OTA.
#endif

#include <Plynx/PlynxHandlers.h>
#include <Plynx/PlynxParam.h>
#include <Plynx/PlynxDebug.h>

#if defined(ESP32)
    #include <WiFi.h>
    #include <Update.h>
    #include <Preferences.h>
    #include <esp_ota_ops.h>
    #include <freertos/FreeRTOS.h>
    #include <freertos/task.h>
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

// Crash-loop guard for the SOFTWARE fallback only (see above): boots spent
// armed with pending-confirm before forcing an immediate rollback. Boots
// 1..N-1 each get the full confirm window; boot N rolls back at the first tick.
#ifndef PLYNX_OTA_MAX_BOOT_TRIES
#define PLYNX_OTA_MAX_BOOT_TRIES    3
#endif

// Window the bootloader guard gives the new firmware to open its first
// authenticated session. It is measured from initArduino(), so unlike
// PLYNX_OTA_CONFIRM_MS above it INCLUDES setup(): a sketch that calibrates a
// sensor or waits for a GPS fix before Plynx.begin() must raise it. The
// default is deliberately generous, because the two mistakes are not
// symmetric -- a window too long only postpones the rollback to the next
// reset (an unconfirmed image never survives one), while a window too short
// throws away a firmware that was working.
//
// It is an absolute number and NOT derived from PLYNX_OTA_CONFIRM_MS: the two
// have different starting points, so a sketch that had tuned the old one down
// would silently inherit a much stricter deadline that now also has to cover
// setup(), WiFi association and DHCP.
#ifndef PLYNX_OTA_GUARD_MS
/**
 * @brief How long, in milliseconds, a firmware just received over the air has
 * to connect to the server before the ESP32 goes back to the previous one.
 * The default is 180000, three minutes.
 *
 * The time counts from the start of the board, setup() included. Any reset
 * before the first connection also brings the old firmware back. Raise it for
 * a sketch that spends long in setup() before Plynx.begin(), or a board that
 * stays offline for long.
 *
 * Write it above `<PlynxSimpleEsp32.h>`.
 *
 * @code
 * #define PLYNX_OTA_GUARD_MS 600000UL   // ten minutes
 * #include <PlynxSimpleEsp32.h>
 * @endcode
 *
 * @see PLYNX_NO_BOOTLOADER_ROLLBACK, PLYNX_NO_OTA
 */
#define PLYNX_OTA_GUARD_MS          180000UL
#endif

// Bootloader-rollback guard task: stack, polling interval and priority.
// The stack must fit a Preferences write (it clears the NVS flag on confirm);
// 4096 is the same budget the IDF gives its own main task.
#ifndef PLYNX_OTA_GUARD_STACK
#define PLYNX_OTA_GUARD_STACK       4096
#endif
#ifndef PLYNX_OTA_GUARD_POLL_MS
#define PLYNX_OTA_GUARD_POLL_MS     250
#endif
#ifndef PLYNX_OTA_GUARD_PRIO
#define PLYNX_OTA_GUARD_PRIO        1
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
    // Written by the protocol on the sketch's task, read by the guard task
    // below: volatile so the poll loop cannot hoist it into a register.
    volatile bool confirmed;  // an authenticated handshake happened
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

/*
 * ---- bootloader rollback: arming and confirming, in the same function ----
 *
 * The ESP32 bootloader can undo an OTA on its own. An image written through
 * esp_ota_set_boot_partition() starts as ESP_OTA_IMG_PENDING_VERIFY and,
 * unless somebody confirms it, the NEXT reset makes the bootloader mark it
 * ABORTED and boot the previous app again. That covers the one case no
 * software watchdog can: a firmware that panics BEFORE our code ever runs.
 *
 * The core throws that net away inside initArduino(), before setup()
 * (esp32-hal-misc.c:314-329). verifyRollbackLater() is the weak hook that
 * tells it not to. We never call it: providing a strong definition wins at
 * LINK time. It lives in a .c file and is declared in no header of the core,
 * so the definition MUST be extern "C" -- from C++ without it the name is
 * mangled, the build succeeds, and the override silently does nothing.
 *
 * The one rule this code exists to respect: NEVER postpone the confirm unless
 * the code that will confirm is in this build and already running. That is
 * why the guard task is started by verifyRollbackLater() itself, two lines
 * above `return true`, and why failing to start it means not arming. No
 * #ifdef, no template and no second translation unit sits between the two
 * halves -- they are the same function. An earlier design confirmed from
 * PlynxProtocol.h instead, and could be split in two by nothing more than the
 * order of two #includes, or by the alphabetical order of two file names.
 *
 * Opt out with #define PLYNX_NO_BOOTLOADER_ROLLBACK. It is REQUIRED when
 * another library already defines the hook (ESP RainMaker does): without it
 * the link fails with "multiple definition of verifyRollbackLater", which is
 * the correct, loud outcome.
 */

// Set by verifyRollbackLater() below, which the core calls from initArduino()
// before setup(). PlynxOtaTick() reads them to decide whether the software
// watchdog is needed at all -- see plynxOtaSoftwareWatchdogNeeded().
// Volatile: plynxOtaGuardArmed is cleared by the guard task itself and read by
// PlynxOtaTick() on the sketch's task.
static volatile bool plynxOtaCoreHookRan = false; // the core has the hook, and called it
static volatile bool plynxOtaGuardArmed  = false; // ...and the guard task is running
static volatile bool plynxOtaGuardFailed = false; // ...and it could NOT be started

// Confirms the running image to the bootloader. Safe to call more than once and
// on an image that is not on probation: the IDF turns it into a no-op then.
static bool plynxOtaConfirmToBootloader()
{
    return esp_ota_mark_app_valid_cancel_rollback() == ESP_OK;
}

// The core's OTHER weak hook (esp32-hal-misc.c:261-265): the place a sketch is
// meant to put its own self-test before an image is kept. initArduino() asks it
// just before confirming -- and postponing the confirm skipped it for good, so
// a sketch that defined it got silence. Ask it here instead, at the moment we
// would confirm. Declared weak because it is in no header and older cores do
// not have it at all: then the address is null and there is nothing to ask.
extern "C" bool verifyOta() __attribute__((weak));

static bool plynxOtaSketchAgrees()
{
    return (&verifyOta == NULL) || verifyOta();
}

#if !defined(PLYNX_NO_BOOTLOADER_ROLLBACK)

static void plynxOtaGuardTask(void*)
{
    const millis_time_t deadline = PlynxMillis() + PLYNX_OTA_GUARD_MS;
    uint8_t confirmTries = 0;
    bool    giveUpNow    = false;
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(PLYNX_OTA_GUARD_POLL_MS));

        // The only proof that counts: a session that completed the handshake.
        // connected() is plain protocol state (PlynxProtocol.h), readable
        // without any OTA macro being defined -- that is the point.
        PlynxOtaState& s = plynxOtaState();
        if (!giveUpNow && (Plynx.connected() || s.confirmed)) {
            if (!plynxOtaSketchAgrees()) {
                // The sketch's own verifyOta() refused the image. The core
                // would have rolled back on the spot; do the same, do not
                // sit out the rest of the window.
                PLYNX_LOG1(PLYNX_F("OTA: verifyOta() refused the image"));
                giveUpNow = true;
                continue;
            }
            s.confirmed = true;
            s.armed     = false;
            // PlynxOtaAuthOk() normally got here first and this is a no-op. It
            // is repeated because that path only exists when the protocol in
            // this build carries the OTA hooks, and this one always does.
            if (plynxOtaConfirmToBootloader()) {
                plynxOtaFlagWrite(false); // the software fallback must not re-arm
                PLYNX_LOG1(PLYNX_F("OTA: image confirmed to the bootloader"));
                break;
            }
            // An image that failed to confirm is still on probation and would
            // be thrown away at the next reset after having proved it works:
            // keep trying rather than leave. Bounded, so a state that can never
            // be confirmed does not leave a task polling for ever.
            if (++confirmTries < 20) {
                continue;
            }
            PLYNX_LOG1(PLYNX_F("OTA: could not confirm the image to the bootloader"));
            break;
        }

        if (giveUpNow || (int32_t)(PlynxMillis() - deadline) >= 0) {
            // Clear the flag BEFORE the reset. The image we are going back to
            // may be an older build of this library, without the staleness
            // check below -- and it would read the flag, arm its own watchdog
            // and jump FORWARD onto the image the bootloader just condemned.
            plynxOtaFlagWrite(false);
            if (esp_ota_check_rollback_is_possible() == ESP_OK) {
                PLYNX_LOG1(PLYNX_F("OTA: no authenticated session, rolling back"));
                PlynxDelay(50); // let the line leave the UART before the reset
                // Marks this image invalid and restarts; the bootloader starts
                // the previous app. Unlike Update.rollBack() this honours
                // ABORTED, so it cannot jump onto an image already condemned.
                esp_ota_mark_app_invalid_rollback_and_reboot();
                // It returns only on failure -- and then nothing was written,
                // so falling through to the confirm below is still correct.
            }
            // There is no other bootable app. Leaving this one on probation
            // would make the bootloader abort it at the next reset with
            // nothing to start instead, which is a board that boots nothing.
            PLYNX_LOG1(PLYNX_F("OTA: nothing to roll back to, keeping this image"));
            plynxOtaConfirmToBootloader();
            break;
        }
    }
    plynxOtaGuardArmed = false;
    vTaskDelete(NULL);
}

extern "C" bool verifyRollbackLater()
{
    plynxOtaCoreHookRan = true;
    (void)plynxOtaState(); // build the state here, in the main task, not later

    // NVS is not initialised yet at this point (initArduino() does that a few
    // lines further down), so nothing here may read the pending-confirm flag.
    // It is not needed: the partition state is the authoritative answer.
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t   state;
    if (!running || esp_ota_get_state_partition(running, &state) != ESP_OK ||
        state != ESP_OTA_IMG_PENDING_VERIFY)
    {
        // Not on probation: USB-flashed image, single-app partition scheme, or
        // an image confirmed long ago. Nothing to postpone.
        return false;
    }

    if (xTaskCreate(plynxOtaGuardTask, "plynxota", PLYNX_OTA_GUARD_STACK,
                    NULL, PLYNX_OTA_GUARD_PRIO, NULL) != pdPASS)
    {
        // No guard, no arming: better an image confirmed too easily than one
        // that nothing in this firmware can ever confirm. Remember it, or the
        // boot would end with neither net -- the core confirms the image, and
        // the software watchdog would then read the partition as VALID and
        // decide the pending-confirm flag was stale.
        plynxOtaGuardFailed = true;
        return false;
    }
    plynxOtaGuardArmed = true;
    return true;
}

#endif // !PLYNX_NO_BOOTLOADER_ROLLBACK

// Should the software watchdog arm for the flag just read from NVS? Only
// observable facts decide, because the flag alone cannot tell "this boot" from
// "some previous boot":
//
//  - the hook never ran -> either a core too old to have it, or the opt-out.
//    The image was confirmed at boot and NVS is the only record that an OTA
//    happened: arm, exactly as this library always did.
//  - the hook ran and armed the guard -> handled earlier in PlynxOtaTick(),
//    we never reach here.
//  - the hook ran and did NOT arm -> the running image is not on probation.
//    Still ESP_OTA_IMG_NEW means the bootloader cannot put images on probation
//    (mismatched bootloader), so software is the only net: arm. Anything else
//    (VALID / UNDEFINED / ABORTED) means the flag is left over: typically the
//    bootloader has ALREADY rolled us back and this is the old, healthy
//    firmware reading a flag about the image that was just thrown away.
//    Arming there would undo the rescue -- Update.rollBack() only checks the
//    0xE9 magic byte, so it would jump FORWARD onto the condemned image.
static bool plynxOtaSoftwareWatchdogNeeded()
{
    if (!plynxOtaCoreHookRan || plynxOtaGuardFailed) {
        return true;
    }
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_ota_img_states_t   state;
    if (!running || esp_ota_get_state_partition(running, &state) != ESP_OK) {
        return false;
    }
    return state == ESP_OTA_IMG_NEW;
}

#endif // ESP32

/*
 * Called from PlynxProtocol::run() (and from the WiFi connect loop, so the
 * watchdog also fires when the new firmware cannot even join WiFi).
 *
 * Nothing safety-critical depends on this being called: when the bootloader
 * guard is armed it owns the boot on its own task and this returns at once.
 */
void PlynxOtaTick()
{
#if defined(ESP32)
    if (plynxOtaGuardArmed) {
        // The guard confirms on the first authenticated session and rolls back
        // on its own deadline. Two rollback mechanisms racing on one boot would
        // be worse than either alone.
        return;
    }
    PlynxOtaState& s = plynxOtaState();
    if (!s.checked) {
        s.checked = true;
        if (!s.confirmed && plynxOtaFlagRead()) {
            if (!plynxOtaSoftwareWatchdogNeeded()) {
                // Stale flag: clear it and, above all, never roll back on it.
                plynxOtaFlagWrite(false);
                return;
            }
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
 *
 * The guard task reaches the same conclusion on its own from
 * Plynx.connected(), so a build where these calls are missing is slower to
 * confirm, not unsafe. But this path stays SYNCHRONOUS with the handshake:
 * doing the confirm here and not up to a poll interval later is what makes it
 * safe for a sketch that resets or deep-sleeps as soon as it is connected --
 * the classic battery sensor, which would otherwise throw away an image that
 * had just proved it works.
 */
void PlynxOtaAuthOk()
{
#if defined(ESP32)
    PlynxOtaState& s = plynxOtaState();
    s.confirmed = true;
    if (plynxOtaGuardArmed) {
        if (!plynxOtaSketchAgrees()) {
            return; // the sketch's verifyOta() said no: let the guard roll back
        }
        if (plynxOtaConfirmToBootloader()) {
            plynxOtaFlagWrite(false);
            PLYNX_LOG1(PLYNX_F("OTA: image confirmed to the bootloader"));
        }
        // Either way the guard task also checks, and will retry if this failed.
        return;
    }
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

#if !defined(PLYNX_OTA_TRANSPORT_PUSH)

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

#endif // !PLYNX_OTA_TRANSPORT_PUSH

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
    // Same number the board reports as "ota-max" in blnkinf, so the server's
    // verdict and the board's verdict can never disagree by a page: keep the
    // macro as the single source (Plynx/PlynxFlashInfo.h).
#if defined(PLYNX_INFO_OTA_MAX)
    const uint32_t space = (uint32_t)(PLYNX_INFO_OTA_MAX);
#else
    const uint32_t space = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
#endif
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
 * ---- the chip the image was built for ----
 *
 * An ESP32-family image says in its header which chip it was built for
 * (esp_image_header_t.chip_id, bytes 12-13: 0 ESP32, 5 C3, 9 S3...). The
 * bootloader checks it, but only at the very end, when Update.end() fails with
 * "mismatch chip ID" after the whole flash has been rewritten: measured in
 * QEMU, an ESP32 image on a C3 downloads all of its ~900 KB first. Here the
 * first 14 bytes are enough to refuse, before anything is written.
 *
 * Collects the head of the image across chunks (a chunk can be shorter than
 * 14 bytes) and answers false only on a real header that names another chip.
 */
#if defined(ESP32) && defined(CONFIG_IDF_FIRMWARE_CHIP_ID)
#define PLYNX_OTA_CHECKS_CHIP
static bool plynxOtaHeadChipOk(uint8_t* head, uint32_t at, const uint8_t* d, size_t n)
{
    for (size_t k = 0; at + k < 14 && k < n; k++) {
        head[at + k] = d[k];
    }
    if (at >= 14 || at + n < 14) {
        return true; // already judged, or not all of it yet
    }
    if (head[0] != 0xE9) {
        return true; // not an image header: Update.write() says why
    }
    const uint16_t chip = (uint16_t)head[12] | ((uint16_t)head[13] << 8);
    return chip == (uint16_t)(CONFIG_IDF_FIRMWARE_CHIP_ID);
}
#endif

#if !defined(PLYNX_OTA_TRANSPORT_PUSH)

/*
 * ---- the download itself ----
 */

// Failure after the cloud link was dropped: clean up and arm a reconnect.
// We may be nested inside processInput() here (msgIdOutOverride is set, so
// a blocking Plynx.connect() would spin on the nesting guard): startSession()
// re-opens the socket and lets the sketch's normal Plynx.run() loop redo the
// login instead.
//
// `code` is the short word the server reads at the next login as "ota-err"
// (Plynx/PlynxFlashInfo.h): the board does not reboot after a failure, it
// reconnects, and that login is the first moment it can say what went wrong.
static void plynxOtaFail(WiFiClient& client, const char* msg, const char* code, bool abortFlash)
{
    PLYNX_LOG2(PLYNX_F("OTA: failed: "), msg);
    plynxOtaLastError = code;
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
        // Fail fast, before dropping the cloud link. Last line of defence:
        // a server that reads "ota-max" from blnkinf refuses an oversized
        // image before it ever reaches us (an old server does not).
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
        plynxOtaFail(client, "connect", "conn", false);
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
        plynxOtaFail(client, "no response", "http", false);
        return;
    }
    const char* sp = strchr(line, ' ');
    if (!sp || atoi(sp + 1) != 200) {
        plynxOtaFail(client, "http status", "http", false);
        return;
    }

    uint32_t contentLength = 0;
    for (;;) {
        const int n = plynxOtaReadLine(client, line, sizeof(line));
        if (n < 0) {
            plynxOtaFail(client, "headers", "http", false);
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
        plynxOtaFail(client, "size mismatch", "size", false);
        return;
    }
    const uint32_t expected = declaredSize ? declaredSize : contentLength;

    if (!plynxOtaFlashBegin(expected)) {
        plynxOtaFail(client, "flash begin (space?)", "space", false);
        return;
    }

    PlynxOtaSha256 sha;
    sha.init();

    uint8_t  buf[PLYNX_OTA_CHUNK];
#ifdef PLYNX_OTA_CHECKS_CHIP
    uint8_t  head[14];
#endif
    uint32_t written  = 0;
    millis_time_t deadline = PlynxMillis() + PLYNX_OTA_STREAM_TIMEOUT_MS;

    while (expected == 0 || written < expected) {
        size_t avail = client.available();
        if (!avail) {
            if (!client.connected()) {
                break; // server closed: end of body
            }
            if ((int32_t)(PlynxMillis() - deadline) >= 0) {
                plynxOtaFail(client, "stream timeout", "timeout", true);
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
#ifdef PLYNX_OTA_CHECKS_CHIP
        if (!plynxOtaHeadChipOk(head, written, buf, (size_t)n)) {
            plynxOtaFail(client, "image built for another chip", "chip", true);
            return;
        }
#endif
        sha.update(buf, (size_t)n);
        if (Update.write(buf, (size_t)n) != (size_t)n) {
            plynxOtaFail(client, "flash write", "write", true);
            return;
        }
        written += (uint32_t)n;
        deadline = PlynxMillis() + PLYNX_OTA_STREAM_TIMEOUT_MS;
        PlynxDelay(0); // yield / feed WDT during the long download
    }
    client.stop();

    if ((expected && written != expected) || written == 0) {
        plynxOtaFail(client, "truncated download", "trunc", true);
        return;
    }

    uint8_t digest[32];
    sha.final(digest);
    if (shaHex && !plynxOtaShaMatches(digest, shaHex)) {
        plynxOtaFail(client, "sha256 mismatch", "sha", true);
        return;
    }

    if (!Update.end(expected == 0)) {
        plynxOtaFail(client, "flash end", "end", false);
        return;
    }

#if defined(ESP32)
    // Arm the software fallback. The bootloader arms itself (the image goes to
    // PENDING_VERIFY on its own), and the firmware we just wrote may well be an
    // older build with no guard task, so the flag is written either way. A boot
    // that turns out to be under bootloader probation ignores it and clears it.
    plynxOtaFlagWrite(true);
#endif
    PLYNX_LOG1(PLYNX_F("OTA: success, rebooting"));
    PlynxDelay(100);
    ESP.restart();
}

#else // PLYNX_OTA_TRANSPORT_PUSH

/*
 * ---- the push receiver: the other side hands us the bytes ----
 *
 * EXPERIMENTAL. Written for Bluetooth, where the board has no way to fetch
 * anything: there is no WiFiClient, no route, no DNS. The phone holds the
 * image and pushes it, so the frames run the opposite way from the WiFi
 * handler above and nothing of that HTTP code is compiled in here.
 *
 * Everything else is deliberately shared with it: the same Update wrappers,
 * the same sha256, and above all the same two rollback clocks. A second
 * transport must not mean a second set of safety rules.
 *
 *   ota fw  <size> [<sha256hex>]   open a session   -> ota rdy <maxchunk>
 *   ota d   <offset> <raw bytes>   write            -> ota ack <written>
 *   ota fin                        close and reboot -> ota ok  <confirm_ms>
 *   ota st                         ask where we are -> ota ack <written>
 *   ota bye                        give up          -> ota ack 0
 *
 * ⚠️ CONTRACT FOR WHOEVER SENDS (read this before writing the other side).
 * The payload of "d" is raw firmware, so it can contain 0x00 anywhere and its
 * length cannot be recovered by looking for a terminator: it is taken as
 * "whatever is left of the frame", from the length in the protocol header.
 * Therefore the frame must END with the last firmware byte -- no trailing NUL
 * after it, the way the text helpers usually append one. One extra byte per
 * chunk builds an image that is byte-for-byte wrong and the only thing that
 * notices is the sha256, at the very end, after the whole flash was rewritten.
 * A chunk must also be at most the <maxchunk> announced by "rdy".
 *
 * 🔴 Every chunk carries the offset it believes it is writing, and the board
 * writes it ONLY if it is exactly where it already is. This is not belt and
 * braces: `PlynxFifo::put` is not blocking and drops silently when full, so
 * bytes can disappear with no error anywhere. Without the offset the loss
 * would surface only as a sha256 mismatch at the end, after the whole flash
 * had been rewritten and the user had waited. With it, the very next chunk
 * gets told where the board really is.
 *
 * ⚠️ Acks are batched (PLYNX_OTA_ACK_WINDOW) but a mismatch answers at once:
 * success is cheap to report late, failure is not.
 *
 * 🔴 HOW MUCH MAY BE IN FLIGHT IS OURS TO SAY, NOT THE SENDER'S TO GUESS.
 * Batching acks is the same thing as telling the sender "keep going without
 * hearing from me", so the two windows below and the transport's receive
 * buffer are one single decision taken in three places:
 *
 *   ack window  <=  push window  <=  what the RX buffer can hold
 *
 * Break the left-hand side and the transfer STALLS: the sender fills its
 * window and waits, while we sit below the threshold that would make us
 * speak. Break the right-hand side and the buffer overflows mid-frame -- and
 * since the offset check counts bytes, not frames, a truncated frame is the
 * one loss it cannot see. PLYNX_OTA_PUSH_WINDOW is the single knob: the ack
 * window is half of it and the buffer is sized from it, so there is no way to
 * shrink one and leave the others behind. The board states that number out
 * loud in its own info (PLYNX_PARAM_KV("ota-win")) instead of leaving the
 * sender to work it back out of the ack traffic.
 */

/* PLYNX_OTA_PUSH_WINDOW and PLYNX_OTA_RX_BUFFER live in PlynxConfig.h: a
 * transport has to size its receive queue long before this file is included.
 *
 * The ack window is DERIVED from the push window, not chosen beside it. They
 * were two independent literals that happened to read 4096 each, and nothing
 * said they had to: whoever shrank one for RAM would have stalled every
 * transfer without a single warning. Half is the useful ratio -- we speak while
 * the sender still has room left, so the pipe never empties waiting for us. */
#ifndef PLYNX_OTA_ACK_WINDOW
#define PLYNX_OTA_ACK_WINDOW (PLYNX_OTA_PUSH_WINDOW / 2)
#endif

#if PLYNX_OTA_ACK_WINDOW > PLYNX_OTA_PUSH_WINDOW
#error "PLYNX_OTA_ACK_WINDOW above PLYNX_OTA_PUSH_WINDOW stalls every transfer: \
the sender would fill its window and wait for an ack the board never sends."
#endif

struct PlynxOtaPush {
    bool     open;
    bool     hasSha;
    // The hash context is opened and closed on its own schedule: `fin` closes
    // it to read the digest and only THEN tears the session down, so the two
    // must not be the same flag or the context gets finished twice.
    bool     hashOpen;
    uint32_t expected;
    uint32_t written;
    uint32_t acked;
    char     shaHex[65];
    PlynxOtaSha256 sha;
#ifdef PLYNX_OTA_CHECKS_CHIP
    uint8_t  head[14];
#endif
};

static PlynxOtaPush& plynxOtaPush()
{
    static PlynxOtaPush p;
    return p;
}

// The biggest payload that still fits one frame, with room for "ota\0d\0<offset>\0".
static uint32_t plynxOtaPushMaxChunk()
{
    return (uint32_t)PLYNX_MAX_READBYTES - 32;
}

static void plynxOtaPushClose(bool abortFlash)
{
    PlynxOtaPush& p = plynxOtaPush();
    if (p.open && abortFlash) {
        plynxOtaFlashAbort();
    }
    // Close the hash too. `init()` pairs with `final()`, and here a session is
    // abandoned every time the link drops mid-transfer: re-initialising a
    // context that was never finished is the documented way to leak the SHA
    // peripheral's lock.
    if (p.hashOpen) {
        uint8_t scarto[32];
        p.sha.final(scarto);
        p.hashOpen = false;
    }
    p.open = p.hasSha = false;
    p.expected = p.written = p.acked = 0;
}

static void plynxOtaPushFail(const char* msg)
{
    PLYNX_LOG2(PLYNX_F("OTA: failed: "), msg);
    plynxOtaPushClose(true);
    Plynx.sendInternal("ota", "err", msg);
}

// ota fw <size> [<sha256hex>]
static void plynxOtaPushBegin(PlynxParam::iterator it, const PlynxParam& param)
{
    // A session still open means the last one never finished (the link went
    // away mid-transfer). Throw it away: Update cannot be resumed across a
    // begin, and a stale one would refuse the new size.
    plynxOtaPushClose(true);

    if (it >= param.end() || it.isEmpty()) {
        plynxOtaPushFail("no size");
        return;
    }
    const long size = it.asLong();
    if (size <= 0) {
        plynxOtaPushFail("bad size");
        return;
    }

    PlynxOtaPush& p = plynxOtaPush();
#if defined(PLYNX_INFO_OTA_MAX)
    // The same number the board declares as "ota-max", so the board's verdict
    // and whoever asked can never disagree by a page.
    if ((uint32_t)size > (uint32_t)(PLYNX_INFO_OTA_MAX)) {
        plynxOtaPushFail("too big");
        return;
    }
#endif

    if (++it < param.end() && !it.isEmpty()) {
        const char* s = it.asStr();
        if (strlen(s) == 64) {
            memcpy(p.shaHex, s, 65);
            p.hasSha = true;
        } else {
            PLYNX_LOG1(PLYNX_F("OTA: malformed sha256, ignoring it"));
        }
    }

    if (!plynxOtaFlashBegin((uint32_t)size)) {
        plynxOtaPushFail("flash begin");
        return;
    }
    p.sha.init();
    p.hashOpen = true;
    p.open     = true;
    p.expected = (uint32_t)size;
    p.written  = 0;
    p.acked    = 0;

    PLYNX_LOG2(PLYNX_F("OTA: push session open, bytes: "), (unsigned long)size);
    Plynx.sendInternal("ota", "rdy", (unsigned long)plynxOtaPushMaxChunk());
}

// ota d <offset> <raw bytes>
static void plynxOtaPushData(PlynxParam::iterator it, const PlynxParam& param)
{
    PlynxOtaPush& p = plynxOtaPush();
    if (!p.open) {
        Plynx.sendInternal("ota", "err", "no session");
        return;
    }
    if (it >= param.end() || it.isEmpty()) {
        plynxOtaPushFail("no offset");
        return;
    }
    const long offset = it.asLong();

    // The payload is BINARY: it can contain 0x00 anywhere, so it must never go
    // through an iterator or a C string. Its length is what is left of the
    // frame, which the protocol layer measured from the header.
    if (++it >= param.end()) {
        plynxOtaPushFail("no data");
        return;
    }
    const uint8_t* base = (const uint8_t*)param.getBuffer();
    const uint8_t* data = (const uint8_t*)it.asStr();
    if (data < base || (size_t)(data - base) > param.getLength()) {
        plynxOtaPushFail("bad frame");
        return;
    }
    const size_t n = param.getLength() - (size_t)(data - base);
    if (n == 0) {
        plynxOtaPushFail("empty chunk");
        return;
    }

    // 🔴 Not where we are: do not write, and say where we really are. The
    // other side rewinds instead of building an image with a hole in it.
    if ((uint32_t)offset != p.written) {
        PLYNX_LOG4(PLYNX_F("OTA: chunk at "), (unsigned long)offset,
                   PLYNX_F(" but we are at "), (unsigned long)p.written);
        Plynx.sendInternal("ota", "ack", (unsigned long)p.written);
        return;
    }
    if (p.written + n > p.expected) {
        plynxOtaPushFail("overrun");
        return;
    }

#ifdef PLYNX_OTA_CHECKS_CHIP
    if (!plynxOtaHeadChipOk(p.head, p.written, data, n)) {
        plynxOtaPushFail("image built for another chip");
        return;
    }
#endif
    p.sha.update(data, n);
    if (Update.write((uint8_t*)data, n) != n) {
        plynxOtaPushFail("flash write");
        return;
    }
    p.written += (uint32_t)n;

    if (p.written - p.acked >= PLYNX_OTA_ACK_WINDOW || p.written == p.expected) {
        p.acked = p.written;
        Plynx.sendInternal("ota", "ack", (unsigned long)p.written);
    }
}

// ota fin
static void plynxOtaPushFinish()
{
    PlynxOtaPush& p = plynxOtaPush();
    if (!p.open) {
        Plynx.sendInternal("ota", "err", "no session");
        return;
    }
    if (p.written != p.expected) {
        plynxOtaPushFail("truncated");
        return;
    }

    uint8_t digest[32];
    p.sha.final(digest);
    p.hashOpen = false;
    if (p.hasSha && !plynxOtaShaMatches(digest, p.shaHex)) {
        plynxOtaPushFail("sha256 mismatch");
        return;
    }

    if (!Update.end(false)) {
        plynxOtaPushFail("flash end");
        return;
    }
    plynxOtaPushClose(false);

#if defined(ESP32)
    plynxOtaFlagWrite(true);
#endif

    // 🔴 The last thing said before the link dies, and it has to carry the
    // number. The board reboots now, and from that reboot it has
    // PLYNX_OTA_CONFIRM_MS to see an authenticated session again or it rolls
    // back to the old firmware. On this transport the only thing in the world
    // that can produce that session is whoever just sent the bytes: it has to
    // know it must come back, and by when.
    PLYNX_LOG1(PLYNX_F("OTA: success, rebooting"));
    Plynx.sendInternal("ota", "ok", (unsigned long)PLYNX_OTA_CONFIRM_MS);
    PlynxDelay(200);
    ESP.restart();
}

#endif // PLYNX_OTA_TRANSPORT_PUSH

/*
 * Dispatch entry, wired into PlynxProtocol's PLYNX_INT_OTA case.
 */
// A transport lost bytes on the way in (see the declaration in PlynxProtocol.h).
//
// 🔴 If an OTA push is open, that image is now unrecoverable and we are the
// only one who knows. The offset check cannot save us here: it compares byte
// counts, and the bytes that vanished took the FRAMING with them, so what the
// parser reads next is not a chunk at the wrong offset -- it is garbage that
// may still parse. Ending the session now costs the user one retry; saying
// nothing costs a flash rewrite, a reboot, and a rollback they get no reason
// for.
//
// 🪤 Sizing the queue for the window (PLYNX_OTA_RX_BUFFER) is what should make
// this impossible. This exists because "should" was decided on paper: those
// numbers have never met a real radio. If this ever fires, the buffer is the
// thing to grow, and the serial line will say so by name.
void PlynxOtaRxLost(size_t lost)
{
    if (!lost) {
        return;
    }
    PLYNX_LOG2(PLYNX_F("OTA: receive queue overflowed, bytes lost: "),
               (unsigned long)lost);
#if defined(PLYNX_OTA_TRANSPORT_PUSH)
    if (plynxOtaPush().open) {
        plynxOtaPushFail("rx overflow");
    }
#endif
}

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

#if defined(PLYNX_OTA_TRANSPORT_PUSH)

    // param layout: [0]=verb, then whatever that verb takes. The data verb
    // carries binary after its offset, so it reads the buffer itself.
    PlynxParam::iterator it = param.begin();
    if (it >= param.end() || it.isEmpty()) {
        PLYNX_LOG1(PLYNX_F("OTA: missing verb"));
        return;
    }
    const char* verb = it.asStr();
    ++it;

    if (!strcmp(verb, "d")) {
        plynxOtaPushData(it, param);
    } else if (!strcmp(verb, "fw")) {
        plynxOtaPushBegin(it, param);
    } else if (!strcmp(verb, "fin")) {
        plynxOtaPushFinish();
    } else if (!strcmp(verb, "st")) {
        Plynx.sendInternal("ota", "ack", (unsigned long)plynxOtaPush().written);
    } else if (!strcmp(verb, "bye")) {
        plynxOtaPushClose(true);
        Plynx.sendInternal("ota", "ack", 0UL);
    } else {
        PLYNX_LOG2(PLYNX_F("OTA: unknown verb: "), verb);
    }

#else

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

#endif // PLYNX_OTA_TRANSPORT_PUSH
}

#endif // PlynxOta_h
