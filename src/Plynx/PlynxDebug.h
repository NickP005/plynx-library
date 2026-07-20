/**
 * @file       PlynxDebug.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Debug utilities
 *
 */

#ifndef PlynxDebug_h
#define PlynxDebug_h

#include <Plynx/PlynxConfig.h>

#include <stddef.h>
#ifdef ESP8266
    extern "C" {
    #include "ets_sys.h"
    #include "os_type.h"
    #include "mem.h"
    }
#else
    #include <inttypes.h>
#endif

#if defined(ARDUINO_ARCH_ARC32)
    typedef uint64_t millis_time_t;
#else
    typedef uint32_t millis_time_t;
#endif

void            PlynxDelay(millis_time_t ms);
millis_time_t   PlynxMillis();
size_t          PlynxFreeRam();
void            PlynxReset() PLYNX_NORETURN;
void            PlynxFatal() PLYNX_NORETURN;


#if defined(SPARK) || defined(PARTICLE)
    #include "application.h"
#endif

#if defined(ARDUINO)
    #if ARDUINO >= 100
        #include <Arduino.h>
    #else
        #include <WProgram.h>
    #endif
#endif

#if defined(LINUX)
    #if defined(RASPBERRY)
        #include <wiringPi.h>
    #endif
#endif

#if !defined(PLYNX_RUN_YIELD)
    #if defined(PLYNX_NO_YIELD)
        #define PLYNX_RUN_YIELD() {}
    #elif defined(SPARK) || defined(PARTICLE)
        #define PLYNX_RUN_YIELD() { Particle.process(); }
    #elif !defined(ARDUINO) || (ARDUINO < 151)
        #define PLYNX_RUN_YIELD() {}
    #else
        #define PLYNX_RUN_YIELD() { PlynxDelay(0); }
    #endif
#endif

#if defined(__AVR__)
    #include <avr/pgmspace.h>
    #define PLYNX_HAS_PROGMEM
    #define PLYNX_PROGMEM PROGMEM
    #define PLYNX_F(s) F(s)
    #define PLYNX_PSTR(s) PSTR(s)
#else
    #define PLYNX_PROGMEM
    #define PLYNX_F(s) s
    #define PLYNX_PSTR(s) s
#endif

#ifdef ARDUINO_AVR_DIGISPARK
    typedef fstr_t __FlashStringHelper;
#endif

#if defined(PLYNX_DEBUG_ALL) && !(__cplusplus >= 201103L || defined(__GXX_EXPERIMENTAL_CXX0X__))
    #warning "Compiler features not enabled -> please contact yor board vendor to enable c++0x"
#endif

// Diagnostic defines

#define PLYNX_FATAL(msg)     { PLYNX_LOG1(msg); PlynxFatal(); }
#define PLYNX_LOG_RAM()      { PLYNX_LOG2(PLYNX_F("Free RAM: "), PlynxFreeRam()); }
#define PLYNX_LOG_FN()       PLYNX_LOG3(PLYNX_F(__FUNCTION__), '@', __LINE__);
#define PLYNX_LOG_TROUBLE(t) PLYNX_LOG2(PLYNX_F("Trouble detected: http://docs.plynx.cc/#troubleshooting-"), t)

#ifndef PLYNX_PRINT
#undef PLYNX_DEBUG
#endif

#ifdef PLYNX_DEBUG_ALL
#define PLYNX_DEBUG
#endif

#ifdef PLYNX_PRINT

    #if defined(ARDUINO) || defined(SPARK) || defined(PARTICLE)

#if defined(ARDUINO_ARCH_ARC32)
        // This will cause error - on purpose
        #define PLYNX_LOG(msg, ...)  PLYNX_LOG_UNAVAILABLE(msg, ##__VA_ARGS__)
#else
        #define PLYNX_LOG(msg, ...)  plynx_dbg_print(PLYNX_PSTR(msg), ##__VA_ARGS__)
#endif

        #define PLYNX_LOG1(p1)            { PLYNX_LOG_TIME(); PLYNX_PRINT.println(p1); }
        #define PLYNX_LOG2(p1,p2)         { PLYNX_LOG_TIME(); PLYNX_PRINT.print(p1); PLYNX_PRINT.println(p2); }
        #define PLYNX_LOG3(p1,p2,p3)      { PLYNX_LOG_TIME(); PLYNX_PRINT.print(p1); PLYNX_PRINT.print(p2); PLYNX_PRINT.println(p3); }
        #define PLYNX_LOG4(p1,p2,p3,p4)   { PLYNX_LOG_TIME(); PLYNX_PRINT.print(p1); PLYNX_PRINT.print(p2); PLYNX_PRINT.print(p3); PLYNX_PRINT.println(p4); }
        #define PLYNX_LOG6(p1,p2,p3,p4,p5,p6) { PLYNX_LOG_TIME(); PLYNX_PRINT.print(p1); PLYNX_PRINT.print(p2); PLYNX_PRINT.print(p3); PLYNX_PRINT.print(p4); PLYNX_PRINT.print(p5); PLYNX_PRINT.println(p6); }
        #define PLYNX_LOG_IP(msg, ip)     { PLYNX_LOG_TIME(); PLYNX_PRINT.print(PLYNX_F(msg)); \
                                            PLYNX_PRINT.print(ip[0]); PLYNX_PRINT.print('.');  \
                                            PLYNX_PRINT.print(ip[1]); PLYNX_PRINT.print('.');  \
                                            PLYNX_PRINT.print(ip[2]); PLYNX_PRINT.print('.');  \
                                            PLYNX_PRINT.println(ip[3]); }
        #define PLYNX_LOG_IP_REV(msg, ip) { PLYNX_LOG_TIME(); PLYNX_PRINT.print(PLYNX_F(msg)); \
                                            PLYNX_PRINT.print(ip[3]); PLYNX_PRINT.print('.');  \
                                            PLYNX_PRINT.print(ip[2]); PLYNX_PRINT.print('.');  \
                                            PLYNX_PRINT.print(ip[1]); PLYNX_PRINT.print('.');  \
                                            PLYNX_PRINT.println(ip[0]); }

        static
        void PLYNX_LOG_TIME() {
            PLYNX_PRINT.print('[');
            PLYNX_PRINT.print(PlynxMillis());
            PLYNX_PRINT.print(PLYNX_F("] "));
        }

#ifdef PLYNX_DEBUG
        #include <ctype.h>
        #define PLYNX_DBG_BREAK()    { for(;;); }
        #define PLYNX_ASSERT(expr)   { if(!(expr)) { PLYNX_LOG2(PLYNX_F("Assertion failed: "), PLYNX_F(#expr)); PLYNX_DBG_BREAK() } }

        static
        void PLYNX_DBG_DUMP(const char* msg, const void* addr, size_t len) {
            if (len) {
                PLYNX_LOG_TIME();
                PLYNX_PRINT.print(msg);
                int l2 = len;
                const uint8_t* octets = (const uint8_t*)addr;
                bool prev_print = true;
                while (l2--) {
                    const uint8_t c = *octets++ & 0xFF;
                    if (c >= 32 && c < 127) {
                        if (!prev_print) { PLYNX_PRINT.print(']'); }
                        PLYNX_PRINT.print((char)c);
                        prev_print = true;
                    } else {
                        PLYNX_PRINT.print(prev_print?'[':'|');
                        if (c < 0x10) { PLYNX_PRINT.print('0'); }
                        PLYNX_PRINT.print(c, HEX);
                        prev_print = false;
                    }
                }
                if (!prev_print) {
                    PLYNX_PRINT.print(']');
                }
                PLYNX_PRINT.println();
            }
        }
#endif

        #if !defined(ARDUINO_ARCH_ARC32)
        #include <stdio.h>
        #include <stdarg.h>

        PLYNX_UNUSED
        void plynx_dbg_print(const char* PLYNX_PROGMEM fmt, ...)
        {
            va_list ap;
            va_start(ap, fmt);
            char buff[128];
            PLYNX_PRINT.print('[');
            PLYNX_PRINT.print(PlynxMillis());
            PLYNX_PRINT.print(PLYNX_F("] "));
#if defined(__AVR__)
            vsnprintf_P(buff, sizeof(buff), fmt, ap);
#else
            vsnprintf(buff, sizeof(buff), fmt, ap);
#endif
            PLYNX_PRINT.println(buff);
            va_end(ap);
        }
        #endif // ARDUINO_ARCH_ARC32

    #elif defined(__MBED__)

        #define PLYNX_LOG(msg, ...)       { PLYNX_PRINT.printf("[%ld] " msg PLYNX_NEWLINE, PlynxMillis(), ##__VA_ARGS__); }
        #define PLYNX_LOG1(p1)            { PLYNX_LOG(p1);}
        #define PLYNX_LOG2(p1,p2)         { PLYNX_LOG(p1,p2);}
        #define PLYNX_LOG3(p1,p2,p3)      { PLYNX_LOG(p1,p2,p3);}
        #define PLYNX_LOG4(p1,p2,p3,p4)   { PLYNX_LOG(p1,p2,p3,p4);}
        #define PLYNX_LOG6(p1,p2,p3,p4,p5,p6)   { PLYNX_LOG(p1,p2,p3,p4,p5,p6);}

        #define PLYNX_LOG_TIME() PLYNX_PRINT.printf("[%ld]", PlynxMillis());

#ifdef PLYNX_DEBUG
        #define PLYNX_DBG_BREAK()    raise(SIGTRAP);
        #define PLYNX_ASSERT(expr)   assert(expr)

        static
        void PLYNX_DBG_DUMP(const char* msg, const void* addr, size_t len) {
            PLYNX_LOG_TIME();
            PLYNX_PRINT.printf(msg);
            int l2 = len;
            const uint8_t* octets = (const uint8_t*)addr;
            bool prev_print = true;
            while (l2--) {
                const uint8_t c = *octets++ & 0xFF;
                if (c >= 32 && c < 127) {
                    if (!prev_print) { PLYNX_PRINT.putc(']'); }
                    PLYNX_PRINT.putc((char)c);
                    prev_print = true;
                } else {
                    PLYNX_PRINT.putc(prev_print?'[':'|');
                    PLYNX_PRINT.printf("%02x", c);
                    prev_print = false;
                }
            }
            PLYNX_PRINT.printf("%s" PLYNX_NEWLINE, prev_print?"":"]");
        }
#endif

    #elif defined(LINUX)

        #include <assert.h>
        #include <stdio.h>
        #include <string.h>
        #include <errno.h>
        #include <signal.h>

        #include <iostream>
        using namespace std;
        #define PLYNX_LOG(msg, ...)       { fprintf(PLYNX_PRINT, "[%ld] " msg PLYNX_NEWLINE, PlynxMillis(), ##__VA_ARGS__); }
        #define PLYNX_LOG1(p1)            { PLYNX_LOG_TIME(); cout << p1 << endl; }
        #define PLYNX_LOG2(p1,p2)         { PLYNX_LOG_TIME(); cout << p1 << p2 << endl; }
        #define PLYNX_LOG3(p1,p2,p3)      { PLYNX_LOG_TIME(); cout << p1 << p2 << p3 << endl; }
        #define PLYNX_LOG4(p1,p2,p3,p4)   { PLYNX_LOG_TIME(); cout << p1 << p2 << p3 << p4 << endl; }
        #define PLYNX_LOG6(p1,p2,p3,p4,p5,p6)   { PLYNX_LOG_TIME(); cout << p1 << p2 << p3 << p4 << p5 << p6 << endl; }

        #define PLYNX_LOG_TIME() cout << '[' << PlynxMillis() << "] ";

#ifdef PLYNX_DEBUG
        #define PLYNX_DBG_BREAK()    raise(SIGTRAP);
        #define PLYNX_ASSERT(expr)   assert(expr)

        static
        void PLYNX_DBG_DUMP(const char* msg, const void* addr, size_t len) {
            PLYNX_LOG_TIME();
            fprintf(PLYNX_PRINT, "%s", msg);
            int l2 = len;
            const uint8_t* octets = (const uint8_t*)addr;
            bool prev_print = true;
            while (l2--) {
                const uint8_t c = *octets++ & 0xFF;
                if (c >= 32 && c < 127) {
                    if (!prev_print) { fputc(']', PLYNX_PRINT); }
                    fputc((char)c, PLYNX_PRINT);
                    prev_print = true;
                } else {
                    fputc(prev_print?'[':'|', PLYNX_PRINT);
                    fprintf(PLYNX_PRINT, "%02x", c);
                    prev_print = false;
                }
            }
            fprintf(PLYNX_PRINT, "%s" PLYNX_NEWLINE, prev_print?"":"]");
        }
#endif

    #else

        #warning "Cannot detect platform"

    #endif

#endif

#ifndef PLYNX_LOG
    #define PLYNX_LOG(...)
    #define PLYNX_LOG1(p1)
    #define PLYNX_LOG2(p1,p2)
    #define PLYNX_LOG3(p1,p2,p3)
    #define PLYNX_LOG4(p1,p2,p3,p4)
    #define PLYNX_LOG6(p1,p2,p3,p4,p5,p6)
    #define PLYNX_LOG_IP(msg, ip)
    #define PLYNX_LOG_IP_REV(msg, ip)
#endif

#ifndef PLYNX_DBG_BREAK
    #define PLYNX_DBG_BREAK()
    #define PLYNX_ASSERT(expr)
    #define PLYNX_DBG_DUMP(msg, addr, len)
#endif

#endif
