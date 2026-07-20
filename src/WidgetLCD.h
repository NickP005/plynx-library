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

class WidgetLCD
    : public PlynxWidgetBase
{
public:
    WidgetLCD(uint8_t vPin) : PlynxWidgetBase(vPin) {}

    void clear() {
        Plynx.virtualWrite(mPin, "clr");
    }

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
