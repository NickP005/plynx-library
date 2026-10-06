/**
 * @file       WidgetTimeInput.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Aug 2016
 * @brief
 *
 */

#ifndef WidgetTimeInput_h
#define WidgetTimeInput_h

#include <Plynx/PlynxApi.h>
#include <utility/PlynxDateTime.h>
#include <utility/PlynxUtility.h>

/**
 * @brief Reads a start time, a stop time, a time zone and the days of the
 * week from the values of a PLYNX_WRITE().
 *
 * It decodes the format of the Time Input widget of Blynk-legacy apps. A
 * start or a stop can be a time of day, sunrise or sunset, or nothing.
 *
 * The Plynx app (October 2026) offers no Time Input widget. For a daily
 * schedule it has the Automation widget, which writes 1 and 0 to the pin at
 * the chosen times.
 *
 * @code
 * PLYNX_WRITE(V10)
 * {
 *   TimeInputParam t(param);
 *   if (t.hasStartTime()) {
 *     Serial.print(t.getStartHour());
 *     Serial.print(":");
 *     Serial.println(t.getStartMinute());
 *   }
 * }
 * @endcode
 */
class TimeInputParam
{
public:
    /** @brief What a start or a stop is. */
    enum TimeMode {
        TIME_UNDEFINED,   ///< Not set.
        TIME_SUNSET,      ///< At sunset.
        TIME_SUNRISE,     ///< At sunrise.
        TIME_SPECIFIED    ///< At a time of day.
    };

    /**
     * @brief Reads the values from `param`.
     * @param param  The `param` of the PLYNX_WRITE().
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     * }
     * @endcode
     */
    TimeInputParam(const PlynxParam& param)
    {
        mStartMode = TIME_UNDEFINED;
        mStopMode = TIME_UNDEFINED;
        mTZ[0] = '\0';
        mWeekdays = -1; // All set
        mTZ_Offset = 0;

        PlynxParam::iterator it = param.begin();
        if (it >= param.end())
            return;

        if (0 == strcmp(it.asStr(), "sr")) {
            mStartMode = TIME_SUNRISE;
        } else if (0 == strcmp(it.asStr(), "ss")) {
            mStartMode = TIME_SUNSET;
        } else if (!it.isEmpty()) {
            mStart = PlynxTime(it.asLong());
            if (mStart.isValid()) {
                mStartMode = TIME_SPECIFIED;
            }
        }

        if (++it >= param.end())
            return;

        if (0 == strcmp(it.asStr(), "sr")) {
            mStopMode = TIME_SUNRISE;
        } else if (0 == strcmp(it.asStr(), "ss")) {
            mStopMode = TIME_SUNSET;
        } else if (!it.isEmpty()) {
            mStop = PlynxTime(it.asLong());
            if (mStop.isValid()) {
                mStopMode = TIME_SPECIFIED;
            }
        }

        if (++it >= param.end())
            return;

        strncpy(mTZ, it.asStr(), sizeof(mTZ));

        if (++it >= param.end())
            return;

        if (!it.isEmpty()) {
            mWeekdays = 0;
            const char* p = it.asStr();

            while (int c = *p++) {
                if (c >= '1' && c <= '7') {
                    PlynxBitSet(mWeekdays, c - '1');
                }
            }
        }

        if (++it >= param.end())
            return;

        mTZ_Offset = it.asLong();
    }

    /**
     * @brief Gives the start time.
     * @return The start, as a PlynxTime. It is valid only when
     *         hasStartTime() is true.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     *   long secondsAfterMidnight = t.getStart().getUnixOffset();
     * }
     * @endcode
     */
    PlynxTime& getStart() { return mStart; }

    /**
     * @brief Gives the stop time.
     * @return The stop, as a PlynxTime. It is valid only when hasStopTime()
     *         is true.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     *   int stopHour = t.getStop().hour();
     * }
     * @endcode
     */
    PlynxTime& getStop()  { return mStop;  }

    /**
     * @brief Tells what the start is.
     * @return TIME_SPECIFIED, TIME_SUNRISE, TIME_SUNSET or TIME_UNDEFINED.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     *   if (t.getStartMode() == TimeInputParam::TIME_SUNSET) {
     *     Serial.println("starts at sunset");
     *   }
     * }
     * @endcode
     */
    TimeMode getStartMode() const { return mStartMode; }

    /**
     * @brief Tells what the stop is.
     * @return TIME_SPECIFIED, TIME_SUNRISE, TIME_SUNSET or TIME_UNDEFINED.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     *   bool noStop = t.getStopMode() == TimeInputParam::TIME_UNDEFINED;
     * }
     * @endcode
     */
    TimeMode getStopMode()  const { return mStopMode; }

    /**
     * @brief Tells whether the start is a time of day.
     * @return true when the start is a time of day.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     *   if (t.hasStartTime()) {
     *     Serial.println(t.getStartHour());
     *   }
     * }
     * @endcode
     */
    bool hasStartTime()   const { return mStartMode == TIME_SPECIFIED; }

    /**
     * @brief Tells whether the start is at sunrise.
     * @return true when the start is at sunrise.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   bool dawn = TimeInputParam(param).isStartSunrise();
     * }
     * @endcode
     */
    bool isStartSunrise() const { return mStartMode == TIME_SUNRISE; }

    /**
     * @brief Tells whether the start is at sunset.
     * @return true when the start is at sunset.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   bool dusk = TimeInputParam(param).isStartSunset();
     * }
     * @endcode
     */
    bool isStartSunset()  const { return mStartMode == TIME_SUNSET; }

    /**
     * @brief Gives the hour of the start, from 0 to 23.
     * @return The hour.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   int h = TimeInputParam(param).getStartHour();
     * }
     * @endcode
     */
    int getStartHour()    const { return mStart.hour(); }

    /**
     * @brief Gives the minute of the start, from 0 to 59.
     * @return The minute.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   int m = TimeInputParam(param).getStartMinute();
     * }
     * @endcode
     */
    int getStartMinute()  const { return mStart.minute(); }

    /**
     * @brief Gives the second of the start, from 0 to 59.
     * @return The second.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   int s = TimeInputParam(param).getStartSecond();
     * }
     * @endcode
     */
    int getStartSecond()  const { return mStart.second(); }

    /**
     * @brief Tells whether the stop is a time of day.
     * @return true when the stop is a time of day.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     *   if (t.hasStopTime()) {
     *     Serial.println(t.getStopHour());
     *   }
     * }
     * @endcode
     */
    bool hasStopTime()    const { return mStopMode == TIME_SPECIFIED; }

    /**
     * @brief Tells whether the stop is at sunrise.
     * @return true when the stop is at sunrise.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   bool dawn = TimeInputParam(param).isStopSunrise();
     * }
     * @endcode
     */
    bool isStopSunrise()  const { return mStopMode == TIME_SUNRISE; }

    /**
     * @brief Tells whether the stop is at sunset.
     * @return true when the stop is at sunset.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   bool dusk = TimeInputParam(param).isStopSunset();
     * }
     * @endcode
     */
    bool isStopSunset()   const { return mStopMode == TIME_SUNSET; }

    /**
     * @brief Gives the hour of the stop, from 0 to 23.
     * @return The hour.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   int h = TimeInputParam(param).getStopHour();
     * }
     * @endcode
     */
    int getStopHour()     const { return mStop.hour(); }

    /**
     * @brief Gives the minute of the stop, from 0 to 59.
     * @return The minute.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   int m = TimeInputParam(param).getStopMinute();
     * }
     * @endcode
     */
    int getStopMinute()   const { return mStop.minute(); }

    /**
     * @brief Gives the second of the stop, from 0 to 59.
     * @return The second.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   int s = TimeInputParam(param).getStopSecond();
     * }
     * @endcode
     */
    int getStopSecond()   const { return mStop.second(); }

    /**
     * @brief Gives the name of the time zone, such as "Europe/Rome".
     * @return The name, or an empty text.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   Serial.println(TimeInputParam(param).getTZ());
     * }
     * @endcode
     */
    const char* getTZ()   const { return mTZ; }

    /**
     * @brief Gives the offset of the time zone from UTC, in seconds.
     * @return The offset, 3600 for one hour ahead of UTC.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   long offset = TimeInputParam(param).getTZ_Offset();
     * }
     * @endcode
     */
    int32_t getTZ_Offset() const { return mTZ_Offset; }

    /**
     * @brief Tells whether a day of the week is chosen.
     *
     * @param day  The day, from 1 to 7, in the numbering of the widget (1 is
     *             Monday and 7 is Sunday).
     * @return true when the day is chosen. With no days in the value, every
     *         day is chosen.
     *
     * @code
     * PLYNX_WRITE(V10)
     * {
     *   TimeInputParam t(param);
     *   if (t.isWeekdaySelected(6) || t.isWeekdaySelected(7)) {
     *     Serial.println("weekend too");
     *   }
     * }
     * @endcode
     */
    bool isWeekdaySelected(int day) const {
        return PlynxBitRead(mWeekdays, (day - 1) % 7);
    }

private:
    PlynxTime mStart;
    PlynxTime mStop;
    char      mTZ[32];
    int32_t   mTZ_Offset;

    TimeMode  mStopMode;
    TimeMode  mStartMode;

    uint8_t   mWeekdays;
};

#endif
