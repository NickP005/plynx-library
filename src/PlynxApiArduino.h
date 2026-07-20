/**
 * @file       PlynxApiArduino.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef PlynxApiArduino_h
#define PlynxApiArduino_h

#include <Plynx/PlynxApi.h>
#include <Arduino.h>

template<class Proto>
PLYNX_FORCE_INLINE
void PlynxApi<Proto>::sendInfo()
{
    static const char profile[] PLYNX_PROGMEM = "blnkinf\0"
        PLYNX_PARAM_KV("ver"    , PLYNX_VERSION)
        PLYNX_PARAM_KV("h-beat" , PLYNX_TOSTRING(PLYNX_HEARTBEAT))
        PLYNX_PARAM_KV("buff-in", PLYNX_TOSTRING(PLYNX_MAX_READBYTES))
#ifdef PLYNX_INFO_DEVICE
        PLYNX_PARAM_KV("dev"    , PLYNX_INFO_DEVICE)
#endif
#ifdef PLYNX_INFO_CPU
        PLYNX_PARAM_KV("cpu"    , PLYNX_INFO_CPU)
#endif
#ifdef PLYNX_INFO_CONNECTION
        PLYNX_PARAM_KV("con"    , PLYNX_INFO_CONNECTION)
#endif
#ifdef PLYNX_FIRMWARE_TYPE
        PLYNX_PARAM_KV("fw-type", PLYNX_FIRMWARE_TYPE)
#endif
#ifdef PLYNX_FIRMWARE_VERSION
        PLYNX_PARAM_KV("fw"     , PLYNX_FIRMWARE_VERSION)
#endif
        PLYNX_PARAM_KV("build"  , __DATE__ " " __TIME__)
        "\0"
    ;
    const size_t profile_len = sizeof(profile)-8-2;

    char mem_dyn[64];
    PlynxParam profile_dyn(mem_dyn, 0, sizeof(mem_dyn));
#ifdef PLYNX_TEMPLATE_ID
    {
        const char* tmpl = PLYNX_TEMPLATE_ID;
        if (tmpl && strlen(tmpl)) {
            profile_dyn.add_key("tmpl", tmpl);
        }
    }
#endif

#ifdef PLYNX_HAS_PROGMEM
    char mem[profile_len];
    memcpy_P(mem, profile+8, profile_len);
    static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_INTERNAL, 0, mem, profile_len, profile_dyn.getBuffer(), profile_dyn.getLength());
#else
    static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_INTERNAL, 0, profile+8, profile_len, profile_dyn.getBuffer(), profile_dyn.getLength());
#endif
    return;
}


// Check if analog pins can be referenced by name on this device
#if defined(analogInputToDigitalPin)
    #define PLYNX_DECODE_PIN(it) (((it).asStr()[0] == 'A') ? analogInputToDigitalPin(atoi((it).asStr()+1)) : (it).asInt())
#else
    #define PLYNX_DECODE_PIN(it) ((it).asInt())

    #if defined(PLYNX_DEBUG_ALL)
        #pragma message "analogInputToDigitalPin not defined"
    #endif
#endif

template<class Proto>
PLYNX_FORCE_INLINE
void PlynxApi<Proto>::processCmd(const void* buff, size_t len)
{
    PlynxParam param((void*)buff, len);
    PlynxParam::iterator it = param.begin();
    if (it >= param.end())
        return;
    const char* cmd = it.asStr();
    uint16_t cmd16;
    memcpy(&cmd16, cmd, sizeof(cmd16));
    if (++it >= param.end())
        return;

    const uint8_t pin = PLYNX_DECODE_PIN(it);

    switch(cmd16) {

#ifndef PLYNX_NO_BUILTIN

    case PLYNX_HW_PM: {
        while (it < param.end()) {
            const uint8_t pin = PLYNX_DECODE_PIN(it);
            ++it;
            if (!strcmp(it.asStr(), "in")) {
                pinMode(pin, INPUT);
            } else if (!strcmp(it.asStr(), "out") || !strcmp(it.asStr(), "pwm")) {
                pinMode(pin, OUTPUT);
#ifdef INPUT_PULLUP
            } else if (!strcmp(it.asStr(), "pu")) {
                pinMode(pin, INPUT_PULLUP);
#endif
#ifdef INPUT_PULLDOWN
            } else if (!strcmp(it.asStr(), "pd")) {
                pinMode(pin, INPUT_PULLDOWN);
#endif
            } else {
#ifdef PLYNX_DEBUG
                PLYNX_LOG4(PLYNX_F("Invalid pin "), pin, PLYNX_F(" mode "), it.asStr());
#endif
            }
            ++it;
        }
    } break;
    case PLYNX_HW_DR: {
        char mem[16];
        PlynxParam rsp(mem, 0, sizeof(mem));
        rsp.add("dw");
        rsp.add(pin);
        rsp.add(digitalRead(pin));
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE, 0, rsp.getBuffer(), rsp.getLength()-1);
    } break;
    case PLYNX_HW_DW: {
        // Should be 1 parameter (value)
        if (++it >= param.end())
            return;

#ifdef ESP8266
        // Disable PWM...
        analogWrite(pin, 0);
#endif
#ifndef PLYNX_MINIMIZE_PINMODE_USAGE
        pinMode(pin, OUTPUT);
#endif
        digitalWrite(pin, it.asInt() ? HIGH : LOW);
    } break;
    case PLYNX_HW_AR: {
        char mem[16];
        PlynxParam rsp(mem, 0, sizeof(mem));
        rsp.add("aw");
        rsp.add(pin);
        rsp.add(analogRead(pin));
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE, 0, rsp.getBuffer(), rsp.getLength()-1);
    } break;

// TODO: Remove workaround for ESP32
#if !defined(ESP32)

    case PLYNX_HW_AW: {
        // Should be 1 parameter (value)
        if (++it >= param.end())
            return;

#ifndef PLYNX_MINIMIZE_PINMODE_USAGE
        pinMode(pin, OUTPUT);
#endif
        analogWrite(pin, it.asInt());
    } break;

#endif // TODO: Remove workaround for ESP32

#endif

    case PLYNX_HW_VR: {
        callReadHandler(pin);
    } break;
    case PLYNX_HW_VW: {
        ++it;
        char* start = (char*)it.asStr();
        PlynxParam param2(start, len - (start - (char*)buff));
        callWriteHandler(pin, param2);
    } break;
    default:
        PLYNX_LOG2(PLYNX_F("Invalid HW cmd: "), cmd);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_RESPONSE, static_cast<Proto*>(this)->msgIdOutOverride, NULL, PLYNX_ILLEGAL_COMMAND);
    }
}

#endif
