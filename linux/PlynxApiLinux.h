/**
 * @file       PlynxApiLinux.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef PlynxApiLinux_h
#define PlynxApiLinux_h

#include <Plynx/PlynxApi.h>

#ifndef PLYNX_INFO_DEVICE
    #define PLYNX_INFO_DEVICE  "Linux"
#endif

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
#ifdef PLYNX_INFO_PINS
        // "pins": the header this sketch was written for (PLYNX_BOARD_PINS,
        // Plynx/PlynxConfig.h). Static on purpose: it is a fact of the build,
        // and here it costs no byte of the dynamic buffer below.
        PLYNX_PARAM_KV("pins"   , PLYNX_INFO_PINS)
#endif
        PLYNX_PARAM_KV("build"  , __DATE__ " " __TIME__)
        "\0"
    ;
    const size_t profile_len = sizeof(profile)-8-2;

    char mem_dyn[64];
    PlynxParam profile_dyn(mem_dyn, 0, sizeof(mem_dyn));
    profile_dyn.add_key("conn", "Socket");
#ifdef PLYNX_TEMPLATE_ID
    {
        const char* tmpl = PLYNX_TEMPLATE_ID;
        if (tmpl && strlen(tmpl)) {
            profile_dyn.add_key("tmpl", tmpl);
        }
    }
#endif

    static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_INTERNAL, 0, profile+8, profile_len, profile_dyn.getBuffer(), profile_dyn.getLength());
    return;
}


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

    const uint8_t pin = it.asInt();

    switch(cmd16) {

#ifndef PLYNX_NO_BUILTIN

    case PLYNX_HW_PM: {
        while (it < param.end()) {
            ++it;
#ifdef PLYNX_DEBUG
            PLYNX_LOG4(PLYNX_F("Invalid pin "), pin, PLYNX_F(" mode "), it.asStr());
#endif
            ++it;
        }
    } break;
    case PLYNX_HW_DR: {
        char mem[16];
        PlynxParam rsp(mem, 0, sizeof(mem));
        rsp.add("dw");
        rsp.add(pin);
        rsp.add(0); // TODO
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE, 0, rsp.getBuffer(), rsp.getLength()-1);
    } break;
    case PLYNX_HW_DW: {
        // Should be 1 parameter (value)
        if (++it >= param.end())
            return;

        // TODO: digitalWrite(pin, it.asInt() ? HIGH : LOW);
    } break;
    case PLYNX_HW_AW: {
        // Should be 1 parameter (value)
        if (++it >= param.end())
            return;

        // TODO: analogWrite(pin, it.asInt());
    } break;

#endif

    case PLYNX_HW_VR: {
        PlynxReq req = { pin };
        WidgetReadHandler handler = GetReadHandler(pin);
        if (handler && (handler != PlynxWidgetRead)) {
            handler(req);
        } else {
            PlynxWidgetReadDefault(req);
        }
    } break;
    case PLYNX_HW_VW: {
        ++it;
        char* start = (char*)it.asStr();
        PlynxParam param2(start, len - (start - (char*)buff));
        PlynxReq req = { pin };
        WidgetWriteHandler handler = GetWriteHandler(pin);
        if (handler && (handler != PlynxWidgetWrite)) {
            handler(req, param2);
        } else {
            PlynxWidgetWriteDefault(req, param2);
        }
    } break;
    default:
        PLYNX_LOG2(PLYNX_F("Invalid HW cmd: "), cmd);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_RESPONSE, static_cast<Proto*>(this)->msgIdOutOverride, NULL, PLYNX_ILLEGAL_COMMAND);
    }
}

#endif
