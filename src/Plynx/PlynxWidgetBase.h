/**
 * @file       PlynxWidgetBase.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 * @brief
 */

#ifndef PlynxWidgetBase_h
#define PlynxWidgetBase_h

#include <Plynx/PlynxApi.h>

/**
 * @brief What every widget object has in common, starting with the virtual
 * pin it talks to.
 *
 * WidgetLED, WidgetLCD, WidgetTerminal, WidgetMap and WidgetTable build on it,
 * so they all have setLabel(), setColor(), setMin() and setMax().
 *
 * @code
 * WidgetLED led(V3);
 *
 * void markWarning()
 * {
 *   led.setColor("#ED9D00");
 *   led.setLabel("Check the filter");
 * }
 * @endcode
 *
 * @see PlynxApi::setProperty
 */
class PlynxWidgetBase
{
public:
    /**
     * @brief Ties the widget object to a virtual pin.
     * @param vPin  The virtual pin, written V0, V1, V2 and so on.
     *
     * @code
     * WidgetLED led(V3);   // the LED widget in the app is set to V3
     * @endcode
     */
    PlynxWidgetBase(uint8_t vPin) : mPin(vPin) {}

    /**
     * @brief Moves the widget object to another virtual pin.
     * @param vPin  The new virtual pin.
     *
     * @code
     * WidgetLED led(V3);
     *
     * void useOtherPin()
     * {
     *   led.setVPin(V4);
     * }
     * @endcode
     */
    void setVPin(uint8_t vPin) { mPin = vPin; }

    void onWrite(PlynxReq PLYNX_UNUSED &request, const PlynxParam PLYNX_UNUSED &param) {
        PLYNX_LOG1(PLYNX_F("PlynxWidgetBase::onWrite should not be called"));
    }

    /**
     * @brief Changes the title the widget shows in the app.
     * @param args  The new title.
     *
     * @par App widget
     * The widget on this pin. It is Plynx.setProperty(pin, "label", ...).
     *
     * @code
     * WidgetLED pump(V3);
     *
     * PLYNX_CONNECTED()
     * {
     *   pump.setLabel("Pump");
     * }
     * @endcode
     *
     * @see PlynxApi::setProperty
     */
    template<typename... Args>
    void setLabel(Args... args) {
        Plynx.setProperty(mPin, "label", args...);
    }

    /**
     * @brief Changes the main color of the widget in the app.
     * @param args  The color, as "#RRGGBB" or as a number such as 0xFF0000.
     *
     * @code
     * WidgetLED alarm(V5);
     *
     * void alarmRed()
     * {
     *   alarm.setColor("#D3435C");
     * }
     * @endcode
     *
     * @see PlynxApi::setProperty
     */
    template<typename... Args>
    void setColor(Args... args) {
        Plynx.setProperty(mPin, "color", args...);
    }

    /**
     * @brief Changes the lowest value of the widget scale in the app.
     * @param args  The new minimum.
     *
     * @code
     * WidgetLED level(V6);
     *
     * PLYNX_CONNECTED()
     * {
     *   level.setMin(0);
     *   level.setMax(100);
     * }
     * @endcode
     *
     * @see setMax, PlynxApi::setProperty
     */
    template<typename... Args>
    void setMin(Args... args) {
        Plynx.setProperty(mPin, "min", args...);
    }

    /**
     * @brief Changes the highest value of the widget scale in the app.
     * @param args  The new maximum.
     *
     * @code
     * WidgetLED level(V6);
     *
     * void wideRange()
     * {
     *   level.setMax(1023);
     * }
     * @endcode
     *
     * @see setMin, PlynxApi::setProperty
     */
    template<typename... Args>
    void setMax(Args... args) {
        Plynx.setProperty(mPin, "max", args...);
    }

protected:
    uint8_t mPin;
};

class PlynxAttachWidgetHelper {
public:
    template<typename T>
    explicit PlynxAttachWidgetHelper(T& widget, uint8_t vPin) {
        widget.setVPin(vPin);
    }
};

// Could use __attribute__ ((constructor)), but hope for better portability

/**
 * @brief Connects a widget object to a virtual pin and passes it what the app
 * writes there.
 *
 * It sets the pin of the object and writes the PLYNX_WRITE() for that pin,
 * which calls the onWrite() of the object. WidgetTable uses it to receive the
 * rows the person selects or moves.
 *
 * @param widget  The widget object, created without a pin.
 * @param pin     The virtual pin, written V0, V1, V2 and so on.
 *
 * @par Where to call
 * At the top level of the sketch, after the object.
 *
 * @code
 * WidgetTable table;
 * PLYNX_ATTACH_WIDGET(table, V8);
 * @endcode
 *
 * @note It writes the PLYNX_WRITE() of that pin, so the sketch cannot write
 * its own for the same pin.
 *
 * @see WidgetTable
 */
#define PLYNX_ATTACH_WIDGET(widget, pin) \
    PlynxAttachWidgetHelper PLYNX_CONCAT2(blnk_widget_helper_, __COUNTER__)((widget), (pin)); \
    PLYNX_WRITE(pin) { (widget).onWrite(request, param); }

#endif
