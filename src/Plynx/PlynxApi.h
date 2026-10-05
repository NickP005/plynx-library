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
 * @brief The part of the Plynx object that sends values, syncs pins and talks
 * to the app.
 *
 * A sketch never creates this class. The board header (for example
 * `<PlynxSimpleEsp32.h>`) creates one global object called `Plynx`, and the
 * sketch calls these functions on it, as in `Plynx.virtualWrite(V5, 42)`.
 * Connecting (`begin`, `connect`, `run`) lives in PlynxProtocol and in the
 * board class.
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
     * @brief Sends one or more values from the board to a virtual pin in the app.
     *
     * The values travel to the server, which keeps them as the current value of
     * the pin and forwards them to every phone that shows the project. Numbers,
     * text, `String` and `F("...")` text are accepted. With more than one value
     * the widget receives them all, in order, which is what an RGB Picker, a Map
     * or a Table expect.
     *
     * Numbers become text on the board. A `float` keeps 3 decimals and a
     * `double` keeps 7.
     *
     * @param pin     The virtual pin, written V0, V1, V2 and so on.
     * @param values  One or more values to send, separated by commas.
     *
     * @par Where to call
     * In a function run by a PlynxTimer, inside PLYNX_READ() to answer the app,
     * inside PLYNX_WRITE() to confirm a value, or in PLYNX_CONNECTED() to send
     * the current state after a reconnect.
     *
     * @par App widget
     * Every widget set to that virtual pin. A Value Display, a Gauge, a Chart
     * or a Level Display show the value, a Button or a Slider move to it.
     *
     * @code
     * PlynxTimer timer;
     *
     * void sendTemperature()
     * {
     *   float celsius = analogRead(A0) * 0.1;   // replace with your sensor
     *   Plynx.virtualWrite(V5, celsius);
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
     * @note While the board is offline the call does nothing and the value is
     * lost. Send the values that matter again in PLYNX_CONNECTED().
     *
     * @warning Calling it at every turn of loop() floods the connection. The
     * library spaces messages out to PLYNX_MSG_LIMIT per second (15 by default)
     * by waiting inside the call, so the whole loop() slows down. Send from a
     * timer, or only when the value changes.
     *
     * @warning The whole message has to fit in PLYNX_MAX_SENDBYTES (128 bytes
     * by default). A text value that does not fit is left out of the message.
     *
     * @see PlynxTimer::setInterval, PLYNX_READ, syncVirtual, PLYNX_MSG_LIMIT
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
     * @brief Sends raw bytes to a virtual pin, exactly as they are in memory.
     *
     * The bytes are sent without any conversion. Use it to send several values
     * already packed with zero bytes between them, or text that is not
     * zero-terminated. WidgetTerminal uses it to send what it has collected.
     *
     * @param pin   The virtual pin, written V0, V1, V2 and so on.
     * @param buff  The first byte to send.
     * @param len   How many bytes to send.
     *
     * @par Where to call
     * The same places as virtualWrite(), never at every turn of loop().
     *
     * @par App widget
     * Every widget set to that virtual pin. Zero bytes inside the buffer split
     * it into separate values.
     *
     * @code
     * void sendTwoValues()
     * {
     *   const char packed[] = "21.5\0" "48";   // two values, 21.5 and 48
     *   Plynx.virtualWriteBinary(V7, packed, sizeof(packed) - 1);
     * }
     * @endcode
     *
     * @note Like virtualWrite(), it does nothing while the board is offline.
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
     * @brief Sends a list of values built with PlynxParam to a virtual pin.
     *
     * Useful when the number of values is known only while the sketch runs,
     * for example a reading for each sensor found on a bus. Each add() on the
     * PlynxParam becomes one value in the app.
     *
     * @param pin    The virtual pin, written V0, V1, V2 and so on.
     * @param param  The values, added one by one with PlynxParam::add().
     *
     * @par Where to call
     * The same places as virtualWrite(), never at every turn of loop().
     *
     * @par App widget
     * Every widget set to that virtual pin, the same as virtualWrite() with
     * several values.
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
     * @warning A value that does not fit in the buffer given to PlynxParam is
     * left out. Size the buffer for the longest list you send.
     *
     * @see virtualWrite, PlynxParam::add
     */
    void virtualWrite(int pin, const PlynxParam& param) {
        virtualWriteBinary(pin, param.getBuffer(), param.getLength());
    }

    /**
     * @brief Sends a list of values built with PlynxParamAllocated to a
     * virtual pin.
     *
     * The same as the PlynxParam version, for a list whose memory was taken
     * from the heap with PlynxParamAllocated.
     *
     * @param pin    The virtual pin, written V0, V1, V2 and so on.
     * @param param  The values, added one by one with PlynxParam::add().
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
     * @brief Runs the PLYNX_WRITE() handler of a pin from the sketch, as if the
     * app had sent the value.
     *
     * It calls the handler the sketch wrote for that pin, or the
     * PLYNX_WRITE_DEFAULT() handler when there is none. Nothing travels on the
     * network and the app does not see the value.
     *
     * @param pin    The virtual pin whose handler runs.
     * @param param  The value the handler finds in `param`.
     *
     * @par Where to call
     * Anywhere in the sketch, for example in setup() to apply a starting value
     * with the same code that handles the app.
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
     * @brief Runs the PLYNX_READ() handler of a pin from the sketch, as if the
     * app had asked for the value.
     *
     * It calls the handler the sketch wrote for that pin, or the
     * PLYNX_READ_DEFAULT() handler when there is none. The handler usually
     * answers with virtualWrite(), so the value does reach the app.
     *
     * @param pin  The virtual pin whose handler runs.
     *
     * @par Where to call
     * Anywhere in the sketch, for example from a timer, to push the value with
     * the same code that answers the app.
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
     * @brief Asks the server to send back the last value of every pin of this
     * board.
     *
     * The values arrive as normal writes, so every PLYNX_WRITE() handler of a
     * pin with a stored value runs once. It is the way a board finds the state
     * of the app again after a restart, for example the position of a Button.
     *
     * @par Where to call
     * Inside PLYNX_CONNECTED(), so it runs after every connect and reconnect.
     *
     * @par App widget
     * Every widget of the project that has a value for this board, such as a
     * Button, a Slider or an RGB Picker.
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
     * @note The answer arrives during the next calls to Plynx.run(), not
     * inside syncAll() itself.
     *
     * @see syncVirtual, PLYNX_CONNECTED, PLYNX_WRITE
     */
    void syncAll() {
        static_cast<Proto*>(this)->sendCmd(PLYNX_CMD_HARDWARE_SYNC);
    }

    /**
     * @brief Sends an internal command to the server, used by the library
     * itself.
     *
     * The library uses it to ask for the time (WidgetRTC) and to report about
     * the board. A sketch has no need to call it.
     *
     * @param params  The words of the command, sent in order.
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
     * @brief Asks the server to send back the last value of the virtual pins
     * you name.
     *
     * The values arrive as normal writes, so the PLYNX_WRITE() handler of each
     * pin runs once with its stored value. It is the same as syncAll() for a
     * few pins only.
     *
     * @param pins  One or more virtual pins, such as V1 or V1, V2, V3.
     *
     * @par Where to call
     * Inside PLYNX_CONNECTED(), or wherever the sketch needs the value kept on
     * the server, for example after waking from deep sleep.
     *
     * @par App widget
     * The widgets set to those pins. A pin with no stored value sends nothing
     * back.
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
     * @note The answer arrives during the next calls to Plynx.run(), not
     * inside syncVirtual() itself.
     *
     * @warning A pin without a PLYNX_WRITE() handler receives the value in
     * PLYNX_WRITE_DEFAULT(), or nowhere when that is missing too.
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
     * @brief Sends a message for the Twitter widget of old Blynk-legacy
     * projects.
     *
     * Kept so that old sketches still compile. With PLYNX_TEMPLATE_ID defined
     * it becomes `logEvent("notify", msg)`.
     *
     * @param msg  The text of the message.
     *
     * @deprecated Use logEvent(). The compiler prints a warning where tweet()
     * is used.
     *
     * @code
     * void reportLeak()
     * {
     *   Plynx.logEvent("leak", "Water under the sink");   // instead of tweet()
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
     * @brief Sends a push notification to the phone, the old way.
     *
     * The server delivers it through the Notification widget of the project.
     * With PLYNX_TEMPLATE_ID defined it becomes `logEvent("notify", msg)`.
     *
     * @param msg  The text the phone shows.
     *
     * @deprecated Use logEvent(), which reaches the phone the same way. The
     * compiler prints a warning where notify() is used.
     *
     * @par App widget
     * Notification. Without that widget in the project nothing arrives.
     *
     * @code
     * void reportDoor()
     * {
     *   Plynx.logEvent("door", "The garage door is open");   // instead of notify()
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
     * @brief Asks the server to send an email to an address, the old way.
     *
     * With PLYNX_TEMPLATE_ID defined it becomes `logEvent("notify", msg)` and
     * the address and the subject are not used.
     *
     * @param email    The address that receives the email.
     * @param subject  The subject line.
     * @param msg      The body of the email.
     *
     * @deprecated Use logEvent(). The compiler prints a warning where email()
     * is used.
     *
     * @par App widget
     * Email. The server sends the email only when the project has that widget,
     * and an address written in the widget wins over the one in the sketch.
     *
     * @code
     * void reportBattery()
     * {
     *   Plynx.logEvent("battery", "Battery below 20%");   // instead of email()
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
     * @brief Asks the server to send an email to the address set in the Email
     * widget, the old way.
     *
     * With PLYNX_TEMPLATE_ID defined it becomes `logEvent("notify", msg)` and
     * the subject is not used.
     *
     * @param subject  The subject line.
     * @param msg      The body of the email.
     *
     * @deprecated Use logEvent(). The compiler prints a warning where email()
     * is used.
     *
     * @par App widget
     * Email. The email goes to the address written in the widget, or to the
     * email of the Plynx account when the widget has none.
     *
     * @code
     * void reportTank()
     * {
     *   Plynx.logEvent("tank", "The water tank is full");   // instead of email()
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
     * @brief Changes how a widget looks in the app, such as its label or its
     * color, from the board.
     *
     * It changes a property of every widget set to that virtual pin, never its
     * value. The Plynx app applies these property names.
     * - `label`, the title of the widget.
     * - `color`, the main color, as `"#RRGGBB"` or as a number like 0xFF0000.
     * - `onLabel` and `offLabel`, the text of a Button when on and off.
     * - `onColor`, `offColor`, `onBackColor` and `offBackColor`, the text and
     *   background colors of a Button when on and off.
     * - `min`, `max` and `step`, the range and the step of a Slider, a Step, a
     *   Gauge or a Level Display.
     * - `maximumFractionDigits`, the decimals shown, from 0 to 3.
     * - `valueFormatting` and `suffix`, the text around the value, like `"°C"`.
     * - `url`, the address shown by an Image or a Video widget.
     *
     * @param pin       The virtual pin of the widget.
     * @param property  The name of the property, from the list above.
     * @param values    The new value of the property.
     *
     * @par Where to call
     * In PLYNX_CONNECTED() to set the look once, or when something changes,
     * for example to turn a Gauge red above a limit.
     *
     * @par App widget
     * Every widget on that virtual pin that has the property.
     *
     * @code
     * void showAlarm(bool on)
     * {
     *   Plynx.setProperty(V3, "color", on ? "#D3435C" : "#23C48E");
     *   Plynx.setProperty(V3, "label", on ? "Too hot" : "Temperature");
     * }
     * @endcode
     *
     * @warning The server refuses an empty value and a name it does not know.
     * Its answer is an error code that the library does not pass on, so the
     * sketch is not told. Check the name in the list above.
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
     * @brief Changes a widget property with a value list built with PlynxParam.
     *
     * The same as the other setProperty(), for a property whose value is a list
     * built while the sketch runs.
     *
     * @param pin       The virtual pin of the widget.
     * @param property  The name of the property.
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
     * @brief Changes a widget property with a value list built with
     * PlynxParamAllocated.
     *
     * The same as the PlynxParam version, for a list kept on the heap.
     *
     * @param pin       The virtual pin of the widget.
     * @param property  The name of the property.
     * @param param     The value, built with PlynxParam::add().
     *
     * @code
     * void renameWidget()
     * {
     *   PlynxParamAllocated value(32);
     *   value.add("Garden");
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
     * @brief Reports that something happened on the board, and the phone gets
     * a push notification.
     *
     * A Plynx server delivers the event as a push notification through the
     * Notification widget of the project. With only a name, the name is the
     * text the phone shows.
     *
     * @param event_name  A short name for the event, such as `"leak"`.
     *
     * @par Where to call
     * Where the event is detected, once per event. A timer or a change check
     * keeps it from repeating at every turn of loop().
     *
     * @par App widget
     * Notification. Without that widget in the project nothing arrives.
     *
     * @code
     * void checkWater()
     * {
     *   static bool reported = false;
     *   bool wet = digitalRead(4) == LOW;
     *   if (wet && !reported) {
     *     Plynx.logEvent("leak");
     *   }
     *   reported = wet;
     * }
     * @endcode
     *
     * @warning The name and the description together have to fit in
     * PLYNX_MAX_SENDBYTES (128 bytes by default), and a text that does not fit
     * is left out. With the default server settings the phone receives at most
     * one notification every 5 seconds from the same board.
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
     * @brief Reports that something happened on the board, with a sentence the
     * phone shows in the push notification.
     *
     * A Plynx server delivers the event as a push notification through the
     * Notification widget of the project. The description is the text on the
     * lock screen.
     *
     * @param event_name   A short name for the event, such as `"leak"`.
     * @param description  The sentence the person reads on the phone.
     *
     * @par Where to call
     * Where the event is detected, once per event, never at every turn of
     * loop().
     *
     * @par App widget
     * Notification. Without that widget in the project nothing arrives.
     *
     * @code
     * PlynxTimer timer;
     *
     * void checkTemperature()
     * {
     *   float celsius = analogRead(A0) * 0.1;   // replace with your sensor
     *   if (celsius > 30) {
     *     Plynx.logEvent("hot", String("Greenhouse at ") + celsius + " C");
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
     * @warning The name and the description together have to fit in
     * PLYNX_MAX_SENDBYTES (128 bytes by default), and a text that does not fit
     * is left out. With the default server settings the phone receives at most
     * one notification every 5 seconds from the same board.
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
     * @brief Sends the command that marks an event as solved, for servers that
     * keep a list of events.
     *
     * A Plynx server turns events into push notifications and keeps no list,
     * so there it has no effect. It is here for sketches written for servers
     * that have an events list.
     *
     * @param event_name  The name used in logEvent().
     *
     * @code
     * void waterGone()
     * {
     *   Plynx.clearEvent("leak");
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
     * @brief Runs the PLYNX_READ() handler of a pin, so the widget gets a fresh
     * value.
     *
     * Available only with PLYNX_EXPERIMENTAL defined before the board header.
     *
     * @experimental
     *
     * @param pin  The virtual pin whose PLYNX_READ() handler runs.
     *
     * @code
     * // callReadHandler() does the same job and needs no setting
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
     * @warning In this version a sketch that calls refresh() stops the build
     * with "too many initializers for PlynxReq". Use callReadHandler().
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
     * @brief Waits a number of milliseconds while the board keeps talking to
     * the server.
     *
     * Unlike the Arduino delay(), it calls Plynx.run() over and over while it
     * waits, so the connection stays alive and PLYNX_WRITE() handlers run in
     * the meantime. Available only with PLYNX_EXPERIMENTAL defined before the
     * board header.
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
     * @warning A PLYNX_WRITE() handler that runs during the wait can call
     * Plynx.delay() again, and each level takes more stack. Boards with little
     * RAM can crash this way. A PlynxTimer is the safer way to wait.
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
