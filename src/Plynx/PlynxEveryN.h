
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

#define PLYNX_EVERY_N_MILLIS(N)  PLYNX_EVERY_N_MILLIS_I(PLYNX_CONCAT2(PER, __COUNTER__),N)
#define PLYNX_EVERY_N_SECONDS(N) PLYNX_EVERY_N_SECONDS_I(PLYNX_CONCAT2(PER, __COUNTER__),N)
#define PLYNX_EVERY_N_MINUTES(N) PLYNX_EVERY_N_MINUTES_I(PLYNX_CONCAT2(PER, __COUNTER__),N)
#define PLYNX_EVERY_N_HOURS(N)   PLYNX_EVERY_N_HOURS_I(PLYNX_CONCAT2(PER, __COUNTER__),N)

#endif
