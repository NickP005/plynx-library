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

class PlynxStream
    : public PlynxProtocol<PlynxTransportStream>
{
    typedef PlynxProtocol<PlynxTransportStream> Base;
public:
    PlynxStream(PlynxTransportStream& transp)
        : Base(transp)
    {}

    void config(Stream&     stream,
                const char* auth)
    {
        Base::begin(auth);
        this->conn.begin(stream);
    }

    void begin(Stream& stream, const char* auth) {
        config(stream, auth);
        while(this->connect() != true) {}
    }
};

#endif
