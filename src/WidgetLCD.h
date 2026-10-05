/**
 * @file       WidgetLCD.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 */

#ifndef WidgetLCD_h
#define WidgetLCD_h

#include <Plynx/PlynxWidgetBase.h>

/**
 * @brief Writes text at a position on the LCD widget of the app, a screen of
 * 2 lines of 16 characters.
 *
 * Each print() writes over the characters already on the screen, like a real
 * LCD, and what goes past the end of the line is lost.
 *
 * @par Where to call
 * Create it at the top level with the pin of the LCD widget. Print from a
 * timer function or from a handler.
 *
 * @par App widget
 * LCD.
 *
 * @code
 * WidgetLCD lcd(V4);
 * PlynxTimer timer;
 *
 * void showTemperature()
 * {
 *   lcd.clear();
 *   lcd.print(0, 0, "Temperature");
 *   lcd.print(0, 1, String(analogRead(A0) * 0.1, 1));   // one decimal
 * }
 *
 * void setup()
 * {
 *   // Plynx.begin(...) as in the example for your board
 *   timer.setInterval(5000L, showTemperature);
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 *   timer.run();
 * }
 * @endcode
 *
 * @see print, clear
 */
class WidgetLCD
    : public PlynxWidgetBase
{
public:
    /**
     * @brief Ties the LCD object to the virtual pin of the LCD widget.
     * @param vPin  The virtual pin, written V0, V1, V2 and so on.
     *
     * @code
     * WidgetLCD lcd(V4);
     * @endcode
     */
    WidgetLCD(uint8_t vPin) : PlynxWidgetBase(vPin) {}

    /**
     * @brief Empties the screen of the LCD widget.
     *
     * @code
     * WidgetLCD lcd(V4);
     *
     * PLYNX_CONNECTED()
     * {
     *   lcd.clear();
     * }
     * @endcode
     *
     * @see print
     */
    void clear() {
        Plynx.virtualWrite(mPin, "clr");
    }

    /**
     * @brief Writes a text or a number on the LCD widget, starting at a
     * column and a line.
     *
     * @param x    The column, from 0 to 15.
     * @param y    The line, 0 for the top one and 1 for the bottom one.
     * @param str  What to write, a text or a number.
     *
     * @code
     * WidgetLCD lcd(V4);
     *
     * void showCounter(int count)
     * {
     *   lcd.print(0, 0, "Visitors");
     *   lcd.print(10, 0, count);
     * }
     * @endcode
     *
     * @warning A shorter text leaves the old characters after it on the
     * screen. Print spaces over them, or call clear() first.
     *
     * @see clear
     */
    template<typename T>
    void print(int x, int y, const T& str) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add("p");
        cmd.add(x);
        cmd.add(y);
        cmd.add(str);
        Plynx.virtualWrite(mPin, cmd);
    }

};

#endif
