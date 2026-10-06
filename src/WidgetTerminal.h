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
 * @brief Prints text in the Terminal widget of the app, like the Serial
 * Monitor, and reads what you type there.
 *
 * print() and println() collect the text, flush() sends it. The Terminal keeps
 * the last 100 messages. What you type in the Terminal arrives in the
 * PLYNX_WRITE() of the same pin.
 *
 * @code
 * WidgetTerminal terminal(V9);
 *
 * PLYNX_WRITE(V9)                      // you typed a line
 * {
 *   terminal.print("You wrote ");
 *   terminal.println(param.asStr());
 *   terminal.flush();                  // send it now
 * }
 * @endcode
 *
 * @warning Without flush() the text waits on the board until 64 characters
 * pile up. Call flush() after the last print().
 *
 * @warning Print from a timer or when something changes. A print() at every
 * turn of loop() floods the connection.
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
     * @brief The terminal on the virtual pin of your Terminal widget.
     * @param vPin  The virtual pin, such as V9.
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
     * @brief Adds one character to the text waiting to be sent. At 64 characters
     * it sends them by itself.
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
     * @brief Sends to the Terminal the text collected with print() and println().
     *
     * Call it after the last print() of a message.
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
     * @brief Asks the Terminal to empty its screen.
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
     * @warning The Plynx app for iPhone (October 2026) shows the word `clr` as a
     * line instead of emptying the Terminal.
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
