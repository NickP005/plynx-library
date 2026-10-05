/**
 * @file       PlynxDetectDevice.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       May 2016
 * @brief
 *
 */

#ifndef PlynxDetectDevice_h
#define PlynxDetectDevice_h

// General defines

#define PLYNX_NEWLINE "\r\n"

#define PLYNX_CONCAT(a, b) a ## b
#define PLYNX_CONCAT2(a, b) PLYNX_CONCAT(a, b)

#define PLYNX_STRINGIFY(x) #x
#define PLYNX_TOSTRING(x) PLYNX_STRINGIFY(x)

#define PLYNX_COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define PLYNX_ATTR_PACKED __attribute__ ((__packed__))
#define PLYNX_NORETURN __attribute__ ((noreturn))
#define PLYNX_UNUSED __attribute__((__unused__))
#define PLYNX_DEPRECATED __attribute__ ((deprecated))
#define PLYNX_CONSTRUCTOR __attribute__((constructor))

// Causes problems on some platforms
#define PLYNX_FORCE_INLINE inline //__attribute__((always_inline))

#ifndef PLYNX_INFO_CPU

    /******************************************
     * ATmega
     */

    #if   defined(__AVR_ATmega168__)
    #define PLYNX_INFO_CPU      "ATmega168"
    #elif defined(__AVR_ATmega328P__)
    #define PLYNX_INFO_CPU      "ATmega328P"
    #elif defined(__AVR_ATmega1280__)
    #define PLYNX_INFO_CPU      "ATmega1280"
    #elif defined(__AVR_ATmega1284__)
    #define PLYNX_INFO_CPU      "ATmega1284"
    #elif defined(__AVR_ATmega2560__)
    #define PLYNX_INFO_CPU      "ATmega2560"
    #elif defined(__AVR_ATmega32U4__)
    #define PLYNX_INFO_CPU      "ATmega32U4"
    #elif defined(__SAM3X8E__)
    #define PLYNX_INFO_CPU      "AT91SAM3X8E"

    /******************************************
     * ATtiny
     */

    #elif defined(__AVR_ATtiny25__)
    #define PLYNX_INFO_CPU      "ATtiny25"
    #elif defined(__AVR_ATtiny45__)
    #define PLYNX_INFO_CPU      "ATtiny45"
    #elif defined(__AVR_ATtiny85__)
    #define PLYNX_INFO_CPU      "ATtiny85"
    #elif defined(__AVR_ATtiny24__)
    #define PLYNX_INFO_CPU      "ATtiny24"
    #elif defined(__AVR_ATtiny44__)
    #define PLYNX_INFO_CPU      "ATtiny44"
    #elif defined(__AVR_ATtiny84__)
    #define PLYNX_INFO_CPU      "ATtiny84"
    #elif defined(__AVR_ATtiny2313__)
    #define PLYNX_INFO_CPU      "ATtiny2313"
    #elif defined(__AVR_ATtiny4313__)
    #define PLYNX_INFO_CPU      "ATtiny4313"
    #endif

    /******************************************
     * ESP32 family
     *
     * "dev" stays "ESP32" for the whole family (servers and apps of before
     * 1.3.0 read that word, and changing it would make them refuse updates
     * to boards that are fine). The chip itself goes in "cpu", spelled the way
     * ESP-IDF spells its target: "esp32", "esp32c3", "esp32s3", "esp32s2",
     * "esp32c6"... An image built for one of them does not boot on another,
     * and before this key the server had no way to tell them apart.
     *
     * CONFIG_IDF_TARGET is a string literal in sdkconfig.h on every core that
     * knows more than one chip (2.x and 3.x). Core 1.0.x only builds for the
     * classic ESP32 and has no such macro: there the key is simply absent,
     * which the server already reads as "classic ESP32".
     */
    #if !defined(PLYNX_INFO_CPU) && (defined(ARDUINO_ARCH_ESP32) || defined(ESP32))
        #if defined(__has_include)
            #if __has_include(<sdkconfig.h>)
                #include <sdkconfig.h>
            #endif
        #endif
        #if defined(CONFIG_IDF_TARGET)
        #define PLYNX_INFO_CPU      CONFIG_IDF_TARGET
        #endif
    #endif
#endif

#ifndef PLYNX_INFO_DEVICE

    #if   defined(ENERGIA)

        #define PLYNX_NO_YIELD
        #define PLYNX_USE_128_VPINS
        #define PLYNX_USE_INTERNAL_DTOSTRF

        #if   defined(ENERGIA_ARCH_MSP430)
            #define PLYNX_INFO_DEVICE  "LaunchPad MSP430"
            #define PLYNX_INFO_CPU     "MSP430"
            #define PLYNX_NO_FLOAT
            #define PLYNX_NO_LONGLONG
        #elif defined(ENERGIA_ARCH_MSP432)
            #define PLYNX_INFO_DEVICE  "LaunchPad MSP432"
            #define PLYNX_INFO_CPU     "MSP432"
        #elif defined(ENERGIA_ARCH_TIVAC)
            #define PLYNX_INFO_DEVICE  "LaunchPad"

        #elif defined(ENERGIA_ARCH_CC3200EMT) || defined(ENERGIA_ARCH_CC3200)
            #define PLYNX_INFO_CONNECTION  "CC3200"
            #define PLYNX_SEND_CHUNK 64
            #define PLYNX_BUFFERS_SIZE 1024

            #if   defined(ENERGIA_CC3200_LAUNCHXL) //TODO: This is a bug in Energia IDE
            #define PLYNX_INFO_DEVICE  "CC3200 LaunchXL"
            #elif defined(ENERGIA_RedBearLab_CC3200)
            #define PLYNX_INFO_DEVICE  "RBL CC3200"
            #elif defined(ENERGIA_RedBearLab_WiFiMini)
            #define PLYNX_INFO_DEVICE  "RBL WiFi Mini"
            #elif defined(ENERGIA_RedBearLab_WiFiMicro)
            #define PLYNX_INFO_DEVICE  "RBL WiFi Micro"
            #endif
        #elif defined(ENERGIA_ARCH_CC3220EMT) || defined(ENERGIA_ARCH_CC3220)
            #define PLYNX_INFO_CONNECTION  "CC3220"
            #define PLYNX_SEND_CHUNK 64
            #define PLYNX_BUFFERS_SIZE 1024

            #define PLYNX_INFO_DEVICE  "CC3220"
            #define PLYNX_INFO_CPU     "CC3220"
        #endif

        #if !defined(PLYNX_INFO_DEVICE)
        #define PLYNX_INFO_DEVICE  "Energia"
        #endif

    #elif defined(LINUX)

        #define PLYNX_INFO_DEVICE  "Linux"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 4096

    #elif defined(SPARK) || defined(PARTICLE)

        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024

        #if PLATFORM_ID==0
        #define PLYNX_INFO_DEVICE  "Particle Core"
        #undef PLYNX_BUFFERS_SIZE // Use default on Core
        #elif PLATFORM_ID==6
        #define PLYNX_INFO_DEVICE  "Particle Photon"
        #elif PLATFORM_ID==8
        #define PLYNX_INFO_DEVICE  "Particle P1"
        #elif PLATFORM_ID==9
        #define PLYNX_INFO_DEVICE  "Particle Ethernet"
        #elif PLATFORM_ID==10
        #define PLYNX_INFO_DEVICE  "Particle Electron"
        #elif PLATFORM_ID==31
        #define PLYNX_INFO_DEVICE  "Particle RPi"
        #elif PLATFORM_ID==82
        #define PLYNX_INFO_DEVICE  "Digistump Oak"
        #elif PLATFORM_ID==88
        #define PLYNX_INFO_DEVICE  "RedBear Duo"
        #elif PLATFORM_ID==103
        #define PLYNX_INFO_DEVICE  "Bluz"
        #else
        #if defined(PLYNX_DEBUG_ALL)
            #warning "Cannot detect board type"
        #endif
        #define PLYNX_INFO_DEVICE  "Particle"
        #endif

    #elif defined(__MBED__)

        #define PLYNX_INFO_DEVICE  "MBED"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 512
        #define noInterrupts() __disable_irq()
        #define interrupts()   __enable_irq()

    #elif defined(ARDUINO) && defined(MPIDE)
        #define PLYNX_NO_YIELD

        #if   defined(_BOARD_UNO_)
        #define PLYNX_INFO_DEVICE  "chipKIT Uno32"
        #else
        #define PLYNX_INFO_DEVICE  "chipKIT"
        #endif

    #elif defined(ARDUINO) && defined(ARDUINO_AMEBA)
        #if defined(BOARD_RTL8710)
        #define PLYNX_INFO_DEVICE  "RTL8710"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(BOARD_RTL8711AM)
        #define PLYNX_INFO_DEVICE  "RTL8711AM"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(BOARD_RTL8195A)
        #define PLYNX_INFO_DEVICE  "RTL8195A"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #else
        #define PLYNX_INFO_DEVICE  "Ameba"
        #endif

    #elif defined(ARDUINO) && defined(TEENSYDUINO)

        #if   defined(__MK66FX1M0__)
        #define PLYNX_INFO_DEVICE  "Teensy 3.6"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(__MK64FX512__)
        #define PLYNX_INFO_DEVICE  "Teensy 3.5"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(__MK20DX256__)
        #define PLYNX_INFO_DEVICE  "Teensy 3.2/3.1"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif   defined(__MK20DX128__)
        #define PLYNX_INFO_DEVICE  "Teensy 3.0"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif   defined(__MKL26Z64__)
        #define PLYNX_INFO_DEVICE  "Teensy LC"
        #define PLYNX_BUFFERS_SIZE 512
        #elif   defined(ARDUINO_ARCH_AVR)
        #define PLYNX_INFO_DEVICE  "Teensy 2.0"
        #else
        #define PLYNX_INFO_DEVICE  "Teensy"
        #endif

    #elif defined(ARDUINO)

        #if defined(ARDUINO_ARCH_SAMD) || defined(ESP32) || defined(ESP8266)
            #define PLYNX_USE_128_VPINS
            #define PLYNX_BUFFERS_SIZE 1024
        #endif

        #if defined(ARDUINO_ARCH_AVR)
            #define PLYNX_USE_INTERNAL_ATOLL
        #endif

        #if defined(ARDUINO_ARCH_SAMD)
            #define PLYNX_USE_INTERNAL_DTOSTRF
        #endif

        /* Arduino AVR */
        #if   defined(ARDUINO_AVR_NANO)
        #define PLYNX_INFO_DEVICE  "Arduino Nano"
        #elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_DUEMILANOVE)
        #define PLYNX_INFO_DEVICE  "Arduino Uno"
        #elif defined(ARDUINO_AVR_YUN)
        #define PLYNX_INFO_DEVICE  "Arduino Yun"
        #elif defined(ARDUINO_AVR_MINI)
        #define PLYNX_INFO_DEVICE  "Arduino Mini"
        #elif defined(ARDUINO_AVR_ETHERNET)
        #define PLYNX_INFO_DEVICE  "Arduino Ethernet"
        #elif defined(ARDUINO_AVR_FIO)
        #define PLYNX_INFO_DEVICE  "Arduino Fio"
        #elif defined(ARDUINO_AVR_BT)
        #define PLYNX_INFO_DEVICE  "Arduino BT"
        #elif defined(ARDUINO_AVR_PRO)
        #define PLYNX_INFO_DEVICE  "Arduino Pro"
        #elif defined(ARDUINO_AVR_NG)
        #define PLYNX_INFO_DEVICE  "Arduino NG"
        #elif defined(ARDUINO_AVR_GEMMA)
        #define PLYNX_INFO_DEVICE  "Arduino Gemma"
        #elif defined(ARDUINO_AVR_MEGA) || defined(ARDUINO_AVR_MEGA2560)
        #define PLYNX_INFO_DEVICE  "Arduino Mega"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(ARDUINO_AVR_ADK)
        #define PLYNX_INFO_DEVICE  "Arduino Mega ADK"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(ARDUINO_AVR_LEONARDO)
        #define PLYNX_INFO_DEVICE  "Arduino Leonardo"
        #elif defined(ARDUINO_AVR_MICRO)
        #define PLYNX_INFO_DEVICE  "Arduino Micro"
        #elif defined(ARDUINO_AVR_ESPLORA)
        #define PLYNX_INFO_DEVICE  "Arduino Esplora"
        #elif defined(ARDUINO_AVR_LILYPAD)
        #define PLYNX_INFO_DEVICE  "Lilypad"
        #elif defined(ARDUINO_AVR_LILYPAD_USB)
        #define PLYNX_INFO_DEVICE  "Lilypad USB"
        #elif defined(ARDUINO_AVR_ROBOT_MOTOR)
        #define PLYNX_INFO_DEVICE  "Robot Motor"
        #elif defined(ARDUINO_AVR_ROBOT_CONTROL)
        #define PLYNX_INFO_DEVICE  "Robot Control"

        /* Arduino megaAVR */
        #elif defined(ARDUINO_AVR_UNO_WIFI_REV2)
        #define PLYNX_INFO_DEVICE  "Arduino UNO WiFi Rev2"

        /* Arduino SAM */
        #elif defined(ARDUINO_SAM_DUE)
        #define PLYNX_INFO_DEVICE  "Arduino Due"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024

        /* Arduino SAMD */
        #elif defined(ARDUINO_SAMD_ZERO)
        #define PLYNX_INFO_DEVICE  "Arduino Zero"
        #elif defined(ARDUINO_SAMD_MKR1000)
        #define PLYNX_INFO_DEVICE  "MKR1000"
        #elif defined(ARDUINO_SAMD_MKRZERO)
        #define PLYNX_INFO_DEVICE  "MKRZERO"
        #elif defined(ARDUINO_SAMD_MKRNB1500)
        #define PLYNX_INFO_DEVICE  "MKR NB 1500"
        #elif defined(ARDUINO_SAMD_MKRGSM1400)
        #define PLYNX_INFO_DEVICE  "MKR GSM 1400"
        #elif defined(ARDUINO_SAMD_MKRWAN1300)
        #define PLYNX_INFO_DEVICE  "MKR WAN 1300"
        #elif defined(ARDUINO_SAMD_MKRFox1200)
        #define PLYNX_INFO_DEVICE  "MKR FOX 1200"
        #elif defined(ARDUINO_SAMD_MKRWIFI1010)
        #define PLYNX_INFO_DEVICE  "MKR WiFi 1010"
        #elif defined(ARDUINO_SAMD_MKRVIDOR4000)
        #define PLYNX_INFO_DEVICE  "MKR Vidor 4000"

        /* Intel */
        #elif defined(ARDUINO_GALILEO)
        #define PLYNX_INFO_DEVICE  "Galileo"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 4096
        #elif defined(ARDUINO_GALILEOGEN2)
        #define PLYNX_INFO_DEVICE  "Galileo Gen2"
        #define PLYNX_BUFFERS_SIZE 4096
        #define PLYNX_USE_128_VPINS
        #elif defined(ARDUINO_EDISON)
        #define PLYNX_INFO_DEVICE  "Edison"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 4096
        #elif defined(ARDUINO_ARCH_ARC32)
        #define PLYNX_INFO_DEVICE  "Arduino 101"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024

        /* Konekt */
        #elif defined(ARDUINO_DASH)
        #define PLYNX_INFO_DEVICE  "Dash"
        #elif defined(ARDUINO_DASHPRO)
        #define PLYNX_INFO_DEVICE  "Dash Pro"

        /* Red Bear Lab */
        #elif defined(ARDUINO_RedBear_Duo)
        #define PLYNX_INFO_DEVICE  "RedBear Duo"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(ARDUINO_BLEND)
        #define PLYNX_INFO_DEVICE  "Blend"
        #elif defined(ARDUINO_BLEND_MICRO_8MHZ) || defined(ARDUINO_BLEND_MICRO_16MHZ)
        #define PLYNX_INFO_DEVICE  "Blend Micro"
        #elif defined(ARDUINO_RBL_nRF51822)
        #define PLYNX_INFO_DEVICE  "BLE Nano"

        /* ESP8266 */
        #elif defined(ARDUINO_ESP8266_NODEMCU)
        #define PLYNX_INFO_DEVICE  "NodeMCU"
        #elif defined(ARDUINO_ARCH_ESP8266)
        #define PLYNX_INFO_DEVICE  "ESP8266"

        /* ESP32 */
        #elif defined(ARDUINO_ARCH_ESP32)
        #define PLYNX_INFO_DEVICE  "ESP32"

        /* STM32 */
        #elif defined(ARDUINO_ARCH_STM32F1)
        #define PLYNX_INFO_DEVICE  "STM32F1"
        #define PLYNX_NO_YIELD
        #elif defined(ARDUINO_ARCH_STM32F3)
        #define PLYNX_INFO_DEVICE  "STM32F3"
        #define PLYNX_NO_YIELD
        #elif defined(ARDUINO_ARCH_STM32F4)
        #define PLYNX_INFO_DEVICE  "STM32F4"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #define PLYNX_NO_YIELD

        /* Digistump */
        #elif defined(ARDUINO_ESP8266_OAK)
        #define PLYNX_INFO_DEVICE  "Oak"
        #define PLYNX_USE_128_VPINS
        #elif defined(ARDUINO_AVR_DIGISPARK)
        #define PLYNX_INFO_DEVICE  "Digispark"
        #define PLYNX_NO_YIELD
        #elif defined(ARDUINO_AVR_DIGISPARKPRO)
        #define PLYNX_INFO_DEVICE  "Digispark Pro"
        #define PLYNX_NO_YIELD

        /* Microduino */
        #elif defined(ARDUINO_AVR_USB)
        #define PLYNX_INFO_DEVICE  "CoreUSB"
        #elif defined(ARDUINO_AVR_PLUS)
        #define PLYNX_INFO_DEVICE  "Core+"
        #elif defined(ARDUINO_AVR_RF)
        #define PLYNX_INFO_DEVICE  "CoreRF"

        /* Wildfire */
        #elif defined(ARDUINO_WILDFIRE_V2)
        #define PLYNX_INFO_DEVICE  "Wildfire V2"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(ARDUINO_WILDFIRE_V3)
        #define PLYNX_INFO_DEVICE  "Wildfire V3"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024
        #elif defined(ARDUINO_WILDFIRE_V4)
        #define PLYNX_INFO_DEVICE  "Wildfire V4"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024

         /* Seeed studio */
        #elif defined(SEEED_WIO_TERMINAL)
        #define PLYNX_INFO_DEVICE "Seeed Wio Terminal"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024

        /* Simblee */
        #elif defined(__Simblee__)
        #define PLYNX_INFO_DEVICE  "Simblee"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 512

        /* RFduino */
        #elif defined(__RFduino__)
        #define PLYNX_INFO_DEVICE  "RFduino"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 512

        /* Nordic NRF5x */
        #elif defined(ARDUINO_ARCH_NRF5)
        #define PLYNX_INFO_DEVICE  "nRF5"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 512

        #else
        #if defined(PLYNX_DEBUG_ALL)
            #warning "Cannot detect board type"
        #endif
        #define PLYNX_INFO_DEVICE  "Arduino"
        #endif

    #elif defined(TI_CC3220)
        #define PLYNX_INFO_DEVICE  "TI CC3220"
        #define PLYNX_USE_128_VPINS
        #define PLYNX_BUFFERS_SIZE 1024

        #define PLYNX_USE_INTERNAL_DTOSTRF

    #else

        #define PLYNX_INFO_DEVICE  "Custom platform"

    #endif

    #if !defined(PLYNX_MAX_READBYTES) && defined(PLYNX_BUFFERS_SIZE)
    #define PLYNX_MAX_READBYTES  PLYNX_BUFFERS_SIZE
    #endif

    #if !defined(PLYNX_MAX_SENDBYTES) && defined(PLYNX_BUFFERS_SIZE)
    #define PLYNX_MAX_SENDBYTES  PLYNX_BUFFERS_SIZE
    #endif

    // Print diagnostics

    #if defined(PLYNX_DEBUG_ALL)
        #if defined(PLYNX_INFO_DEVICE)
        #pragma message ("PLYNX_INFO_DEVICE=" PLYNX_TOSTRING(PLYNX_INFO_DEVICE))
        #endif

        #if defined(PLYNX_INFO_CPU)
        #pragma message ("PLYNX_INFO_CPU="    PLYNX_TOSTRING(PLYNX_INFO_CPU))
        #endif

        #if defined(PLYNX_BUFFERS_SIZE)
        #pragma message ("PLYNX_BUFFERS_SIZE=" PLYNX_TOSTRING(PLYNX_BUFFERS_SIZE))
        #endif
    #endif

#endif

#endif
