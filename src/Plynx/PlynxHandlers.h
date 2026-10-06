/**
 * @file       PlynxHandlers.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Handlers for virtual pin operations
 *
 */

#ifndef PlynxHandlers_h
#define PlynxHandlers_h

#include <Plynx/PlynxConfig.h>
#include <Plynx/PlynxParam.h>

/**
 * @name Virtual pins
 * The names V0, V1, V2 and so on, one for each virtual pin.
 * @{
 */

/**
 * @brief A virtual pin. It carries a number or a text between the board and
 * the app, in both directions.
 *
 * In the app you set a widget to a V pin. The board reads what the widget sends
 * in PLYNX_WRITE() and shows values on it with Plynx.virtualWrite(). Every
 * board has V0 to V31. ESP32, ESP8266, SAMD boards and Linux have V0 to V127
 * (PLYNX_USE_128_VPINS).
 *
 * @code
 * PLYNX_WRITE(V1)                // a Button in the app on V1
 * {
 *   digitalWrite(2, param.asInt());
 * }
 *
 * void sendLevel()               // a Gauge in the app on V2
 * {
 *   Plynx.virtualWrite(V2, analogRead(A0));
 * }
 * @endcode
 *
 * @warning On a board with 32 virtual pins, V32 and above do not compile ("was
 * not declared in this scope"). Pick a pin from V0 to V31 in the app.
 *
 * @see PLYNX_WRITE, PLYNX_READ, PlynxApi::virtualWrite, PLYNX_USE_128_VPINS
 */
#define V0  0
#define V1  1
#define V2  2
#define V3  3
#define V4  4
#define V5  5
#define V6  6
#define V7  7
#define V8  8
#define V9  9
#define V10 10
#define V11 11
#define V12 12
#define V13 13
#define V14 14
#define V15 15
#define V16 16
#define V17 17
#define V18 18
#define V19 19
#define V20 20
#define V21 21
#define V22 22
#define V23 23
#define V24 24
#define V25 25
#define V26 26
#define V27 27
#define V28 28
#define V29 29
#define V30 30
#define V31 31
#ifdef PLYNX_USE_128_VPINS
  #define V32 32
  #define V33 33
  #define V34 34
  #define V35 35
  #define V36 36
  #define V37 37
  #define V38 38
  #define V39 39
  #define V40 40
  #define V41 41
  #define V42 42
  #define V43 43
  #define V44 44
  #define V45 45
  #define V46 46
  #define V47 47
  #define V48 48
  #define V49 49
  #define V50 50
  #define V51 51
  #define V52 52
  #define V53 53
  #define V54 54
  #define V55 55
  #define V56 56
  #define V57 57
  #define V58 58
  #define V59 59
  #define V60 60
  #define V61 61
  #define V62 62
  #define V63 63
  #define V64 64
  #define V65 65
  #define V66 66
  #define V67 67
  #define V68 68
  #define V69 69
  #define V70 70
  #define V71 71
  #define V72 72
  #define V73 73
  #define V74 74
  #define V75 75
  #define V76 76
  #define V77 77
  #define V78 78
  #define V79 79
  #define V80 80
  #define V81 81
  #define V82 82
  #define V83 83
  #define V84 84
  #define V85 85
  #define V86 86
  #define V87 87
  #define V88 88
  #define V89 89
  #define V90 90
  #define V91 91
  #define V92 92
  #define V93 93
  #define V94 94
  #define V95 95
  #define V96 96
  #define V97 97
  #define V98 98
  #define V99 99
  #define V100 100
  #define V101 101
  #define V102 102
  #define V103 103
  #define V104 104
  #define V105 105
  #define V106 106
  #define V107 107
  #define V108 108
  #define V109 109
  #define V110 110
  #define V111 111
  #define V112 112
  #define V113 113
  #define V114 114
  #define V115 115
  #define V116 116
  #define V117 117
  #define V118 118
  #define V119 119
  #define V120 120
  #define V121 121
  #define V122 122
  #define V123 123
  #define V124 124
  #define V125 125
  #define V126 126
  #define V127 127
#endif

/** @} */

// Initial syntax:
#define PLYNX_WRITE_2(pin) \
    void PlynxWidgetWrite ## pin (PlynxReq PLYNX_UNUSED &request, const PlynxParam PLYNX_UNUSED &param)

#define PLYNX_READ_2(pin)  \
    void PlynxWidgetRead ## pin  (PlynxReq PLYNX_UNUSED &request)

/**
 * @brief Your code for the values the app sends to pins that have no
 * PLYNX_WRITE() of their own.
 *
 * `request.pin` is the pin and `param` is the value. Without it those values
 * are lost.
 *
 * @code
 * PLYNX_WRITE_DEFAULT()
 * {
 *   Serial.print("V");
 *   Serial.print(request.pin);
 *   Serial.print(" is now ");
 *   Serial.println(param.asStr());
 * }
 * @endcode
 *
 * @see PLYNX_WRITE, PLYNX_READ_DEFAULT
 */
#define PLYNX_WRITE_DEFAULT() PLYNX_WRITE_2(Default)

/**
 * @brief Your answer when the app asks for the value of a pin that has no
 * PLYNX_READ() of its own.
 *
 * `request.pin` is the pin. Answer with Plynx.virtualWrite() on that pin.
 *
 * @code
 * PLYNX_READ_DEFAULT()
 * {
 *   Plynx.virtualWrite(request.pin, millis() / 1000);
 * }
 * @endcode
 *
 * @see PLYNX_READ, PLYNX_WRITE_DEFAULT
 */
#define PLYNX_READ_DEFAULT()  PLYNX_READ_2(Default)

/**
 * @brief Your code that runs when the app sends a value to this pin, for
 * example when you press a Button or move a Slider.
 *
 * The new value is in `param`. Write it outside setup() and loop().
 *
 * @param pin  The virtual pin, such as V1.
 *
 * @code
 * PLYNX_WRITE(V1)
 * {
 *   int on = param.asInt();         // a Button sends 1 when on, 0 when off
 *   digitalWrite(2, on ? HIGH : LOW);
 * }
 *
 * PLYNX_WRITE(V2)
 * {
 *   int red   = param[0].asInt();   // an RGB Picker sends three values
 *   int green = param[1].asInt();
 *   int blue  = param[2].asInt();
 *   analogWrite(25, red);
 *   analogWrite(26, green);
 *   analogWrite(27, blue);
 * }
 * @endcode
 *
 * @warning Keep it short. A delay() or a long loop in here blocks the
 * connection. Start long work with a PlynxTimer.
 *
 * @warning One PLYNX_WRITE() per pin. Two for the same pin do not compile
 * ("redefinition").
 *
 * @see PlynxParam, PLYNX_READ, PlynxApi::syncVirtual, PLYNX_WRITE_DEFAULT
 */
#define PLYNX_WRITE(pin)      PLYNX_WRITE_2(pin)

/**
 * @brief Your answer when the app asks the board for the value of this pin.
 *
 * The app asks while the project is open on a phone, at the reading interval
 * of the widget (Value Display, Labeled Value, Gauge, Level Display, LCD).
 * Answer with Plynx.virtualWrite() on the same pin. Write it outside setup()
 * and loop().
 *
 * @param pin  The virtual pin, such as V1.
 *
 * @code
 * PLYNX_READ(V5)
 * {
 *   Plynx.virtualWrite(V5, millis() / 1000);   // the answer goes to V5
 * }
 * @endcode
 *
 * @warning Without a Plynx.virtualWrite() inside, the widget stays empty.
 *
 * @see PLYNX_WRITE, PlynxApi::virtualWrite, PLYNX_READ_DEFAULT
 */
#define PLYNX_READ(pin)       PLYNX_READ_2(pin)

// New, more readable syntax:
#define PLYNX_IN_2(pin)  \
    void PlynxWidgetWrite ## pin (PlynxReq PLYNX_UNUSED &request, const PlynxParam PLYNX_UNUSED &getValue)

#define PLYNX_OUT_2(pin) \
    void PlynxWidgetRead ## pin  (PlynxReq PLYNX_UNUSED &request)

/**
 * @brief PLYNX_WRITE_DEFAULT() with the value called `getValue`.
 *
 * @code
 * PLYNX_INPUT_DEFAULT()
 * {
 *   Serial.println(getValue.asStr());
 * }
 * @endcode
 *
 * @see PLYNX_WRITE_DEFAULT
 */
#define PLYNX_INPUT_DEFAULT()   PLYNX_IN_2(Default)

/**
 * @brief PLYNX_READ_DEFAULT() under its old name.
 *
 * @code
 * PLYNX_OUTPUT_DEFAULT()
 * {
 *   Plynx.virtualWrite(request.pin, 0);
 * }
 * @endcode
 *
 * @see PLYNX_READ_DEFAULT
 */
#define PLYNX_OUTPUT_DEFAULT()  PLYNX_OUT_2(Default)

/**
 * @brief PLYNX_WRITE() with the value called `getValue`.
 *
 * @param pin  The virtual pin, such as V1.
 *
 * @code
 * PLYNX_INPUT(V1)
 * {
 *   digitalWrite(2, getValue.asInt());
 * }
 * @endcode
 *
 * @see PLYNX_WRITE
 */
#define PLYNX_INPUT(pin)        PLYNX_IN_2(pin)

/**
 * @brief PLYNX_READ() under its old name.
 *
 * @param pin  The virtual pin, such as V1.
 *
 * @code
 * PLYNX_OUTPUT(V5)
 * {
 *   Plynx.virtualWrite(V5, analogRead(A0));
 * }
 * @endcode
 *
 * @see PLYNX_READ
 */
#define PLYNX_OUTPUT(pin)       PLYNX_OUT_2(pin)

// Additional handlers

/**
 * @brief Your code that runs each time the board connects, the first time and
 * after every reconnect.
 *
 * Here the board gets back the values of the app with Plynx.syncAll() and
 * sends its own state.
 *
 * @code
 * PLYNX_CONNECTED()
 * {
 *   Plynx.syncAll();                    // the Buttons go back in place
 *   Plynx.virtualWrite(V9, "online");
 * }
 * @endcode
 *
 * @warning Keep it short. A long task here delays the first values from the
 * app.
 *
 * @see PLYNX_DISCONNECTED, PLYNX_APP_CONNECTED, PlynxApi::syncAll
 */
#define PLYNX_CONNECTED()    void PlynxOnConnected()

/**
 * @brief Your code that runs when the board loses the connection. The board
 * then reconnects by itself.
 *
 * @code
 * PLYNX_DISCONNECTED()
 * {
 *   digitalWrite(2, LOW);     // an LED on pin 2 shows the link is down
 * }
 * @endcode
 *
 * @note It does not run after Plynx.disconnect(). Values sent while offline
 * are lost.
 *
 * @see PLYNX_CONNECTED, PlynxProtocol::connected
 */
#define PLYNX_DISCONNECTED() void PlynxOnDisconnected()

// Advanced functions

/**
 * @brief An `int` variable that the app can set and read on a virtual pin,
 * with no other code.
 *
 * @param name  The name of the variable.
 * @param pin   The virtual pin, such as V1.
 *
 * @code
 * PLYNX_VAR_INT(threshold, V3)
 *
 * void checkLight()
 * {
 *   digitalWrite(2, analogRead(A0) > threshold ? HIGH : LOW);
 * }
 * @endcode
 *
 * @note It takes the PLYNX_WRITE() and PLYNX_READ() of that pin, so the sketch
 * cannot have its own for the same pin.
 *
 * @see PLYNX_VAR_LONG, PLYNX_VAR_DOUBLE, PLYNX_VAR_STRING
 */
#define PLYNX_VAR_INT(name, pin) \
    int name;  \
    PLYNX_WRITE(pin) { name = param.asInt(); } \
    PLYNX_READ(pin)  { Plynx.virtualWrite(pin, name); }

/**
 * @brief A `long` variable that the app can set and read on a virtual pin, for
 * numbers too large for an `int`.
 *
 * @param name  The name of the variable.
 * @param pin   The virtual pin, such as V1.
 *
 * @code
 * PLYNX_VAR_LONG(intervalMs, V4)
 * @endcode
 *
 * @see PLYNX_VAR_INT
 */
#define PLYNX_VAR_LONG(name, pin) \
    long name;  \
    PLYNX_WRITE(pin) { name = param.asLong(); } \
    PLYNX_READ(pin)  { Plynx.virtualWrite(pin, name); }

#ifndef PLYNX_NO_FLOAT
/**
 * @brief A `double` variable that the app can set and read on a virtual pin,
 * for numbers with decimals. Missing with PLYNX_NO_FLOAT.
 *
 * @param name  The name of the variable.
 * @param pin   The virtual pin, such as V1.
 *
 * @code
 * PLYNX_VAR_DOUBLE(targetCelsius, V6)
 * @endcode
 *
 * @see PLYNX_VAR_INT
 */
#define PLYNX_VAR_DOUBLE(name, pin) \
    double name;  \
    PLYNX_WRITE(pin) { name = param.asDouble(); } \
    PLYNX_READ(pin)  { Plynx.virtualWrite(pin, name); }
#endif

#ifdef ARDUINO
/**
 * @brief A `String` variable that the app can set and read on a virtual pin,
 * for example the text of a Text Input.
 *
 * @param name  The name of the variable.
 * @param pin   The virtual pin, such as V1.
 *
 * @code
 * PLYNX_VAR_STRING(greeting, V7)
 * @endcode
 *
 * @see PLYNX_VAR_INT
 */
#define PLYNX_VAR_STRING(name, pin) \
    String name;  \
    PLYNX_WRITE(pin) { name = param.asStr(); } \
    PLYNX_READ(pin)  { Plynx.virtualWrite(pin, name); }
#endif

// Default read/write handlers (you can redefine them in your code)
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief `request.pin` tells which pin your PLYNX_WRITE_DEFAULT() or
 * PLYNX_READ_DEFAULT() code is answering.
 *
 * @code
 * PLYNX_WRITE_DEFAULT()
 * {
 *   int pin = request.pin;               // the virtual pin the app wrote
 *   Plynx.virtualWrite(pin, param.asInt());
 * }
 * @endcode
 *
 * @see PLYNX_WRITE_DEFAULT, PLYNX_READ_DEFAULT
 */
struct PlynxReq
{
    uint8_t pin;   ///< The number of the virtual pin, 1 for V1.
};

typedef void (*WidgetReadHandler)(PlynxReq PLYNX_UNUSED &request);
typedef void (*WidgetWriteHandler)(PlynxReq PLYNX_UNUSED &request, const PlynxParam PLYNX_UNUSED &param);

WidgetReadHandler GetReadHandler(uint8_t pin);
WidgetWriteHandler GetWriteHandler(uint8_t pin);

// Declare placeholders
PLYNX_READ();
PLYNX_WRITE();
void PlynxNoOpCbk();

// Declare all pin handlers (you can redefine them in your code)
PLYNX_CONNECTED();
PLYNX_DISCONNECTED();

// Internal Virtual Pins
PLYNX_WRITE(InternalPinACON);
PLYNX_WRITE(InternalPinADIS);
PLYNX_WRITE(InternalPinRTC);
PLYNX_WRITE(InternalPinUTC);
PLYNX_WRITE(InternalPinOTA);
PLYNX_WRITE(InternalPinMETA);
PLYNX_WRITE(InternalPinVFS);
PLYNX_WRITE(InternalPinDBG);

// Aliases

/**
 * @brief Your code that runs when someone opens the project in the app, to
 * send fresh values only while someone is looking.
 *
 * It runs only in a project with the `isAppConnectedOn` setting on, and the
 * Plynx app has no switch for it yet, so in a project made in the app it does not
 * run.
 *
 * @code
 * bool someoneIsWatching = false;
 *
 * PLYNX_APP_CONNECTED()
 * {
 *   someoneIsWatching = true;
 * }
 *
 * PLYNX_APP_DISCONNECTED()
 * {
 *   someoneIsWatching = false;
 * }
 * @endcode
 *
 * @see PLYNX_APP_DISCONNECTED, PLYNX_CONNECTED
 */
#define PLYNX_APP_CONNECTED()    PLYNX_WRITE(InternalPinACON)

/**
 * @brief Your code that runs when the app closes the project. Like
 * PLYNX_APP_CONNECTED(), only with `isAppConnectedOn` on.
 *
 * @code
 * PLYNX_APP_DISCONNECTED()
 * {
 *   Serial.println("Nobody is looking at the app");
 * }
 * @endcode
 *
 * @see PLYNX_APP_CONNECTED
 */
#define PLYNX_APP_DISCONNECTED() PLYNX_WRITE(InternalPinADIS)

// Regular Virtual Pins
PLYNX_READ_DEFAULT();
PLYNX_WRITE_DEFAULT();

PLYNX_READ(0 );
PLYNX_READ(1 );
PLYNX_READ(2 );
PLYNX_READ(3 );
PLYNX_READ(4 );
PLYNX_READ(5 );
PLYNX_READ(6 );
PLYNX_READ(7 );
PLYNX_READ(8 );
PLYNX_READ(9 );
PLYNX_READ(10);
PLYNX_READ(11);
PLYNX_READ(12);
PLYNX_READ(13);
PLYNX_READ(14);
PLYNX_READ(15);
PLYNX_READ(16);
PLYNX_READ(17);
PLYNX_READ(18);
PLYNX_READ(19);
PLYNX_READ(20);
PLYNX_READ(21);
PLYNX_READ(22);
PLYNX_READ(23);
PLYNX_READ(24);
PLYNX_READ(25);
PLYNX_READ(26);
PLYNX_READ(27);
PLYNX_READ(28);
PLYNX_READ(29);
PLYNX_READ(30);
PLYNX_READ(31);
#ifdef PLYNX_USE_128_VPINS
  PLYNX_READ(32);
  PLYNX_READ(33);
  PLYNX_READ(34);
  PLYNX_READ(35);
  PLYNX_READ(36);
  PLYNX_READ(37);
  PLYNX_READ(38);
  PLYNX_READ(39);
  PLYNX_READ(40);
  PLYNX_READ(41);
  PLYNX_READ(42);
  PLYNX_READ(43);
  PLYNX_READ(44);
  PLYNX_READ(45);
  PLYNX_READ(46);
  PLYNX_READ(47);
  PLYNX_READ(48);
  PLYNX_READ(49);
  PLYNX_READ(50);
  PLYNX_READ(51);
  PLYNX_READ(52);
  PLYNX_READ(53);
  PLYNX_READ(54);
  PLYNX_READ(55);
  PLYNX_READ(56);
  PLYNX_READ(57);
  PLYNX_READ(58);
  PLYNX_READ(59);
  PLYNX_READ(60);
  PLYNX_READ(61);
  PLYNX_READ(62);
  PLYNX_READ(63);
  PLYNX_READ(64);
  PLYNX_READ(65);
  PLYNX_READ(66);
  PLYNX_READ(67);
  PLYNX_READ(68);
  PLYNX_READ(69);
  PLYNX_READ(70);
  PLYNX_READ(71);
  PLYNX_READ(72);
  PLYNX_READ(73);
  PLYNX_READ(74);
  PLYNX_READ(75);
  PLYNX_READ(76);
  PLYNX_READ(77);
  PLYNX_READ(78);
  PLYNX_READ(79);
  PLYNX_READ(80);
  PLYNX_READ(81);
  PLYNX_READ(82);
  PLYNX_READ(83);
  PLYNX_READ(84);
  PLYNX_READ(85);
  PLYNX_READ(86);
  PLYNX_READ(87);
  PLYNX_READ(88);
  PLYNX_READ(89);
  PLYNX_READ(90);
  PLYNX_READ(91);
  PLYNX_READ(92);
  PLYNX_READ(93);
  PLYNX_READ(94);
  PLYNX_READ(95);
  PLYNX_READ(96);
  PLYNX_READ(97);
  PLYNX_READ(98);
  PLYNX_READ(99);
  PLYNX_READ(100);
  PLYNX_READ(101);
  PLYNX_READ(102);
  PLYNX_READ(103);
  PLYNX_READ(104);
  PLYNX_READ(105);
  PLYNX_READ(106);
  PLYNX_READ(107);
  PLYNX_READ(108);
  PLYNX_READ(109);
  PLYNX_READ(110);
  PLYNX_READ(111);
  PLYNX_READ(112);
  PLYNX_READ(113);
  PLYNX_READ(114);
  PLYNX_READ(115);
  PLYNX_READ(116);
  PLYNX_READ(117);
  PLYNX_READ(118);
  PLYNX_READ(119);
  PLYNX_READ(120);
  PLYNX_READ(121);
  PLYNX_READ(122);
  PLYNX_READ(123);
  PLYNX_READ(124);
  PLYNX_READ(125);
  PLYNX_READ(126);
  PLYNX_READ(127);
#endif

PLYNX_WRITE(0 );
PLYNX_WRITE(1 );
PLYNX_WRITE(2 );
PLYNX_WRITE(3 );
PLYNX_WRITE(4 );
PLYNX_WRITE(5 );
PLYNX_WRITE(6 );
PLYNX_WRITE(7 );
PLYNX_WRITE(8 );
PLYNX_WRITE(9 );
PLYNX_WRITE(10);
PLYNX_WRITE(11);
PLYNX_WRITE(12);
PLYNX_WRITE(13);
PLYNX_WRITE(14);
PLYNX_WRITE(15);
PLYNX_WRITE(16);
PLYNX_WRITE(17);
PLYNX_WRITE(18);
PLYNX_WRITE(19);
PLYNX_WRITE(20);
PLYNX_WRITE(21);
PLYNX_WRITE(22);
PLYNX_WRITE(23);
PLYNX_WRITE(24);
PLYNX_WRITE(25);
PLYNX_WRITE(26);
PLYNX_WRITE(27);
PLYNX_WRITE(28);
PLYNX_WRITE(29);
PLYNX_WRITE(30);
PLYNX_WRITE(31);
#ifdef PLYNX_USE_128_VPINS
  PLYNX_WRITE(32);
  PLYNX_WRITE(33);
  PLYNX_WRITE(34);
  PLYNX_WRITE(35);
  PLYNX_WRITE(36);
  PLYNX_WRITE(37);
  PLYNX_WRITE(38);
  PLYNX_WRITE(39);
  PLYNX_WRITE(40);
  PLYNX_WRITE(41);
  PLYNX_WRITE(42);
  PLYNX_WRITE(43);
  PLYNX_WRITE(44);
  PLYNX_WRITE(45);
  PLYNX_WRITE(46);
  PLYNX_WRITE(47);
  PLYNX_WRITE(48);
  PLYNX_WRITE(49);
  PLYNX_WRITE(50);
  PLYNX_WRITE(51);
  PLYNX_WRITE(52);
  PLYNX_WRITE(53);
  PLYNX_WRITE(54);
  PLYNX_WRITE(55);
  PLYNX_WRITE(56);
  PLYNX_WRITE(57);
  PLYNX_WRITE(58);
  PLYNX_WRITE(59);
  PLYNX_WRITE(60);
  PLYNX_WRITE(61);
  PLYNX_WRITE(62);
  PLYNX_WRITE(63);
  PLYNX_WRITE(64);
  PLYNX_WRITE(65);
  PLYNX_WRITE(66);
  PLYNX_WRITE(67);
  PLYNX_WRITE(68);
  PLYNX_WRITE(69);
  PLYNX_WRITE(70);
  PLYNX_WRITE(71);
  PLYNX_WRITE(72);
  PLYNX_WRITE(73);
  PLYNX_WRITE(74);
  PLYNX_WRITE(75);
  PLYNX_WRITE(76);
  PLYNX_WRITE(77);
  PLYNX_WRITE(78);
  PLYNX_WRITE(79);
  PLYNX_WRITE(80);
  PLYNX_WRITE(81);
  PLYNX_WRITE(82);
  PLYNX_WRITE(83);
  PLYNX_WRITE(84);
  PLYNX_WRITE(85);
  PLYNX_WRITE(86);
  PLYNX_WRITE(87);
  PLYNX_WRITE(88);
  PLYNX_WRITE(89);
  PLYNX_WRITE(90);
  PLYNX_WRITE(91);
  PLYNX_WRITE(92);
  PLYNX_WRITE(93);
  PLYNX_WRITE(94);
  PLYNX_WRITE(95);
  PLYNX_WRITE(96);
  PLYNX_WRITE(97);
  PLYNX_WRITE(98);
  PLYNX_WRITE(99);
  PLYNX_WRITE(100);
  PLYNX_WRITE(101);
  PLYNX_WRITE(102);
  PLYNX_WRITE(103);
  PLYNX_WRITE(104);
  PLYNX_WRITE(105);
  PLYNX_WRITE(106);
  PLYNX_WRITE(107);
  PLYNX_WRITE(108);
  PLYNX_WRITE(109);
  PLYNX_WRITE(110);
  PLYNX_WRITE(111);
  PLYNX_WRITE(112);
  PLYNX_WRITE(113);
  PLYNX_WRITE(114);
  PLYNX_WRITE(115);
  PLYNX_WRITE(116);
  PLYNX_WRITE(117);
  PLYNX_WRITE(118);
  PLYNX_WRITE(119);
  PLYNX_WRITE(120);
  PLYNX_WRITE(121);
  PLYNX_WRITE(122);
  PLYNX_WRITE(123);
  PLYNX_WRITE(124);
  PLYNX_WRITE(125);
  PLYNX_WRITE(126);
  PLYNX_WRITE(127);
#endif

#ifdef __cplusplus
}
#endif

#endif
