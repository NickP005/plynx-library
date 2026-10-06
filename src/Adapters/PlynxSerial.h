/**
 * @file       PlynxSerial.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief
 *
 */

#ifndef PlynxStream_h
#define PlynxStream_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION "Serial"
#endif

#include <PlynxApiArduino.h>
#include <Plynx/PlynxProtocol.h>

class PlynxTransportStream
{
public:
    PlynxTransportStream()
        : stream(NULL), conn(0)
    {}

    // IP redirect not available
    void begin(char PLYNX_UNUSED *h, uint16_t PLYNX_UNUSED p) {}

    void begin(Stream& s) {
        stream = &s;
    }

    bool connect() {
        PLYNX_LOG1(PLYNX_F("Connecting..."));
        stream->flush();
        return conn = true;
    }
    void disconnect() { conn = false; }

    size_t read(void* buf, size_t len) {
        char* beg = (char*)buf;
        char* end = beg + len;
        millis_time_t startMillis = PlynxMillis();
        while ((beg < end) && (PlynxMillis() - startMillis < PLYNX_TIMEOUT_MS)) {
            int c = stream->read();
            if (c < 0)
                continue;
            *beg++ = (char)c;
        }
        return beg-(char*)buf;
    }
    size_t write(const void* buf, size_t len) {
        stream->write((const uint8_t*)buf, len);
        return len;
    }

    bool connected() { return conn; }
    int available() { return stream->available(); }

protected:
    Stream* stream;
    bool    conn;
};

/**
 * @brief The Plynx object of a board that talks to the server through a
 * serial port, such as the USB cable or a Bluetooth serial module.
 *
 * `<PlynxSimpleStream.h>` and `<PlynxSimpleSerialBLE.h>` create it as the
 * global `Plynx`. On the USB cable, the script scripts/plynx-ser.sh, running
 * on the computer, carries the data between the port and the server.
 *
 * @code
 * #include <PlynxSimpleStream.h>
 *
 * char auth[] = "YourAuthToken";
 *
 * void setup()
 * {
 *   Serial.begin(9600);
 *   Plynx.begin(Serial, auth);
 * }
 *
 * void loop()
 * {
 *   Plynx.run();
 * }
 * @endcode
 *
 * @warning The port carries the Plynx data, so the sketch cannot print on it.
 * PLYNX_PRINT has to use another port, or stay off.
 *
 * @see begin
 */
class PlynxStream
    : public PlynxProtocol<PlynxTransportStream>
{
    typedef PlynxProtocol<PlynxTransportStream> Base;
public:
    PlynxStream(PlynxTransportStream& transp)
        : Base(transp)
    {}

    /**
     * @brief Sets the port and the token, without connecting yet.
     *
     * @param stream  The serial port that carries the data, such as Serial.
     * @param auth    The auth token of the board, from the Plynx app.
     *
     * @code
     * #include <PlynxSimpleStream.h>
     *
     * void setup()
     * {
     *   Serial.begin(9600);
     *   Plynx.config(Serial, "YourAuthToken");
     *   Plynx.connect();
     * }
     * @endcode
     *
     * @see begin
     */
    void config(Stream&     stream,
                const char* auth)
    {
        Base::begin(auth);
        this->conn.begin(stream);
    }

    /**
     * @brief Connects to the server through a serial port, and waits until it
     * is done.
     *
     * @param stream  The serial port that carries the data, such as Serial.
     * @param auth    The auth token of the board, from the Plynx app.
     *
     * Call it in setup(), after the port is started with begin().
     *
     * @code
     * #include <PlynxSimpleStream.h>
     *
     * void setup()
     * {
     *   Serial.begin(9600);
     *   Plynx.begin(Serial, "YourAuthToken");
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @warning It waits without a time limit, until the other side answers.
     *
     * @see config
     */
    void begin(Stream& stream, const char* auth) {
        config(stream, auth);
        while(this->connect() != true) {}
    }
};

#endif
