/**
 * @file       WidgetTerminal.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 */

#ifndef WidgetTerminal_h
#define WidgetTerminal_h

#if defined(ARDUINO) && !(defined(LINUX) || defined(__MBED__))
    #define PLYNX_USE_PRINT_CLASS
#elif defined(SPARK) || defined(PARTICLE)
    #define PLYNX_USE_PRINT_CLASS
#endif

#include <Plynx/PlynxWidgetBase.h>

#ifdef PLYNX_USE_PRINT_CLASS
    #if !(defined(SPARK) || defined(PARTICLE) || (PLATFORM_ID==88) || defined(ARDUINO_RedBear_Duo)) // 88 -> RBL Duo
        // On Particle this is auto-included
        #include <Print.h>
    #endif
#endif

/**
 * @brief Writes text to the Terminal widget of the app, like the Serial
 * Monitor, and reads what the person types there.
 *
 * It works like `Serial`. print() and println() collect the text on the board,
 * and flush() sends what was collected. Each send becomes one entry in the
 * Terminal, which keeps the last 100. What the person types in the Terminal
 * arrives in the PLYNX_WRITE() of the same pin.
 *
 * @par Where to call
 * Create it at the top level with the pin of the Terminal widget. Print from
 * a timer function, from a handler, or from setup().
 *
 * @par App widget
 * Terminal.
 *
 * @code
 * WidgetTerminal terminal(V9);
 *
 * PLYNX_WRITE(V9)                      // the person typed a line
 * {
 *   if (String("hello") == param.asStr()) {
 *     terminal.println("Hello to you");
 *   } else {
 *     terminal.print("You wrote ");
 *     terminal.println(param.asStr());
 *   }
 *   terminal.flush();                  // send it now
 * }
 * @endcode
 *
 * @warning Without flush() the text waits on the board until 64 characters
 * have piled up. Call flush() after the last print().
 *
 * @warning A print() at every turn of loop() floods the connection. Print
 * from a timer or when something changes.
 *
 * @see flush, clear, PLYNX_WRITE
 */
class WidgetTerminal
    : public PlynxWidgetBase
#ifdef PLYNX_USE_PRINT_CLASS
    , public Print
#endif
{
public:
    /**
     * @brief Ties the terminal object to the virtual pin of the Terminal
     * widget.
     * @param vPin  The virtual pin, written V0, V1, V2 and so on.
     *
     * @code
     * WidgetTerminal terminal(V9);
     * @endcode
     */
    WidgetTerminal(uint8_t vPin)
        : PlynxWidgetBase(vPin)
        , mOutQty(0)
    {}

    //virtual ~WidgetTerminal() {}

    /**
     * @brief Adds one character to the text waiting to be sent.
     *
     * print() and println() call it for every character. When 64 characters
     * are waiting it sends them by itself.
     *
     * @param byte  The character.
     * @return Always 1.
     *
     * @code
     * WidgetTerminal terminal(V9);
     *
     * void sendStar()
     * {
     *   terminal.write('*');
     *   terminal.flush();
     * }
     * @endcode
     *
     * @see flush
     */
    virtual size_t write(uint8_t byte) {
        mOutBuf[mOutQty++] = byte;
        if (mOutQty >= sizeof(mOutBuf)) {
            flush();
        }
        return 1;
    }

    /**
     * @brief Sends to the app the text collected with print() and println().
     *
     * Nothing happens when no text is waiting.
     *
     * @par Where to call
     * After the last print() of a message.
     *
     * @par App widget
     * Terminal, which shows the text as one entry.
     *
     * @code
     * WidgetTerminal terminal(V9);
     *
     * void report()
     * {
     *   terminal.print("Uptime ");
     *   terminal.print(millis() / 1000);
     *   terminal.println(" s");
     *   terminal.flush();
     * }
     * @endcode
     *
     * @note While the board is offline the text is lost.
     *
     * @see clear
     */
    virtual void flush() {
        if (mOutQty) {
            Plynx.virtualWriteBinary(mPin, mOutBuf, mOutQty);
            mOutQty = 0;
        }
    }
    
    /**
     * @brief Sends what is waiting, then sends the word `clr`, the command
     * that asks the Terminal to empty its screen.
     *
     * @par App widget
     * Terminal.
     *
     * @code
     * WidgetTerminal terminal(V9);
     *
     * PLYNX_CONNECTED()
     * {
     *   terminal.clear();
     *   terminal.println("Board restarted");
     *   terminal.flush();
     * }
     * @endcode
     *
     * @warning The Plynx app (iOS, October 2026) shows `clr` as a line of
     * text instead of emptying the Terminal.
     *
     * @see flush
     */
    void clear() {
        flush();
        Plynx.virtualWrite(mPin, "clr");
    }

#ifdef PLYNX_USE_PRINT_CLASS

    using Print::write;

    /**
     * @brief Adds several characters to the text waiting to be sent.
     *
     * @param buff  The first character.
     * @param len   How many characters.
     * @return How many characters were added.
     *
     * @code
     * WidgetTerminal terminal(V9);
     *
     * void sendPart()
     * {
     *   const char text[] = "temperature";
     *   terminal.write(text, 4);   // "temp"
     *   terminal.flush();
     * }
     * @endcode
     */
    virtual size_t write(const void* buff, size_t len) {
        return write((char*)buff, len);
    }

#else

    virtual size_t write(const void* buff, size_t len) {
        uint8_t* buf = (uint8_t*)buff;
        size_t left = len;
        while (left--) {
            write(*buf++);
        }
        return len;
    }

    virtual size_t write(const char* str) {
        return write(str, strlen(str));
    }

#endif

private:
    uint8_t mOutBuf[64];
    uint8_t mOutQty;
};

#endif
