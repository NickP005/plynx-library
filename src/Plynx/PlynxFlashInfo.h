/**
 * @file       PlynxFlashInfo.h
 * @license    This project is released under the MIT License (MIT)
 * @brief      Flash/OTA capabilities reported by the board in blnkinf.
 *
 * The server must know, BEFORE offering an update, how big an image this
 * particular board can actually take. Otherwise the user picks a .bin, the
 * push succeeds, and the board silently refuses it (ESP8266) or the flash
 * write fails halfway (ESP32) -- an OTA stuck in PENDING forever.
 *
 * Three keys are appended to the blnkinf profile (dynamic block, in this
 * order, always last):
 *
 *   ota-max   bytes, uint32 decimal ASCII. The largest image THIS board can
 *             accept via OTA right now. 0 = cannot do OTA at all (no OTA
 *             handler compiled in, or no OTA partition). The key being
 *             ABSENT means "unknown" and must never be guessed by anyone.
 *   flash     bytes, total flash configured for this build. Informative.
 *   sk-size   bytes, size of the running sketch. Informative.
 *
 * Honesty rule: ota-max is the number the DEVICE ITSELF uses to accept or
 * refuse the image, not an estimate:
 *   ESP32   -> size of the next OTA partition, i.e. exactly what
 *              Update.begin(size) validates against;
 *   ESP8266 -> the very expression used by plynxOtaFlashBegin()
 *              (Plynx/PlynxOta.h), page-aligned, single source of truth.
 * If the server and the board ever disagreed by one page there would be an
 * image size that one accepts and the other rejects.
 *
 * Only ESP32 and ESP8266 emit these keys. Every other target (AVR, SAMD,
 * STM32, Particle, mbed, shields, ...) never includes this header, so it
 * pays zero flash and zero RAM.
 *
 * Include it from the board shell BEFORE <PlynxApiArduino.h> (that is where
 * the preprocessor assembles the profile) and AFTER the
 * PLYNX_OTA_HAS_DEFAULT decision.
 *
 * Escape hatch: a sketch that replaces the built-in OTA with its own
 * handler (PLYNX_NO_OTA + a custom PLYNX_WRITE(InternalPinOTA)) can define
 * PLYNX_INFO_OTA_MAX itself before including the shell; we never override
 * an existing definition.
 */

#ifndef PlynxFlashInfo_h
#define PlynxFlashInfo_h

#if defined(ESP32) || defined(ESP8266)

#ifndef PLYNX_INFO_FLASH_TOTAL
#define PLYNX_INFO_FLASH_TOTAL  (ESP.getFlashChipSize())
#endif

#ifndef PLYNX_INFO_FLASH_SKETCH
#define PLYNX_INFO_FLASH_SKETCH (ESP.getSketchSize())
#endif

#ifndef PLYNX_INFO_OTA_MAX

  #if !defined(PLYNX_OTA_HAS_DEFAULT)
    // No OTA handler in this build (PLYNX_NO_OTA, or a shell without OTA):
    // the board would ignore the frame. Reporting a hard 0 is a true and
    // actionable fact -- better than omitting the key, which would let the
    // server send a frame into the void.
    #define PLYNX_INFO_OTA_MAX 0UL

  #elif defined(ESP32)
    #include <esp_ota_ops.h>
    static inline uint32_t plynxOtaSlotBytes()
    {
        // NULL when the running partition table has no second app slot
        // (single-app / factory-only layouts): the board cannot OTA.
        const esp_partition_t* p = esp_ota_get_next_update_partition(NULL);
        return p ? (uint32_t)p->size : 0UL;
    }
    #define PLYNX_INFO_OTA_MAX (plynxOtaSlotBytes())

  #else // ESP8266
    // MUST stay byte-identical to plynxOtaFlashBegin() in Plynx/PlynxOta.h.
    #define PLYNX_INFO_OTA_MAX ((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000)
  #endif

#endif // PLYNX_INFO_OTA_MAX

/*
 * ota-err: why the last download OTA failed, said ONCE at the next login.
 *
 * After a failed download the board does not reboot: it reconnects to the
 * server (Plynx/PlynxOta.h, plynxOtaFail). Until 1.3.0 that reconnect said
 * nothing, so the server could only notice that the build had not changed,
 * send the same image again, and give up after three attempts with a reason
 * nobody could act on. Now the login carries a short word: "chip" (the image
 * is for another chip), "space", "end" (the image did not validate), "sha",
 * "size", "write", "trunc", "timeout", "http", "conn".
 *
 * Read and cleared in one step, so a board that stays up for days does not
 * repeat a failure that belongs to an update long gone. A server that does not
 * know the key skips it, like every unknown blnkinf key.
 *
 * Only the download handler (WiFi, Ethernet, GSM) sets it: on the push
 * transport the board is talking to whoever sends the bytes, and tells them
 * directly ("ota err ...").
 */
#if defined(PLYNX_OTA_HAS_DEFAULT) && !defined(PLYNX_OTA_TRANSPORT_PUSH) && !defined(PLYNX_INFO_OTA_ERR)
static const char* plynxOtaLastError = NULL;

static inline const char* plynxOtaTakeError()
{
    const char* e = plynxOtaLastError;
    plynxOtaLastError = NULL;
    return e;
}
#define PLYNX_INFO_OTA_ERR (plynxOtaTakeError())
#endif

#endif // ESP32 || ESP8266

#endif
