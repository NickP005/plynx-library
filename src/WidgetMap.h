/**
 * @file       WidgetMap.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 * @brief
 */

#ifndef WidgetMap_h
#define WidgetMap_h

#include <Plynx/PlynxWidgetBase.h>

/**
 * @brief Puts a point on the Map widget of the app, from coordinates the
 * board knows.
 *
 * Create it at the top level with the pin of the Map widget. Send a location
 * when it changes, for example when a GPS module gets a new fix.
 *
 * The app shows a marker at the last location received.
 *
 * @code
 * WidgetMap myMap(V7);
 *
 * PLYNX_CONNECTED()
 * {
 *   myMap.location(1, 45.4642, 9.1900, "Milan");
 * }
 * @endcode
 *
 * @see location, clear
 */
class WidgetMap
    : public PlynxWidgetBase
{
public:
    /**
     * @brief Ties the map object to the virtual pin of the Map widget.
     * @param vPin  The virtual pin, such as V7.
     *
     * @code
     * WidgetMap myMap(V7);
     * @endcode
     */
    WidgetMap(uint8_t vPin) : PlynxWidgetBase(vPin) {}

    /**
     * @brief Removes the point from the Map widget.
     *
     * @code
     * WidgetMap myMap(V7);
     *
     * void forgetPosition()
     * {
     *   myMap.clear();
     * }
     * @endcode
     *
     * @see location
     */
    void clear() {
        Plynx.virtualWrite(mPin, "clr");
    }

    /**
     * @brief Sends a location to the Map widget.
     *
     * @param index  A number for the point, 1 for a single tracker.
     * @param lat    The latitude, from -90 to 90.
     * @param lon    The longitude, from -180 to 180.
     * @param value  The title of the marker, a text or a number.
     *
     * The app shows one marker, at the last location, titled with `value`. It
     * skips the location 0, 0.
     *
     * @code
     * WidgetMap myMap(V7);
     *
     * void sendPosition(float latitude, float longitude)
     * {
     *   myMap.location(1, latitude, longitude, "Bike");
     * }
     * @endcode
     *
     * @note A `float` keeps 3 decimals, about 100 meters. Pass a `double` for
     * a more precise point.
     *
     * @see clear
     */
    template<typename T1, typename T2, typename T3, typename T4>
    void location(const T1& index, const T2& lat, const T3& lon, const T4& value) {
        Plynx.virtualWrite(mPin, index, lat, lon, value);
    }

};

#endif
