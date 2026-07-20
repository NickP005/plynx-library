/**
 * @file       PlynxDebug.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Debug utilities for Arduino
 */
#include <Plynx/PlynxDebug.h>

#if defined(ARDUINO) && defined(__AVR__) && defined(PLYNX_USE_AVR_WDT)

    #include <Arduino.h>
    #include <avr/wdt.h>

    PLYNX_CONSTRUCTOR
    static void PlynxSystemInit()
    {
        MCUSR = 0;
        wdt_disable();
    }

    void PlynxReset()
    {
        wdt_enable(WDTO_15MS);
        delay(50);
        void(*resetFunc)(void) = 0;
        resetFunc();
        for(;;) {} // To make compiler happy
    }

    size_t PlynxFreeRam()
    {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined(ARDUINO) && defined(__AVR__)

    #include <Arduino.h>

    void PlynxReset()
    {
        void(*resetFunc)(void) = 0;
        resetFunc();
        for(;;) {}
    }

    size_t PlynxFreeRam()
    {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined(ARDUINO) && defined(ESP8266)

    #include <Arduino.h>

    size_t PlynxFreeRam()
    {
        return ESP.getFreeHeap();
    }

    void PlynxReset()
    {
        ESP.restart();
        for(;;) {}
    }

    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)

    #include <Arduino.h>

    extern "C" char *sbrk(int i);

    size_t PlynxFreeRam()
    {
        char stack_dummy = 0;
        return &stack_dummy - sbrk(0);
    }

    void PlynxReset()
    {
        NVIC_SystemReset();
        for(;;) {}
    }

    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined (ARDUINO_ARCH_ARC32)

    millis_time_t PlynxMillis()
    {
        // TODO: Remove workaround for Intel Curie
        // https://forum.arduino.cc/index.php?topic=391836.0
        noInterrupts();
        uint64_t t = millis();
        interrupts();
        return t;
    }

    #define _PLYNX_USE_DEFAULT_FREE_RAM
    #define _PLYNX_USE_DEFAULT_RESET
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined(ARDUINO) && (defined(__STM32F1__) || defined(__STM32F3__))

    #include <Arduino.h>
    #include <libmaple/nvic.h>

    void PlynxReset()
    {
        nvic_sys_reset();
        for(;;) {}
    }

    #define _PLYNX_USE_DEFAULT_FREE_RAM
    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined (PARTICLE) || defined(SPARK)

    #include "application.h"

    void PlynxReset()
    {
        System.reset();
        for(;;) {} // To make compiler happy
    }

    #define _PLYNX_USE_DEFAULT_FREE_RAM
    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined(__MBED__)

    #include "mbed.h"

    static Timer  plynx_millis_timer;
    static Ticker plynx_waker;

    static
    void plynx_wake() {
        //pc.puts("(...)");
    }

    PLYNX_CONSTRUCTOR
    static void PlynxSystemInit()
    {
        plynx_waker.attach(&plynx_wake, 2.0);
        plynx_millis_timer.start();
    }

    void PlynxDelay(millis_time_t ms)
    {
        wait_ms(ms);
    }

    millis_time_t PlynxMillis()
    {
        return plynx_millis_timer.read_ms();
    }

    #define _PLYNX_USE_DEFAULT_FREE_RAM
    #define _PLYNX_USE_DEFAULT_RESET

#elif defined(LINUX) && defined(RASPBERRY)

    #include <stdlib.h>
    #include <wiringPi.h>

    PLYNX_CONSTRUCTOR
    static void PlynxSystemInit()
    {
        wiringPiSetupGpio();
    }

    void PlynxReset()
    {
        exit(1);
        for(;;) {} // To make compiler happy
    }

    #define _PLYNX_USE_DEFAULT_FREE_RAM
    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#elif defined(LINUX)

    #define _POSIX_C_SOURCE 200809L
    #include <stdlib.h>
    #include <time.h>
    #include <unistd.h>

    static millis_time_t plynx_startup_time = 0;

    PLYNX_CONSTRUCTOR
    static void PlynxSystemInit()
    {
        plynx_startup_time = PlynxMillis();
    }

    void PlynxReset()
    {
        exit(1);
        for(;;) {} // To make compiler happy
    }

    void PlynxDelay(millis_time_t ms)
    {
        usleep(ms * 1000);
    }

    millis_time_t PlynxMillis()
    {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts );
        return ( ts.tv_sec * 1000 + ts.tv_nsec / 1000000L ) - plynx_startup_time;
    }

    #define _PLYNX_USE_DEFAULT_FREE_RAM

#elif defined(TI_CC3220)

    #include <string.h>
    #include <stdlib.h>
    #include <stdarg.h>
    #include <unistd.h>

    #include <ti/devices/cc32xx/inc/hw_types.h>

    #include <ti/sysbios/knl/Clock.h>
    #include <ti/drivers/net/wifi/device.h>
    #include <ti/devices/cc32xx/driverlib/prcm.h>

    void PlynxReset()
    {
	    sl_Stop(200);
	    for(;;) {
    	    PRCMHibernateCycleTrigger();
        }
    }

    void PlynxDelay(millis_time_t ms)
    {
        usleep(ms * 1000);
    }

    millis_time_t PlynxMillis()
    {
        return Clock_getTicks();
    }

    #define _PLYNX_USE_DEFAULT_FREE_RAM

#else

    #if defined(PLYNX_DEBUG_ALL)
        #warning "Need to implement board-specific utilities"
    #endif

    #define _PLYNX_USE_DEFAULT_FREE_RAM
    #define _PLYNX_USE_DEFAULT_RESET
    #define _PLYNX_USE_DEFAULT_MILLIS
    #define _PLYNX_USE_DEFAULT_DELAY

#endif

#ifdef _PLYNX_USE_DEFAULT_DELAY
    void PlynxDelay(millis_time_t ms)
    {
        return delay(ms);
    }
#endif

#ifdef _PLYNX_USE_DEFAULT_MILLIS
    millis_time_t PlynxMillis()
    {
        return millis();
    }
#endif

#ifdef _PLYNX_USE_DEFAULT_FREE_RAM
    size_t PlynxFreeRam()
    {
        return 0;
    }
#endif

#ifdef _PLYNX_USE_DEFAULT_RESET
    void PlynxReset()
    {
        for(;;) {} // To make compiler happy
    }
#endif

void PlynxFatal()
{
    PlynxDelay(10000L);
    PlynxReset();
}

