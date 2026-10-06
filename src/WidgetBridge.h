/**
 * @file       WidgetBridge.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Apr 2015
 * @brief
 *
 */

#ifndef WidgetBridge_h
#define WidgetBridge_h

#include <Plynx/PlynxWidgetBase.h>

/**
 * @brief Lets this board write the pins of another board of the same account,
 * through the server.
 *
 * The other board is chosen by its auth token. The virtual pin given to the
 * bridge is only a channel number on this board, so each bridge needs its
 * own pin, used by nothing else. What this board writes to a virtual pin of
 * the other board runs there in its PLYNX_WRITE().
 *
 * Create it at the top level, set the token in PLYNX_CONNECTED(), write from
 * a timer or a handler.
 *
 * No widget is needed. The other board has to be online.
 *
 * @code
 * WidgetBridge garage(V20);
 *
 * PLYNX_CONNECTED()
 * {
 *   garage.setAuthToken("TokenOfTheOtherBoard");
 * }
 *
 * PLYNX_WRITE(V1)                       // a Button on this board's project
 * {
 *   garage.virtualWrite(V5, param.asInt());   // runs PLYNX_WRITE(V5) there
 * }
 * @endcode
 *
 * @warning The link lasts as long as the connection. Set the token in
 * PLYNX_CONNECTED(), so it is set again after every reconnect. Set in setup(),
 * before the board is connected, it is lost.
 *
 * @see setAuthToken, virtualWrite
 */
class WidgetBridge
    : private PlynxWidgetBase
{
public:
    /**
     * @brief Creates a bridge on a virtual pin of this board used only as its
     * channel.
     * @param vPin  A virtual pin of this board that nothing else uses, such as V20.
     *
     * @code
     * WidgetBridge garage(V20);
     * @endcode
     */
    WidgetBridge(uint8_t vPin)
        : PlynxWidgetBase(vPin)
    {}

    /**
     * @brief Chooses the other board, by its auth token.
     *
     * @param token  The 32-character auth token of the other board. It has to
     *               belong to the same Plynx account.
     *
     * Call it inside PLYNX_CONNECTED().
     *
     * @code
     * WidgetBridge garage(V20);
     *
     * PLYNX_CONNECTED()
     * {
     *   garage.setAuthToken("TokenOfTheOtherBoard");
     * }
     * @endcode
     */
    void setAuthToken(const char* token) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(mPin);
        cmd.add("i");
        cmd.add(token);
        Plynx.sendCmd(PLYNX_CMD_BRIDGE, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * @brief Sets a digital pin of the other board HIGH or LOW.
     *
     * @param pin  The pin number on the other board.
     * @param val  1 or HIGH for high, 0 or LOW for low.
     *
     * @code
     * WidgetBridge garage(V20);
     *
     * void openDoor()
     * {
     *   garage.digitalWrite(5, HIGH);
     * }
     * @endcode
     *
     * @see virtualWrite
     */
    template <typename T>
    void digitalWrite(const T& pin, int val) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(mPin);
        cmd.add("dw");
        cmd.add(pin);
        cmd.add(val);
        Plynx.sendCmd(PLYNX_CMD_BRIDGE, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * @brief Sets the PWM output of a pin of the other board.
     *
     * @param pin  The pin number on the other board.
     * @param val  The duty cycle, usually from 0 to 255.
     *
     * @code
     * WidgetBridge lamp(V21);
     *
     * PLYNX_WRITE(V2)
     * {
     *   lamp.analogWrite(9, param.asInt());
     * }
     * @endcode
     *
     * @warning On an ESP32 board on the other side the library does not
     * handle this command. Write a virtual pin there instead.
     */
    template <typename T>
    void analogWrite(const T& pin, int val) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(mPin);
        cmd.add("aw");
        cmd.add(pin);
        cmd.add(val);
        Plynx.sendCmd(PLYNX_CMD_BRIDGE, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * @brief Writes one or more values to a virtual pin of the other board,
     * which runs its PLYNX_WRITE() for that pin.
     *
     * @param pin     The virtual pin on the other board.
     * @param values  One or more values.
     *
     * @code
     * WidgetBridge heater(V22);
     *
     * void askForHeat(float target)
     * {
     *   heater.virtualWrite(V3, target);
     * }
     * @endcode
     *
     * @see PLYNX_WRITE
     */
    template <typename... Args>
    void virtualWrite(int pin, Args... values) {
        char mem[PLYNX_MAX_SENDBYTES];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(mPin);
        cmd.add("vw");
        cmd.add(pin);
        cmd.add_multi(values...);
        Plynx.sendCmd(PLYNX_CMD_BRIDGE, 0, cmd.getBuffer(), cmd.getLength()-1);
    }

    /**
     * @brief Writes raw bytes to a virtual pin of the other board.
     *
     * @param pin   The virtual pin on the other board.
     * @param buff  The first byte.
     * @param len   How many bytes.
     *
     * @code
     * WidgetBridge display(V23);
     *
     * void sendPair()
     * {
     *   const char packed[] = "12\0" "34";
     *   display.virtualWriteBinary(V4, packed, sizeof(packed) - 1);
     * }
     * @endcode
     */
    void virtualWriteBinary(int pin, const void* buff, size_t len) {
        char mem[8];
        PlynxParam cmd(mem, 0, sizeof(mem));
        cmd.add(mPin);
        cmd.add("vw");
        cmd.add(pin);
        Plynx.sendCmd(PLYNX_CMD_BRIDGE, 0, cmd.getBuffer(), cmd.getLength(), buff, len);
    }

    /**
     * @brief Writes a list built with PlynxParam to a virtual pin of the other
     * board.
     *
     * @param pin    The virtual pin on the other board.
     * @param param  The values.
     *
     * @code
     * WidgetBridge display(V23);
     *
     * void forward()
     * {
     *   char buffer[32];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add(1);
     *   list.add("on");
     *   display.virtualWrite(V4, list);
     * }
     * @endcode
     */
    void virtualWrite(int pin, const PlynxParam& param) {
        virtualWriteBinary(pin, param.getBuffer(), param.getLength()-1);
    }

};

#endif
