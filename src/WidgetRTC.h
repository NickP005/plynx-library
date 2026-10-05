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

/**
 * @brief Gives the board the current date and time, taken from the server in
 * the time zone of the RTC widget.
 *
 * It sets the clock of the Time library (TimeLib), so after the first answer
 * the sketch reads the time with hour(), minute(), day(), year() and the
 * other functions of that library. Five minutes after each answer, the next
 * time the sketch reads the time, the Time library asks the server again by
 * itself.
 *
 * It needs the Time library, installed from the Library Manager, and
 * `#include <WidgetRTC.h>` after the board header.
 *
 * @par Where to call
 * Create it at the top level and call begin() inside PLYNX_CONNECTED().
 *
 * @par App widget
 * RTC, where the time zone is chosen. Without an RTC widget in the project
 * the server sends no time.
 *
 * @code
 * #include <PlynxSimpleEsp32.h>
 * #include <TimeLib.h>
 * #include <WidgetRTC.h>
 *
 * WidgetRTC rtc;
 *
 * PLYNX_CONNECTED()
 * {
 *   rtc.begin();
 * }
 *
 * void printTime()
 * {
 *   if (timeStatus() != timeNotSet) {
 *     Serial.print(hour());
 *     Serial.print(".");
 *     Serial.println(minute());
 *   }
 * }
 * @endcode
 *
 * @warning The time arrives a moment after begin(), during Plynx.run().
 * Until then year() says 1970. Check timeStatus() first.
 *
 * @see begin, PLYNX_CONNECTED
 */
class WidgetRTC
    : public PlynxSingleton<WidgetRTC>
{
public:
    /**
     * @brief Creates the clock object. Create only one.
     *
     * @code
     * #include <PlynxSimpleEsp32.h>
     * #include <TimeLib.h>
     * #include <WidgetRTC.h>
     *
     * WidgetRTC rtc;
     * @endcode
     */
    WidgetRTC() {}

    /**
     * @brief Asks the server for the time, and makes the Time library ask
     * again by itself from now on.
     *
     * @par Where to call
     * Inside PLYNX_CONNECTED(). Called before the board is connected, the
     * request is lost. The next one leaves the first time the sketch reads
     * the time after 5 minutes.
     *
     * @code
     * #include <PlynxSimpleEsp32.h>
     * #include <TimeLib.h>
     * #include <WidgetRTC.h>
     *
     * WidgetRTC rtc;
     *
     * PLYNX_CONNECTED()
     * {
     *   rtc.begin();
     *   setSyncInterval(10 * 60);   // then every 10 minutes
     * }
     * @endcode
     */
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
