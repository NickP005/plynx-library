/**
 * @file       WidgetLED.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Apr 2015
 * @brief
 */

#ifndef WidgetLED_h
#define WidgetLED_h

#include <Plynx/PlynxWidgetBase.h>

/**
 * @brief Turns the LED widget of the app on and off, or sets how bright it
 * glows.
 *
 * Each call sends the value to the pin with Plynx.virtualWrite(). The LED
 * widget lights up for any value above 0. With brightness turned on in the
 * widget, a value from 0 to 255 sets how bright it glows.
 *
 * @par Where to call
 * Create it at the top level with the pin of the LED widget. Change it where
 * the state changes, never at every turn of loop().
 *
 * @par App widget
 * LED.
 *
 * @code
 * WidgetLED pumpLed(V3);
 *
 * PLYNX_WRITE(V1)
 * {
 *   int on = param.asInt();
 *   digitalWrite(4, on);
 *   if (on) {
 *     pumpLed.on();
 *   } else {
 *     pumpLed.off();
 *   }
 * }
 * @endcode
 *
 * @see PlynxWidgetBase::setColor
 */
class WidgetLED
    : public PlynxWidgetBase
{
public:
    /**
     * @brief Ties the LED object to the virtual pin of the LED widget.
     * @param vPin  The virtual pin, written V0, V1, V2 and so on.
     *
     * @code
     * WidgetLED led(V3);
     * @endcode
     */
    WidgetLED(uint8_t vPin)
        : PlynxWidgetBase(vPin)
        , mValue(0)
    {}

    /**
     * @brief Gives the last value set on this board.
     * @return The value, from 0 to 255. It starts at 0.
     *
     * @code
     * WidgetLED led(V3);
     *
     * void flip()
     * {
     *   if (led.getValue()) {
     *     led.off();
     *   } else {
     *     led.on();
     *   }
     * }
     * @endcode
     *
     * @note It remembers what the sketch set, and knows nothing about the app.
     */
    uint8_t getValue() const {
        return mValue;
    }

    /**
     * @brief Sets the brightness of the LED widget.
     * @param value  From 0 (off) to 255 (full).
     *
     * @code
     * WidgetLED led(V3);
     *
     * void showLight()
     * {
     *   led.setValue(analogRead(A0) / 4);   // 0 to 1023 becomes 0 to 255
     * }
     * @endcode
     *
     * @see on, off
     */
    void setValue(uint8_t value) {
        mValue = value;
        Plynx.virtualWrite(mPin, value);
    }

    /**
     * @brief Turns the LED widget on, at full brightness (255).
     *
     * @code
     * WidgetLED led(V3);
     *
     * PLYNX_CONNECTED()
     * {
     *   led.on();
     * }
     * @endcode
     *
     * @see off, setValue
     */
    void on() {
        setValue(255);
    }

    /**
     * @brief Turns the LED widget off (0).
     *
     * @code
     * WidgetLED led(V3);
     *
     * void allOff()
     * {
     *   led.off();
     * }
     * @endcode
     *
     * @see on, setValue
     */
    void off() {
        setValue(0);
    }

private:
    uint8_t mValue;
};

#endif
