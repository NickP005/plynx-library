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
 * The LED lights up for any value above 0. With brightness on in the widget, 0
 * to 255 sets how bright. Change it when the state changes, not at every turn
 * of loop().
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
     * @brief The LED on the virtual pin of your LED widget.
     * @param vPin  The virtual pin, such as V3.
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
     * @brief The last value the sketch gave this LED, from 0 to 255. It starts at 0.
     * @return The value.
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
     * @note A change made from the app is not counted here.
     */
    uint8_t getValue() const {
        return mValue;
    }

    /**
     * @brief Sets how bright the LED widget glows.
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
