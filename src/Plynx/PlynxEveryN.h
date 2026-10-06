/**
 * @file       PlynxEveryN.h
 * @license    This project is released under the MIT License (MIT)
 * @brief      Blocks that run every N milliseconds, seconds, minutes or
 *             hours, from inside loop(). Experimental, with PLYNX_EXPERIMENTAL.
 */

#ifndef PLYNXEVERYN_H
#define PLYNXEVERYN_H

#include <Plynx/PlynxDebug.h>

millis_time_t plynx_count_millis() {
    const millis_time_t ms = PlynxMillis();
    return ms;
}

uint16_t plynx_count_seconds16() {
    const millis_time_t ms = PlynxMillis();
    return (ms / 1000);
}

uint16_t plynx_count_minutes16()
{
    const millis_time_t ms = PlynxMillis();
    return (ms / (60000L)) & 0xFFFF;
}

uint8_t plynx_count_hours8()
{
    const millis_time_t ms = PlynxMillis();
    return (ms / (3600000L)) & 0xFF;
}

template<typename T, T (*timeGetter)()>
class PlynxPeriodic {
public:
    T mPrev;
    T mPeriod;

    PlynxPeriodic()           { reset(); mPeriod = 1; };
    PlynxPeriodic(T period)   { reset(); setPeriod(period); };
    void setPeriod( T period) { mPeriod = period; };
    T getTime()               { return (T)(timeGetter()); };
    T getPeriod()             { return mPeriod; };
    T getElapsed()            { return getTime() - mPrev; }
    T getRemaining()          { return mPeriod - getElapsed(); }
    T getLastTriggerTime()    { return mPrev; }
    bool ready() {
        bool isReady = (getElapsed() >= mPeriod);
        if( isReady ) { reset(); }
        return isReady;
    }
    void reset()              { mPrev = getTime(); };
    void trigger()            { mPrev = getTime() - mPeriod; };

    operator bool()           { return ready(); }
};

typedef PlynxPeriodic<millis_time_t,plynx_count_millis> PlynxEveryNMillis;
typedef PlynxPeriodic<uint16_t,plynx_count_seconds16>   PlynxEveryNSeconds;
typedef PlynxPeriodic<uint16_t,plynx_count_minutes16>   PlynxEveryNMinutes;
typedef PlynxPeriodic<uint8_t,plynx_count_hours8>       PlynxEveryNHours;

#define PLYNX_EVERY_N_MILLIS_I(NAME,N)  static PlynxEveryNMillis NAME(N); if(NAME)
#define PLYNX_EVERY_N_SECONDS_I(NAME,N) static PlynxEveryNSeconds NAME(N); if(NAME)
#define PLYNX_EVERY_N_MINUTES_I(NAME,N) static PlynxEveryNMinutes NAME(N); if(NAME)
#define PLYNX_EVERY_N_HOURS_I(NAME,N)   static PlynxEveryNHours NAME(N); if(NAME)

/**
 * @brief Runs the block that follows once every N milliseconds, from inside
 * loop().
 *
 * It needs PLYNX_EXPERIMENTAL above the board header.
 *
 * @experimental
 *
 * @param N  The interval in milliseconds.
 *
 * Call it inside loop(), or inside a function that loop() calls at every turn.
 *
 * @code
 * #define PLYNX_EXPERIMENTAL
 *
 * void loop()
 * {
 *   Plynx.run();
 *   PLYNX_EVERY_N_MILLIS(500) {
 *     Plynx.virtualWrite(V1, analogRead(A0));
 *   }
 * }
 * @endcode
 *
 * @see PlynxTimer, PLYNX_EVERY_N_SECONDS
 */
#define PLYNX_EVERY_N_MILLIS(N)  PLYNX_EVERY_N_MILLIS_I(PLYNX_CONCAT2(PER, __COUNTER__),N)

/**
 * @brief Runs the block that follows once every N seconds, from inside
 * loop().
 *
 * @experimental
 *
 * @param N  The interval in seconds.
 *
 * @code
 * #define PLYNX_EXPERIMENTAL
 *
 * void loop()
 * {
 *   Plynx.run();
 *   PLYNX_EVERY_N_SECONDS(10) {
 *     Plynx.virtualWrite(V2, millis() / 1000);
 *   }
 * }
 * @endcode
 *
 * @see PLYNX_EVERY_N_MILLIS
 */
#define PLYNX_EVERY_N_SECONDS(N) PLYNX_EVERY_N_SECONDS_I(PLYNX_CONCAT2(PER, __COUNTER__),N)

/**
 * @brief Runs the block that follows once every N minutes, from inside
 * loop().
 *
 * @experimental
 *
 * @param N  The interval in minutes.
 *
 * @code
 * #define PLYNX_EXPERIMENTAL
 *
 * void loop()
 * {
 *   Plynx.run();
 *   PLYNX_EVERY_N_MINUTES(5) {
 *     Plynx.syncAll();
 *   }
 * }
 * @endcode
 *
 * @see PLYNX_EVERY_N_MILLIS
 */
#define PLYNX_EVERY_N_MINUTES(N) PLYNX_EVERY_N_MINUTES_I(PLYNX_CONCAT2(PER, __COUNTER__),N)

/**
 * @brief Runs the block that follows once every N hours, from inside loop().
 *
 * @experimental
 *
 * @param N  The interval in hours.
 *
 * @code
 * #define PLYNX_EXPERIMENTAL
 *
 * void loop()
 * {
 *   Plynx.run();
 *   PLYNX_EVERY_N_HOURS(1) {
 *     Plynx.logEvent("hourly", "Still running");
 *   }
 * }
 * @endcode
 *
 * @see PLYNX_EVERY_N_MILLIS
 */
#define PLYNX_EVERY_N_HOURS(N)   PLYNX_EVERY_N_HOURS_I(PLYNX_CONCAT2(PER, __COUNTER__),N)

#endif
