/**
 * @file       WidgetGPS.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Oct 2016
 * @brief
 *
 */

#ifndef WidgetGPS_h
#define WidgetGPS_h

#ifndef PLYNX_NO_FLOAT

#include <Plynx/PlynxWidgetBase.h>

/**
 * @brief Reads a position written as latitude, longitude, altitude and speed
 * from the values of a PLYNX_WRITE().
 *
 * It takes the four values in that order. A value that is missing reads as 0.
 *
 * @par App widget
 * It reads the format of the GPS Stream widget of Blynk-legacy apps. The
 * Plynx app (October 2026) offers no such widget, so in a Plynx project the
 * values come from a sketch, for example through WidgetBridge.
 *
 * @code
 * PLYNX_WRITE(V12)
 * {
 *   GpsParam gps(param);
 *   Serial.print(gps.getLat(), 6);
 *   Serial.print(" ");
 *   Serial.println(gps.getLon(), 6);
 * }
 * @endcode
 *
 * @see WidgetMap
 */
class GpsParam
{
public:

    /**
     * @brief Reads the four values from `param`.
     * @param param  The `param` of the PLYNX_WRITE().
     *
     * @code
     * PLYNX_WRITE(V12)
     * {
     *   GpsParam gps(param);
     * }
     * @endcode
     */
    GpsParam(const PlynxParam& param)
        : mLat (0)
        , mLon (0)
        , mAlt (0)
        , mSpeed (0)
    {
        PlynxParam::iterator it = param.begin();
        if (it >= param.end())
            return;

        mLat = it.asDouble();

        if (++it >= param.end())
            return;

        mLon = it.asDouble();

        if (++it >= param.end())
            return;

        mAlt = it.asDouble();

        if (++it >= param.end())
            return;

        mSpeed = it.asDouble();
    }


    /**
     * @brief Gives the latitude.
     * @return The first value, in degrees.
     *
     * @code
     * PLYNX_WRITE(V12)
     * {
     *   double lat = GpsParam(param).getLat();
     * }
     * @endcode
     */
    double getLat() const { return mLat; }

    /**
     * @brief Gives the longitude.
     * @return The second value, in degrees.
     *
     * @code
     * PLYNX_WRITE(V12)
     * {
     *   double lon = GpsParam(param).getLon();
     * }
     * @endcode
     */
    double getLon() const { return mLon; }

    /**
     * @brief Gives the altitude.
     * @return The third value, in meters.
     *
     * @code
     * PLYNX_WRITE(V12)
     * {
     *   double meters = GpsParam(param).getAltitude();
     * }
     * @endcode
     */
    double getAltitude() const { return mAlt; }

    /**
     * @brief Gives the speed.
     * @return The fourth value, in the unit of the sender.
     *
     * @code
     * PLYNX_WRITE(V12)
     * {
     *   double speed = GpsParam(param).getSpeed();
     * }
     * @endcode
     */
    double getSpeed() const { return mSpeed; }

private:
    double mLat;
    double mLon;
    double mAlt;
    double mSpeed;
};

#endif

#endif
