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
 * @brief The functions you call on the `Plynx` object to talk to the app.
 *
 * The board header creates the `Plynx` object, so the sketch writes
 * `Plynx.virtualWrite(V5, 42)` directly.
 *
 * @see PlynxProtocol, PLYNX_WRITE, PLYNX_READ
 */
template <class Proto>
class PlynxApi
{
public:
    PlynxApi() {
    }

    // connect(), disconnect(), connected() and run() are documented where they
    // are defined, in Plynx/PlynxProtocol.h.

    /**
     * @brief Shows a value in the app, on every widget set to that virtual pin.
     *
     * Numbers, text and `String` all work. A widget that takes more than one
     * value, like the RGB Picker, the Map or the Table, gets them all in one call.
     *
     * @param pin     The virtual pin, such as V1.
     * @param values  The value, or several values separated by commas.
     *
     * @code
     * PlynxTimer timer;
     *
     * void sendTemperature()
     * {
     *   Plynx.virtualWrite(V5, analogRead(A0) * 0.1);
     * }
     *
     * void setup()
     * {
     *   // Plynx.begin(...) as in the example for your board
     *   timer.setInterval(2000L, sendTemperature);
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     *   timer.run();
     * }
     * @endcode
     *
     * @note While the board is offline the value is lost. Send it again in
     * PLYNX_CONNECTED().
     *
     * @warning Send from a timer, or when the value changes. Called at every turn
     * of loop() it slows the whole board down to 15 messages a second.
     *
     * @warning One message holds 128 bytes. A longer text does not arrive, and on
     * ESP32 and ESP8266 a long list of numbers can crash the board. Keep texts
     * short, or define a larger PLYNX_MAX_SENDBYTES before the board header.
     *
     * @see PlynxTimer::setInterval, PLYNX_READ, syncVirtual
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
     * @brief Sends bytes to a virtual pin exactly as they are.
     *
     * A zero byte splits them into separate values in the app. WidgetTerminal sends
     * its text this way.
     *
     * @param pin   The virtual pin, such as V1.
     * @param buff  The bytes to send.
     * @param len   How many bytes.
     *
     * @code
     * void sendTwoValues()
     * {
     *   const char packed[] = "21.5\0" "48";   // two values, 21.5 and 48
     *   Plynx.virtualWriteBinary(V7, packed, sizeof(packed) - 1);
     * }
     * @endcode
     *
     * @note While the board is offline the bytes are lost.
     *
     * @see virtualWrite, WidgetTerminal
     */
    void virtualWriteBinary(int pin, const void* buff, size_t len) {
        char mem[8];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add("vw");
        cmd.add(pin);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE, 0, cmd.getBuffer(), cmd.getLength(), buff, len);
    }

    /**
     * @brief Shows in the app a list of values built while the sketch runs.
     *
     * Each add() on the PlynxParam is one value in the app, for example one
     * reading for each sensor found on a bus.
     *
     * @param pin    The virtual pin, such as V1.
     * @param param  The values, added with PlynxParam::add().
     *
     * @code
     * void sendReadings()
     * {
     *   char buffer[64];
     *   PlynxParam values(buffer, 0, sizeof(buffer));
     *   values.add(21.5);
     *   values.add(48);
     *   Plynx.virtualWrite(V3, values);
     * }
     * @endcode
     *
     * @warning A value that does not fit in the buffer is left out. Size the
     * buffer for the longest list you send.
     *
     * @see virtualWrite, PlynxParam::add
     */
    void virtualWrite(int pin, const PlynxParam& param) {
        virtualWriteBinary(pin, param.getBuffer(), param.getLength());
    }

    /**
     * @brief Shows in the app a list of values kept in heap memory.
     *
     * @param pin    The virtual pin, such as V1.
     * @param param  The values, added with PlynxParam::add().
     *
     * @code
     * void sendList()
     * {
     *   PlynxParamAllocated values(128);
     *   values.add("kitchen");
     *   values.add(23);
     *   Plynx.virtualWrite(V4, values);
     * }
     * @endcode
     *
     * @see PlynxParamAllocated
     */
    void virtualWrite(int pin, const PlynxParamAllocated& param) {
        virtualWriteBinary(pin, param.getBuffer(), param.getLength());
    }

    /**
     * @brief Runs your PLYNX_WRITE() code for a pin with a value you choose.
     *
     * The app does not see the value. In setup() it applies a starting value with
     * the same code that handles the app.
     *
     * @param pin    The virtual pin.
     * @param param  The value your code reads as `param`.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   analogWrite(5, param.asInt());
     * }
     *
     * void applyStartValue()
     * {
     *   char text[] = "128";
     *   PlynxParam start(text, sizeof(text));
     *   Plynx.callWriteHandler(V1, start);
     * }
     * @endcode
     *
     * @see PLYNX_WRITE, callReadHandler, syncVirtual
     */
    void callWriteHandler(int pin, const PlynxParam& param) {
        PlynxReq req = { (uint8_t)pin };
        WidgetWriteHandler handler = GetWriteHandler(pin);
        if (handler && (handler != PlynxWidgetWrite)) {
            handler(req, param);
        } else {
            PlynxWidgetWriteDefault(req, param);
        }
    }

    /**
     * @brief Runs your PLYNX_READ() code for a pin, so its widget gets a fresh value.
     *
     * @param pin  The virtual pin.
     *
     * @code
     * PLYNX_READ(V6)
     * {
     *   Plynx.virtualWrite(V6, millis() / 1000);
     * }
     *
     * void pushUptime()
     * {
     *   Plynx.callReadHandler(V6);
     * }
     * @endcode
     *
     * @see PLYNX_READ, callWriteHandler
     */
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
     * @brief Brings back the last values the app set, so the board restarts where
     * it left off.
     *
     * Every pin with a saved value runs its PLYNX_WRITE() code once, for example
     * to turn the relay back on after a power cut. Call it in PLYNX_CONNECTED().
     *
     * @code
     * PLYNX_CONNECTED()
     * {
     *   Plynx.syncAll();
     * }
     *
     * PLYNX_WRITE(V1)
     * {
     *   digitalWrite(2, param.asInt());   // runs again after every reconnect
     * }
     * @endcode
     *
     * @note The values arrive during the next calls to Plynx.run().
     *
     * @see syncVirtual, PLYNX_CONNECTED, PLYNX_WRITE
     */
    void syncAll() {
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE_SYNC);
    }

    /**
     * @brief A command the library sends to the server by itself, for the time
     * and for the board details. A sketch does not call it.
     *
     * @param params  The words of the command, in order.
     *
     * @see WidgetRTC
     */
    template <typename... Args>
    void sendInternal(Args... params) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add_multi(params...);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_INTERNAL, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * @brief Brings back the last value the app set on the pins you name.
     *
     * Each pin runs its PLYNX_WRITE() code once with that value. Call it in
     * PLYNX_CONNECTED(), or after waking from deep sleep.
     *
     * @param pins  One or more virtual pins, such as V1 or V1, V2, V3.
     *
     * @code
     * PLYNX_CONNECTED()
     * {
     *   Plynx.syncVirtual(V1, V2);
     * }
     *
     * PLYNX_WRITE(V1)
     * {
     *   digitalWrite(2, param.asInt());
     * }
     *
     * PLYNX_WRITE(V2)
     * {
     *   analogWrite(4, param.asInt());
     * }
     * @endcode
     *
     * @note The values arrive during the next calls to Plynx.run(). A pin that
     * never had a value gets nothing.
     *
     * @warning A pin without PLYNX_WRITE() gets the value in PLYNX_WRITE_DEFAULT(),
     * or nowhere when that is missing too.
     *
     * @see syncAll, PLYNX_CONNECTED, PLYNX_WRITE
     */
    template <typename... Args>
    void syncVirtual(Args... pins) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add("vr");
        cmd.add_multi(pins...);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE_SYNC, 0, cmd.getBuffer(), cmd.getLength()-1);
    }


    /**
     * @brief The Twitter widget of old Blynk projects, kept so old sketches still
     * compile.
     *
     * With PLYNX_TEMPLATE_ID defined it sends a push notification instead.
     *
     * @param msg  The text of the message.
     *
     * @deprecated Use logEvent().
     *
     * @code
     * void sendAlert()
     * {
     *   Plynx.logEvent("alert", "Temperature above 30 C");   // instead of tweet()
     * }
     * @endcode
     *
     * @see logEvent
     */
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

    /**
     * @brief Sends a push notification to the phone.
     *
     * Needs the Notification widget in the project.
     *
     * @param msg  The text the phone shows.
     *
     * @deprecated Use logEvent(), which does the same.
     *
     * @code
     * void sendAlert()
     * {
     *   Plynx.logEvent("alert", "Temperature above 30 C");   // instead of notify()
     * }
     * @endcode
     *
     * @see logEvent
     */
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

    /**
     * @brief Sends an email to an address.
     *
     * Needs the Email widget in the project. An address written in the widget is
     * used instead of this one. With PLYNX_TEMPLATE_ID defined it sends a push
     * notification instead.
     *
     * @param email    The address.
     * @param subject  The subject.
     * @param msg      The text of the email.
     *
     * @deprecated Use logEvent().
     *
     * @code
     * void sendAlert()
     * {
     *   Plynx.logEvent("alert", "Battery below 20%");   // instead of email()
     * }
     * @endcode
     *
     * @see logEvent
     */
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

    /**
     * @brief Sends an email to the address written in the Email widget.
     *
     * Without an address in the widget, the email goes to the address of the Plynx
     * account. With PLYNX_TEMPLATE_ID defined it sends a push notification
     * instead.
     *
     * @param subject  The subject.
     * @param msg      The text of the email.
     *
     * @deprecated Use logEvent().
     *
     * @code
     * void sendAlert()
     * {
     *   Plynx.logEvent("alert", "Tank full");   // instead of email()
     * }
     * @endcode
     *
     * @see logEvent
     */
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
     * @brief Changes the label, the color or the range of a widget, from the board.
     *
     * The value of the widget stays as it is. These are the property names:
     * - `label`, the title of the widget.
     * - `color`, the main color, as `"#RRGGBB"` or as a number like 0xFF0000.
     * - `onLabel` and `offLabel`, the text of a Button when on and off.
     * - `onColor`, `offColor`, `onBackColor` and `offBackColor`, the text and
     *   background colors of a Button when on and off.
     * - `min`, `max` and `step`, the range of a Slider, a Step, a Gauge or a Level
     *   Display.
     * - `maximumFractionDigits`, the decimals shown, from 0 to 3.
     * - `valueFormatting` and `suffix`, the text around the value, like `"°C"`.
     * - `url`, the address shown by an Image or a Video widget.
     *
     * @param pin       The virtual pin of the widget.
     * @param property  The property name, from the list above.
     * @param values    The new value.
     *
     * @code
     * void showAlarm(bool on)
     * {
     *   Plynx.setProperty(V3, "color", on ? "#D3435C" : "#23C48E");
     *   Plynx.setProperty(V3, "label", on ? "Too hot" : "Temperature");
     * }
     * @endcode
     *
     * @warning A name outside the list, or an empty value, changes nothing and the
     * sketch is not told.
     *
     * @warning One message holds 128 bytes. A longer text does not arrive, and on
     * ESP32 and ESP8266 a long list of numbers can crash the board.
     *
     * @see WidgetLED, PlynxWidgetBase::setLabel, PlynxWidgetBase::setColor
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

    /**
     * @brief Changes a widget property with a value built with PlynxParam.
     *
     * @param pin       The virtual pin of the widget.
     * @param property  The property name.
     * @param param     The value, built with PlynxParam::add().
     *
     * @code
     * void setRange()
     * {
     *   char buffer[16];
     *   PlynxParam value(buffer, 0, sizeof(buffer));
     *   value.add(100);
     *   Plynx.setProperty(V2, "max", value);
     * }
     * @endcode
     *
     * @see setProperty
     */
    template <typename T>
    void setProperty(int pin, const T& property, const PlynxParam& param) {
        char mem[32];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(pin);
        cmd.add(property);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_PROPERTY, 0, cmd.getBuffer(), cmd.getLength(), param.getBuffer(), param.getLength());
    }

    /**
     * @brief Changes a widget property with a value kept in heap memory.
     *
     * @param pin       The virtual pin of the widget.
     * @param property  The property name.
     * @param param     The value, built with PlynxParam::add().
     *
     * @code
     * void renameWidget()
     * {
     *   PlynxParamAllocated value(32);
     *   value.add("Kitchen");
     *   Plynx.setProperty(V8, "label", value);
     * }
     * @endcode
     *
     * @see setProperty
     */
    template <typename T>
    void setProperty(int pin, const T& property, const PlynxParamAllocated& param) {
        char mem[32];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(pin);
        cmd.add(property);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_PROPERTY, 0, cmd.getBuffer(), cmd.getLength(), param.getBuffer(), param.getLength());
    }

    /**
     * @brief Sends a push notification to the phone, with the event name as text.
     *
     * Needs the Notification widget in the project. Call it once when the thing
     * happens, not at every turn of loop().
     *
     * @param event_name  A short name, such as `"alarm"`.
     *
     * @code
     * void checkInput()
     * {
     *   static bool sent = false;
     *   bool on = digitalRead(4) == LOW;
     *   if (on && !sent) {
     *     Plynx.logEvent("alarm");
     *   }
     *   sent = on;
     * }
     * @endcode
     *
     * @warning The phone gets at most one notification every 5 seconds from the
     * same board. Name and text together hold 128 bytes.
     *
     * @see logEvent(const NAME&, const DESCR&), clearEvent
     */
    template <typename NAME>
    void logEvent(const NAME& event_name) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(event_name);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_EVENT_LOG, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * @brief Sends a push notification to the phone, with your sentence as text.
     *
     * Needs the Notification widget in the project. Call it once when the thing
     * happens, not at every turn of loop().
     *
     * @param event_name   A short name, such as `"alarm"`.
     * @param description  The sentence the phone shows.
     *
     * @code
     * PlynxTimer timer;
     *
     * void checkTemperature()
     * {
     *   float celsius = analogRead(A0) * 0.1;   // replace with your sensor
     *   if (celsius > 30) {
     *     Plynx.logEvent("hot", String("Temperature ") + celsius + " C");
     *   }
     * }
     *
     * void setup()
     * {
     *   // Plynx.begin(...) as in the example for your board
     *   timer.setInterval(60000L, checkTemperature);
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     *   timer.run();
     * }
     * @endcode
     *
     * @warning The phone gets at most one notification every 5 seconds from the
     * same board. Name and text together hold 128 bytes.
     *
     * @see logEvent(const NAME&), clearEvent
     */
    template <typename NAME, typename DESCR>
    void logEvent(const NAME& event_name, const DESCR& description) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(event_name);
        cmd.add(description);
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_EVENT_LOG, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * @brief Marks an event as solved on servers that keep a list of events. A
     * Plynx server keeps no list, and nothing happens.
     *
     * @param event_name  The name used in logEvent().
     *
     * @code
     * void alarmOver()
     * {
     *   Plynx.clearEvent("alarm");
     * }
     * @endcode
     *
     * @see logEvent
     */
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
     * @brief Runs your PLYNX_READ() code for a pin. Needs PLYNX_EXPERIMENTAL
     * defined before the board header.
     *
     * @experimental
     *
     * @param pin  The virtual pin.
     *
     * @code
     * // callReadHandler() does the same and needs no setting
     * PLYNX_READ(V6)
     * {
     *   Plynx.virtualWrite(V6, millis() / 1000);
     * }
     *
     * void refreshUptime()
     * {
     *   Plynx.callReadHandler(V6);
     * }
     * @endcode
     *
     * @warning In this version a sketch that calls refresh() does not compile. Use
     * callReadHandler().
     *
     * @see callReadHandler, PLYNX_EXPERIMENTAL
     */
    void refresh(int pin) {
        if (WidgetReadHandler handler = GetReadHandler(pin)) {
            PlynxReq req = { 0, PLYNX_SUCCESS, (uint8_t)pin };
            handler(req);
        }
    }

    /**
     * @brief Waits a number of milliseconds while the board stays connected, so the
     * app keeps working.
     *
     * Needs PLYNX_EXPERIMENTAL defined before the board header.
     *
     * @experimental
     *
     * @param ms  How long to wait, in milliseconds.
     *
     * @code
     * #define PLYNX_EXPERIMENTAL
     * #include <PlynxSimpleEsp32.h>
     *
     * void blinkSlowly()
     * {
     *   digitalWrite(2, HIGH);
     *   Plynx.delay(500);
     *   digitalWrite(2, LOW);
     *   Plynx.delay(500);
     * }
     * @endcode
     *
     * @warning A PLYNX_WRITE() that calls Plynx.delay() while another wait is
     * running uses more memory each time, and a small board can crash. A
     * PlynxTimer is the safer way to wait.
     *
     * @see PlynxTimer, run, PLYNX_EXPERIMENTAL
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
