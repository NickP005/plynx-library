/**
 * @file       PlynxDateTime.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Aug 2016
 * @brief      DateTime implementation
 *
 */

#ifndef PlynxDateTime_h
#define PlynxDateTime_h

typedef long plynx_time_t;

struct plynx_tm *plynx_gmtime_r(const plynx_time_t *time, struct plynx_tm *tm);
plynx_time_t plynx_mk_gmtime(struct plynx_tm *tm);

struct plynx_tm {
    int8_t          tm_sec;
    int8_t          tm_min;
    int8_t          tm_hour;
    int8_t          tm_mday;
    int8_t          tm_wday;
    int8_t          tm_mon;
    int16_t         tm_year;
    int16_t         tm_yday;
    int16_t         tm_isdst;
};

/**
 * @brief A time of day, from 00:00:00 to 23:59:59, kept as the seconds after
 * midnight.
 *
 * TimeInputParam gives the start and the stop in this form.
 *
 * @code
 * void showTime()
 * {
 *   PlynxTime t(7, 30, 0);   // 07:30:00
 *   Serial.print(t.hour());
 *   Serial.print(":");
 *   Serial.println(t.minute());
 * }
 * @endcode
 *
 * @see TimeInputParam
 */
class PlynxTime {

public:
    /** @brief The seconds in a day, 86400. */
    static const uint32_t MAX_TIME = 86400L;

    static PlynxTime invalid() { return PlynxTime(); }

    /**
     * @brief Creates a time that is not set. isValid() is false.
     *
     * @code
     * void unset()
     * {
     *   PlynxTime t;
     *   Serial.println(t.isValid());   // 0
     * }
     * @endcode
     */
    PlynxTime() : mTime(-1) {}

    PlynxTime(const PlynxTime& t) : mTime(t.mTime) {}

    /**
     * @brief Creates a time from the seconds after midnight.
     * @param seconds  The seconds after midnight. A larger number wraps
     *                 around the day.
     *
     * @code
     * void noon()
     * {
     *   PlynxTime t(12L * 3600);
     *   Serial.println(t.hour());      // 12
     * }
     * @endcode
     */
    PlynxTime(long seconds) : mTime(seconds % MAX_TIME) {}

    /**
     * @brief Creates a time from hours, minutes and seconds.
     * @param hour    From 0 to 23.
     * @param minute  From 0 to 59.
     * @param second  From 0 to 59.
     *
     * @code
     * void alarmTime()
     * {
     *   PlynxTime alarm(6, 45, 0);
     * }
     * @endcode
     *
     * @warning On AVR boards, where an `int` has 16 bits, an hour from 10 up
     * overflows and gives a wrong time. There, pass the seconds as a `long`,
     * as in `PlynxTime(22L * 3600)`.
     */
    PlynxTime(int hour, int minute, int second)
    {
        mTime = (hour * 3600 + minute * 60 + second) % MAX_TIME;
    }

    /**
     * @brief Gives the seconds, from 0 to 59.
     * @return The seconds.
     *
     * @code
     * void showSecond(PlynxTime t)
     * {
     *   Serial.println(t.second());
     * }
     * @endcode
     */
    int second() const { return mTime % 60; }

    /**
     * @brief Gives the minutes, from 0 to 59.
     * @return The minutes.
     *
     * @code
     * void showMinute(PlynxTime t)
     * {
     *   Serial.println(t.minute());
     * }
     * @endcode
     */
    int minute() const { return (mTime / 60) % 60; }

    /**
     * @brief Gives the hour, from 0 to 23.
     * @return The hour.
     *
     * @code
     * void showHour(PlynxTime t)
     * {
     *   Serial.println(t.hour());
     * }
     * @endcode
     */
    int hour()   const { return mTime / 3600; }

    /**
     * @brief Gives the hour on a 12-hour clock, from 1 to 12.
     * @return The hour, 12 for midnight and for noon.
     *
     * @code
     * void showClock(PlynxTime t)
     * {
     *   Serial.print(t.hour12());
     *   Serial.println(t.isPM() ? " PM" : " AM");
     * }
     * @endcode
     *
     * @see isAM, isPM
     */
    int hour12() const {
        int h = hour();
        if (h == 0)
            return 12; // 12 midnight
        else if (h > 12)
            return h - 12;
        return h;
    }

    /**
     * @brief Tells whether the time is before noon.
     * @return true from 00:00 to 11:59.
     *
     * @code
     * void greet(PlynxTime t)
     * {
     *   Serial.println(t.isAM() ? "Good morning" : "Good afternoon");
     * }
     * @endcode
     */
    bool isAM() const { return !isPM(); }

    /**
     * @brief Tells whether the time is noon or later.
     * @return true from 12:00 to 23:59.
     *
     * @code
     * void evening(PlynxTime t)
     * {
     *   bool late = t.isPM();
     * }
     * @endcode
     */
    bool isPM() const { return (hour() >= 12); }

    /**
     * @brief Moves the time forward by a number of seconds, wrapping around
     * midnight.
     * @param sec  The seconds to add.
     *
     * @code
     * void later()
     * {
     *   PlynxTime t(23L * 3600 + 59 * 60);   // 23:59:00
     *   t.adjustSeconds(120);                 // 00:01:00
     * }
     * @endcode
     *
     * @warning A negative number that goes back past midnight gives a wrong
     * time.
     */
    void adjustSeconds(int sec) {
        if (isValid()) {
            mTime = (mTime + sec) % MAX_TIME;
        }
    }

    /**
     * @brief Gives the time as seconds after midnight.
     * @return From 0 to 86399.
     *
     * @code
     * void compare(PlynxTime start, PlynxTime stop)
     * {
     *   long length = stop.getUnixOffset() - start.getUnixOffset();
     * }
     * @endcode
     */
    plynx_time_t getUnixOffset() const { return mTime; }

    /**
     * @brief Tells whether the time is set.
     * @return true for a time of day, false for a time that is not set.
     *
     * @code
     * void check(PlynxTime t)
     * {
     *   if (t.isValid()) {
     *     Serial.println(t.hour());
     *   }
     * }
     * @endcode
     */
    bool isValid()  const { return mTime < MAX_TIME; }
    operator bool() const { return isValid(); }

    bool operator != (const PlynxTime& t) const { return mTime != t.mTime; }
    bool operator == (const PlynxTime& t) const { return mTime == t.mTime; }
    bool operator >= (const PlynxTime& t) const { return mTime >= t.mTime; }
    bool operator <= (const PlynxTime& t) const { return mTime <= t.mTime; }
    bool operator >  (const PlynxTime& t) const { return mTime >  t.mTime; }
    bool operator <  (const PlynxTime& t) const { return mTime <  t.mTime; }

private:
    uint32_t mTime;
};

class PlynxDateTime {

public:
    PlynxDateTime() : mTime(0) {}

    PlynxDateTime(const PlynxDateTime& t)
    {
        mTime = t.mTime;
        plynx_gmtime_r(&mTime, &mTm);
    }

    PlynxDateTime(plynx_time_t t)
    {
        mTime = t;
        plynx_gmtime_r(&mTime, &mTm);
    }

    PlynxDateTime(int hour, int minute, int second, int day, int month, int year)
    {
        mTm.tm_hour = hour;
        mTm.tm_min  = minute;
        mTm.tm_sec  = second;

        mTm.tm_mday = day;
        mTm.tm_mon  = month - 1;
        mTm.tm_year = year - 1900;

        mTm.tm_isdst = 0;

        mTime = plynx_mk_gmtime(&mTm);
    }

    int second() const { return mTm.tm_sec; }
    int minute() const { return mTm.tm_min; }
    int hour()   const { return mTm.tm_hour; }
    int day()    const { return mTm.tm_mday; }
    int month()  const { return 1 + mTm.tm_mon; }
    int year()   const { return 1900 + mTm.tm_year; }

    int day_of_year() const { return 1 + mTm.tm_yday; }
    int day_of_week() const { return mTm.tm_wday == 0 ? 7 : mTm.tm_wday; }

    /*int weak_of_year() const {
        int julian = day_of_year();
        int dow = day_of_week();
        int dowJan1 = PlynxDateTime(0,0,0, 1,1,year()).day_of_week();
        int weekNum = ((julian + 6) / 7);
        if (dow < dowJan1)
            ++weekNum;
        return (weekNum);
    }*/

    int hour12() const {
        int h = hour();
        if (h == 0)
            return 12; // 12 midnight
        else if (h > 12)
            return h - 12;
        return h;
    }

    bool isAM() const { return !isPM(); }
    bool isPM() const { return (hour() >= 12); }

    void adjustSeconds(int sec) {
        if (isValid()) {
            mTime += sec;
            plynx_gmtime_r(&mTime, &mTm);
        }
    }

    //tm& getTm() { return mTm; }
    plynx_time_t getUnix() const { return mTime; }

    bool isValid()  const { return mTime != 0; }
    operator bool() const { return isValid(); }

    bool operator != (const PlynxDateTime& t) const { return mTime != t.mTime; }
    bool operator == (const PlynxDateTime& t) const { return mTime == t.mTime; }
    bool operator >= (const PlynxDateTime& t) const { return mTime >= t.mTime; }
    bool operator <= (const PlynxDateTime& t) const { return mTime <= t.mTime; }
    bool operator >  (const PlynxDateTime& t) const { return mTime >  t.mTime; }
    bool operator <  (const PlynxDateTime& t) const { return mTime <  t.mTime; }

private:
    plynx_tm mTm;
    plynx_time_t mTime;
};


#endif
