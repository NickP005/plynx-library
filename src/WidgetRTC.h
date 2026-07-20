/**
 * @file       WidgetRTC.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2016
 * @brief
 *
 */

#ifndef WidgetRTC_h
#define WidgetRTC_h

#if !defined(ARDUINO)
    #error WidgetRTC is not available on this platform!
#endif

#include <Plynx/PlynxWidgetBase.h>
#include <Plynx/PlynxTemplates.h>
#include <TimeLib.h>

class WidgetRTC
    : public PlynxSingleton<WidgetRTC>
{
public:
    WidgetRTC() {}
    void begin();

private:
    static time_t requestTimeSync();
};

// This is called by Time library when it needs time sync
time_t WidgetRTC::requestTimeSync()
{
    // Request RTC widget update from the server
    Plynx.sendInternal("rtc", "sync");
    // Tell the Time library that we'll set it later
    return 0;
}

inline
void WidgetRTC::begin()
{
    setSyncProvider(requestTimeSync);
}

PLYNX_WRITE(InternalPinRTC) {
    const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
    unsigned long plynxTime = param.asLong();

    if (plynxTime >= DEFAULT_TIME) {    // Check the integer is a valid time (greater than Jan 1 2013)
        setTime(plynxTime);             // Sync Time library clock to the value received from Plynx
        PLYNX_LOG1(PLYNX_F("Time sync: OK"));
    }
}

#endif
