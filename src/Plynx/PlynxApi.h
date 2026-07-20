/**
 * @file       PlynxApi.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      High-level functions
 *
 */

#ifndef PlynxApi_h
#define PlynxApi_h

// back-compat

#ifdef BOARD_FIRMWARE_TYPE
    #define PLYNX_FIRMWARE_TYPE         BOARD_FIRMWARE_TYPE
#endif

#ifdef BOARD_FIRMWARE_VERSION
    #define PLYNX_FIRMWARE_VERSION      BOARD_FIRMWARE_VERSION
#endif

#ifdef BOARD_TEMPLATE_ID
    #define PLYNX_TEMPLATE_ID           BOARD_TEMPLATE_ID
#endif

// end of back-compat

#if !defined(PLYNX_FIRMWARE_TYPE) && defined(PLYNX_TEMPLATE_ID)
    #define PLYNX_FIRMWARE_TYPE         PLYNX_TEMPLATE_ID
#endif

#include <Plynx/PlynxConfig.h>
#include <Plynx/PlynxDebug.h>
#include <Plynx/PlynxParam.h>
#include <Plynx/PlynxTimer.h>
#include <Plynx/PlynxHandlers.h>
#include <Plynx/PlynxProtocolDefs.h>

#if defined(PLYNX_EXPERIMENTAL)
    #include <Plynx/PlynxEveryN.h>
#endif

/**
 * Represents high-level functions of Plynx
 */
template <class Proto>
class PlynxApi
{
public:
    PlynxApi() {
    }

#ifdef DOXYGEN // These API here are only for the documentation

    /**
     * Connects to the server.
     * Blocks until connected or timeout happens.
     * May take less or more then timeout value.
     *
     * @param timeout    Connection timeout
     * @returns          True if connected to the server
     */
    bool connect(unsigned long timeout = PLYNX_TIMEOUT_MS*3);

    /**
     * Disconnects from the server.
     * It will not try to reconnect, until connect() is called
     */
    void disconnect();

    /**
     * @returns          True if connected to the server
     */
    bool connected();

    /**
     * Performs Plynx-related housekeeping
     * and processes incoming commands
     *
     * @param available  True if there is incoming data to process
     *                   Only used when user manages connection manually.
     */
    bool run(bool available = false);

#endif // DOXYGEN

    /**
     * Sends value to a Virtual Pin
     *
     * @param pin  Virtual Pin number
     * @param data Value to be sent
     */
    template <typename... Args>
    void virtualWrite(int pin, Args... values) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add("vw");
        cmd.add(pin);
        cmd.add_multi(values...);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * Sends buffer to a Virtual Pin
     *
     * @param pin  Virtual Pin number
     * @param buff Data buffer
     * @param len  Length of data
     */
    void virtualWriteBinary(int pin, const void* buff, size_t len) {
        char mem[8];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add("vw");
        cmd.add(pin);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE, 0, cmd.getBuffer(), cmd.getLength(), buff, len);
    }

    /**
     * Sends PlynxParam to a Virtual Pin
     *
     * @param pin  Virtual Pin number
     * @param param
     */
    void virtualWrite(int pin, const PlynxParam& param) {
        virtualWriteBinary(pin, param.getBuffer(), param.getLength());
    }

    void virtualWrite(int pin, const PlynxParamAllocated& param) {
        virtualWriteBinary(pin, param.getBuffer(), param.getLength());
    }

    void callWriteHandler(int pin, const PlynxParam& param) {
        PlynxReq req = { (uint8_t)pin };
        WidgetWriteHandler handler = GetWriteHandler(pin);
        if (handler && (handler != PlynxWidgetWrite)) {
            handler(req, param);
        } else {
            PlynxWidgetWriteDefault(req, param);
        }
    }

    void callReadHandler(int pin) {
        PlynxReq req = { (uint8_t)pin };
        WidgetReadHandler handler = GetReadHandler(pin);
        if (handler && (handler != PlynxWidgetRead)) {
            handler(req);
        } else {
            PlynxWidgetReadDefault(req);
        }
    }

    /**
     * Requests Server to re-send current values for all widgets.
     */
    void syncAll() {
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE_SYNC);
    }

    /**
     * Sends internal command
     */
    template <typename... Args>
    void sendInternal(Args... params) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add_multi(params...);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_INTERNAL, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * Requests App or Server to re-send current value of a Virtual Pin.
     * This will probably cause user-defined PLYNX_WRITE handler to be called.
     *
     * @param pin Virtual Pin number
     */
    template <typename... Args>
    void syncVirtual(Args... pins) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add("vr");
        cmd.add_multi(pins...);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE_SYNC, 0, cmd.getBuffer(), cmd.getLength()-1);
    }


    // Please use Plynx.logEvent("event", "Description")
    template<typename T>
    PLYNX_DEPRECATED
    void tweet(const T& msg) {
#if defined(PLYNX_TEMPLATE_ID)
        logEvent("notify", msg);
#else
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(msg);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_TWEET, 0, cmd.getBuffer(), cmd.getLength()-1);
#endif
    }

    // Please use Plynx.logEvent("event", "Description")
    template<typename T>
    PLYNX_DEPRECATED
    void notify(const T& msg) {
#if defined(PLYNX_TEMPLATE_ID)
        logEvent("notify", msg);
#else
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(msg);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_NOTIFY, 0, cmd.getBuffer(), cmd.getLength()-1);
#endif
    }

    // Please use Plynx.logEvent("event", "Description")
    template <typename T1, typename T2>
    PLYNX_DEPRECATED
    void email(const char* email, const T1& subject, const T2& msg) {
#if defined(PLYNX_TEMPLATE_ID)
        logEvent("notify", msg);
#else
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(email);
        cmd.add(subject);
        cmd.add(msg);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_EMAIL, 0, cmd.getBuffer(), cmd.getLength()-1);
#endif
    }

    // Please use Plynx.logEvent("event", "Description")
    template <typename T1, typename T2>
    PLYNX_DEPRECATED
    void email(const T1& subject, const T2& msg) {
#if defined(PLYNX_TEMPLATE_ID)
        logEvent("notify", msg);
#else
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(subject);
        cmd.add(msg);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_EMAIL, 0, cmd.getBuffer(), cmd.getLength()-1);
#endif
    }

    /**
     * Sets property of a Widget
     *
     * @experimental
     *
     * @param pin      Virtual Pin number
     * @param property Property name ("label", "labels", "color", ...)
     * @param value    Property value
     */
    template <typename T, typename... Args>
    void setProperty(int pin, const T& property, Args... values) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(pin);
        cmd.add(property);
        cmd.add_multi(values...);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_PROPERTY, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    template <typename T>
    void setProperty(int pin, const T& property, const PlynxParam& param) {
        char mem[32];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(pin);
        cmd.add(property);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_PROPERTY, 0, cmd.getBuffer(), cmd.getLength(), param.getBuffer(), param.getLength());
    }

    template <typename T>
    void setProperty(int pin, const T& property, const PlynxParamAllocated& param) {
        char mem[32];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(pin);
        cmd.add(property);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_PROPERTY, 0, cmd.getBuffer(), cmd.getLength(), param.getBuffer(), param.getLength());
    }

    template <typename NAME>
    void logEvent(const NAME& event_name) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(event_name);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_EVENT_LOG, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    template <typename NAME, typename DESCR>
    void logEvent(const NAME& event_name, const DESCR& description) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(event_name);
        cmd.add(description);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_EVENT_LOG, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    template <typename NAME>
    void clearEvent(const NAME& event_name) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(event_name);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_EVENT_CLEAR, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

#if defined(PLYNX_EXPERIMENTAL)
    // Attention!
    // Every function in this section may be changed, removed or renamed.

    /**
     * Refreshes value of a widget by running
     * user-defined PLYNX_READ handler of a pin.
     *
     * @experimental
     *
     * @param pin Virtual Pin number
     */
    void refresh(int pin) {
        if (WidgetReadHandler handler = GetReadHandler(pin)) {
            PlynxReq req = { 0, PLYNX_SUCCESS, (uint8_t)pin };
            handler(req);
        }
    }

    /**
     * Delays for N milliseconds, handling server communication in background.
     *
     * @experimental
     * @warning Should be used very carefully, especially on platforms with small RAM.
     *
     * @param ms Milliseconds to wait
     */
    void delay(unsigned long ms) {
        uint16_t start = (uint16_t)micros();
        while (ms > 0) {
            static_cast<Proto*>(this)->run();
#if !defined(PLYNX_NO_YIELD)
            yield();
#endif
            if (((uint16_t)micros() - start) >= 1000) {
                ms--;
                start += 1000;
            }
        }
    }

#endif

protected:
    void processCmd(const void* buff, size_t len);
    void sendInfo();
};


#endif
