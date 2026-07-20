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

class PlynxWidgetBase
{
public:
    PlynxWidgetBase(uint8_t vPin) : mPin(vPin) {}
    void setVPin(uint8_t vPin) { mPin = vPin; }

    void onWrite(PlynxReq PLYNX_UNUSED &request, const PlynxParam PLYNX_UNUSED &param) {
        PLYNX_LOG1(PLYNX_F("PlynxWidgetBase::onWrite should not be called"));
    }

    template<typename... Args>
    void setLabel(Args... args) {
        Plynx.setProperty(mPin, "label", args...);
    }

    template<typename... Args>
    void setColor(Args... args) {
        Plynx.setProperty(mPin, "color", args...);
    }

    template<typename... Args>
    void setMin(Args... args) {
        Plynx.setProperty(mPin, "min", args...);
    }

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
#define PLYNX_ATTACH_WIDGET(widget, pin) \
    PlynxAttachWidgetHelper PLYNX_CONCAT2(blnk_widget_helper_, __COUNTER__)((widget), (pin)); \
    PLYNX_WRITE(pin) { (widget).onWrite(request, param); }

#endif
