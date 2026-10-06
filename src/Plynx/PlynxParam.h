/**
 * @file       PlynxParam.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Container for handler parameters
 *
 */

#ifndef PlynxParam_h
#define PlynxParam_h

#include <string.h>
#include <stdlib.h>
#include <Plynx/PlynxConfig.h>
#include <Plynx/PlynxDebug.h>

#define PLYNX_PARAM_KV(k, v) k "\0" v "\0"
#define PLYNX_PARAM_PLACEHOLDER_64 "PlaceholderPlaceholderPlaceholderPlaceholderPlaceholderPlaceholder"

#if !defined(PLYNX_NO_FLOAT)
extern char*        dtostrf_internal(double number, signed char width, unsigned char prec, char *s);
#endif

#if !defined(PLYNX_NO_LONGLONG)
extern long long    atoll_internal(const char *s);
#endif

/**
 * @brief The value that arrived from the app, available as `param` inside
 * PLYNX_WRITE().
 *
 * A value travels as text. `param` reads it as the type the sketch needs,
 * with asInt(), asFloat(), asStr() and the others. Some widgets send more than
 * one value at once (an RGB Picker sends three, a Joystick two), and
 * `param[0]`, `param[1]`, `param[2]` read each of them.
 *
 * The same class builds lists of values to send, with add(), for
 * Plynx.virtualWrite() and Plynx.setProperty().
 *
 * @par Where to call
 * Inside PLYNX_WRITE(), where `param` exists. Building a list works anywhere.
 *
 * @par App widget
 * Every widget that writes a virtual pin.
 *
 * @code
 * PLYNX_WRITE(V1)
 * {
 *   int x = param[0].asInt();   // a Joystick sends x and y
 *   int y = param[1].asInt();
 *   Serial.print(x);
 *   Serial.print(" ");
 *   Serial.println(y);
 * }
 * @endcode
 *
 * @warning The text belongs to the library and is gone when the handler
 * ends. Copy it into a `String` or an array to keep it.
 *
 * @see PLYNX_WRITE, asInt, asStr, operator[]
 */
class PlynxParam
{
public:
    /**
     * @brief One value inside a PlynxParam, as returned by `param[1]` or by a
     * loop over the values.
     *
     * It reads the value with the same functions as PlynxParam (asInt(),
     * asFloat(), asStr()...), and ++ moves it to the next value.
     *
     * @code
     * PLYNX_WRITE(V3)
     * {
     *   for (PlynxParam::iterator it = param.begin(); it < param.end(); ++it) {
     *     Serial.println(it.asStr());   // every value, one per line
     *   }
     * }
     * @endcode
     *
     * @see PlynxParam::begin, PlynxParam::operator[]
     */
    class iterator
    {
    public:
        iterator(const char* c, const char* l) : ptr(c), limit(l) {}
        static iterator invalid() { return iterator(NULL, NULL); }

        operator const char* () const   { return asStr(); }
        operator int () const           { return asInt(); }

        /**
         * @brief Reads this value as text.
         * @return The text, or NULL when the value does not exist.
         *
         * @code
         * PLYNX_WRITE(V3)
         * {
         *   if (param[1].isValid()) {
         *     Serial.println(param[1].asStr());
         *   }
         * }
         * @endcode
         *
         * @warning It is NULL for a value that does not exist. Functions such
         * as strcmp() crash on NULL, so check isValid() first.
         */
        const char* asStr() const       { return ptr; }

        /**
         * @brief Another name for asStr().
         * @return The text, or NULL when the value does not exist.
         *
         * @code
         * PLYNX_WRITE(V3)
         * {
         *   String unit = param[1].isValid() ? param[1].asString() : "";
         * }
         * @endcode
         */
        const char* asString() const    { return ptr; }

        /**
         * @brief Reads this value as a whole number.
         * @return The number, or 0 when the value does not exist or is not a
         *         number.
         *
         * @code
         * PLYNX_WRITE(V2)
         * {
         *   int green = param[1].asInt();   // RGB Picker, second value
         * }
         * @endcode
         */
        int         asInt() const       { if(!isValid()) return 0; return atoi(ptr); }

        /**
         * @brief Reads this value as a `long` whole number.
         * @return The number, or 0 when the value does not exist.
         *
         * @code
         * PLYNX_WRITE(V2)
         * {
         *   long seconds = param[0].asLong();
         * }
         * @endcode
         */
        long        asLong() const      { if(!isValid()) return 0; return atol(ptr); }
#if !defined(PLYNX_NO_LONGLONG) && defined(PLYNX_USE_INTERNAL_ATOLL)
        long long   asLongLong() const  { return atoll_internal(ptr); }
#elif !defined(PLYNX_NO_LONGLONG)
        /**
         * @brief Reads this value as a `long long` whole number.
         * @return The number.
         *
         * @code
         * PLYNX_WRITE(V2)
         * {
         *   long long big = param[0].asLongLong();
         * }
         * @endcode
         */
        long long   asLongLong() const  { return atoll(ptr); }
#endif
#if !defined(PLYNX_NO_FLOAT)
        /**
         * @brief Reads this value as a number with decimals, in a `double`.
         * @return The number, or 0 when the value does not exist.
         *
         * @code
         * PLYNX_WRITE(V8)
         * {
         *   double latitude = param[0].asDouble();
         * }
         * @endcode
         */
        double      asDouble() const    { if(!isValid()) return 0; return atof(ptr); }

        /**
         * @brief Reads this value as a number with decimals, in a `float`.
         * @return The number, or 0 when the value does not exist.
         *
         * @code
         * PLYNX_WRITE(V8)
         * {
         *   float speed = param[1].asFloat();
         * }
         * @endcode
         */
        float       asFloat() const     { if(!isValid()) return 0; return atof(ptr); }
#endif
        /**
         * @brief Tells whether this value exists.
         * @return true when the value is inside the message.
         *
         * @code
         * PLYNX_WRITE(V3)
         * {
         *   if (param[2].isValid()) {
         *     analogWrite(27, param[2].asInt());
         *   }
         * }
         * @endcode
         */
        bool isValid() const            { return ptr != NULL && ptr < limit; }

        /**
         * @brief Tells whether this value is missing or an empty text.
         * @return true when there is nothing to read.
         *
         * @code
         * PLYNX_WRITE(V3)
         * {
         *   if (!param[0].isEmpty()) {
         *     Serial.println(param[0].asStr());
         *   }
         * }
         * @endcode
         */
        bool isEmpty() const            { if(!isValid()) return true; return *ptr == '\0'; }

        bool operator <  (const iterator& it) const { return ptr < it.ptr; }
        bool operator >= (const iterator& it) const { return ptr >= it.ptr; }

        /**
         * @brief Moves to the next value.
         * @return This iterator, now on the next value.
         *
         * @code
         * PLYNX_WRITE(V3)
         * {
         *   PlynxParam::iterator it = param.begin();
         *   ++it;                          // skip the first value
         *   Serial.println(it.asInt());
         * }
         * @endcode
         */
        iterator& operator ++() {
            if(isValid()) {
                ptr += strlen(ptr) + 1;
            }
            return *this;
        }
    private:
        const char* ptr;
        const char* limit;
    };

public:
    /**
     * @brief Reads values that are already in memory, such as a text with
     * zero bytes between the values.
     *
     * @param addr    The first byte of the values.
     * @param length  How many bytes they take.
     *
     * @code
     * void readFixedList()
     * {
     *   const char text[] = "12\0" "34";
     *   PlynxParam list(text, sizeof(text));
     *   int second = list[1].asInt();   // 34
     * }
     * @endcode
     */
    explicit
    PlynxParam(const void* addr, size_t length)
        : buff((char*)addr), len(length), buff_size(length)
    {}

    /**
     * @brief Prepares an empty list in a buffer of the sketch, to fill with
     * add() and send with Plynx.virtualWrite().
     *
     * @param addr      The buffer that will hold the values.
     * @param length    How many bytes are already used, 0 for an empty list.
     * @param buffsize  The size of the buffer in bytes.
     *
     * @code
     * void sendPair()
     * {
     *   char buffer[32];
     *   PlynxParam pair(buffer, 0, sizeof(buffer));
     *   pair.add("on");
     *   pair.add(75);
     *   Plynx.virtualWrite(V4, pair);
     * }
     * @endcode
     *
     * @see add, PlynxApi::virtualWrite
     */
    explicit
    PlynxParam(void* addr, size_t length, size_t buffsize)
        : buff((char*)addr), len(length), buff_size(buffsize)
    {}

    /**
     * @brief Reads the value as text.
     *
     * With several values it reads the first one.
     *
     * @return The text that arrived, such as "1" or "hello".
     *
     * @par App widget
     * A Text Input sends its text, a Menu the number of the chosen item.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   String name = param.asStr();   // a copy that outlives the handler
     *   Serial.println(name);
     * }
     * @endcode
     *
     * @warning The pointer is valid only inside the handler.
     *
     * @see asInt, asFloat, asString
     */
    const char* asStr() const       { return buff; }

    /**
     * @brief Another name for asStr().
     * @return The text that arrived.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   Serial.println(param.asString());
     * }
     * @endcode
     *
     * @see asStr
     */
    const char* asString() const    { return buff; }

    /**
     * @brief Reads the value as a whole number.
     *
     * With several values it reads the first one. A Button sends 1 and 0, a
     * Slider sends a number between its min and max.
     *
     * @return The number, or 0 when the text is not a number. "2.7" gives 2.
     *
     * @par App widget
     * Button, Slider, Step, Menu, Number Input.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   int brightness = param.asInt();   // a Slider from 0 to 255
     *   analogWrite(5, brightness);
     * }
     * @endcode
     *
     * @see asLong, asFloat, asStr
     */
    int         asInt() const       { return atoi(buff); }

    /**
     * @brief Reads the value as a `long` whole number.
     *
     * The same as asInt() for numbers larger than 32767 on AVR boards, where
     * an `int` has 16 bits.
     *
     * @return The number, or 0 when the text is not a number.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   long seconds = param.asLong();
     * }
     * @endcode
     *
     * @see asInt, asLongLong
     */
    long        asLong() const      { return atol(buff); }
#if !defined(PLYNX_NO_LONGLONG) && defined(PLYNX_USE_INTERNAL_ATOLL)
    long long   asLongLong() const  { return atoll_internal(buff); }
#elif !defined(PLYNX_NO_LONGLONG)
    /**
     * @brief Reads the value as a `long long` whole number, 64 bits.
     * @return The number, or 0 when the text is not a number.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   long long millisSince1970 = param.asLongLong();
     * }
     * @endcode
     *
     * @see asLong
     */
    long long   asLongLong() const  { return atoll(buff); }
#endif
#if !defined(PLYNX_NO_FLOAT)
    /**
     * @brief Reads the value as a number with decimals, in a `double`.
     * @return The number, or 0 when the text is not a number.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   double setpoint = param.asDouble();
     * }
     * @endcode
     *
     * @see asFloat, asInt
     */
    double      asDouble() const    { return atof(buff); }

    /**
     * @brief Reads the value as a number with decimals, in a `float`.
     *
     * A Slider with decimals sends values such as "21.5".
     *
     * @return The number, or 0 when the text is not a number.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   float target = param.asFloat();
     *   Serial.println(target, 1);
     * }
     * @endcode
     *
     * @see asDouble, asInt
     */
    float       asFloat() const     { return atof(buff); }
#endif
    /**
     * @brief Tells whether the value is an empty text.
     * @return true when the first value is empty.
     *
     * @code
     * PLYNX_WRITE(V1)
     * {
     *   if (param.isEmpty()) {
     *     return;                     // a Text Input sent nothing
     *   }
     *   Serial.println(param.asStr());
     * }
     * @endcode
     */
    bool isEmpty() const            { return *buff == '\0'; }

    /**
     * @brief Gives the first value, to start a loop over all of them.
     * @return An iterator on the first value.
     *
     * @code
     * PLYNX_WRITE(V3)
     * {
     *   int count = 0;
     *   for (PlynxParam::iterator it = param.begin(); it < param.end(); ++it) {
     *     count++;
     *   }
     *   Serial.println(count);          // how many values arrived
     * }
     * @endcode
     *
     * @see end, iterator
     */
    iterator begin() const { return iterator(buff, buff+len); }

    /**
     * @brief Gives the position after the last value, where a loop stops.
     * @return An iterator past the last value.
     *
     * @code
     * PLYNX_WRITE(V3)
     * {
     *   for (PlynxParam::iterator it = param.begin(); it < param.end(); ++it) {
     *     Serial.println(it.asFloat());
     *   }
     * }
     * @endcode
     *
     * @see begin
     */
    iterator end() const   { return iterator(buff+len, buff+len); }

    /**
     * @brief Gives one value by its position, starting from 0.
     *
     * @param index  0 for the first value, 1 for the second and so on.
     * @return The value. When it does not exist, isValid() is false, asInt()
     *         gives 0 and asStr() gives NULL.
     *
     * @par App widget
     * RGB Picker (red, green, blue), Joystick (x, y), Map and Table.
     *
     * @code
     * PLYNX_WRITE(V2)
     * {
     *   int red   = param[0].asInt();
     *   int green = param[1].asInt();
     *   int blue  = param[2].asInt();
     *   Serial.println(red + green + blue);
     * }
     * @endcode
     *
     * @see iterator, begin
     */
    iterator operator[](int index) const;

    /**
     * @brief Finds the value that follows a given word, in a list made of
     * names and values.
     *
     * @param key  The name to look for.
     * @return The value after that name. When the name is missing, isValid()
     *         is false.
     *
     * @code
     * PLYNX_WRITE(V9)
     * {
     *   // a list such as mode, auto, speed, 3
     *   int speed = param["speed"].asInt();
     *   Serial.println(speed);
     * }
     * @endcode
     */
    iterator operator[](const char* key) const;

    /**
     * @brief Gives the bytes of the values, with a zero byte after each one.
     * @return The first byte.
     *
     * @code
     * PLYNX_WRITE(V3)
     * {
     *   Serial.write((const uint8_t*)param.getBuffer(), param.getLength());
     * }
     * @endcode
     *
     * @see getLength
     */
    void*  getBuffer() const { return (void*)buff; }

    /**
     * @brief Gives how many bytes the values take.
     * @return The number of bytes used, with the zero bytes between values.
     *
     * @code
     * PLYNX_WRITE(V3)
     * {
     *   Serial.println(param.getLength());
     * }
     * @endcode
     *
     * @see getBuffer, getBuffSize
     */
    size_t getLength() const { return len; }

    /**
     * @brief Gives the size of the buffer behind this list.
     * @return The number of bytes the list can hold.
     *
     * @code
     * void showRoom()
     * {
     *   char buffer[32];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add("abc");
     *   Serial.println(list.getBuffSize() - list.getLength());   // bytes left
     * }
     * @endcode
     *
     * @see getLength
     */
    size_t getBuffSize() const { return buff_size; }

    // Modification

    /**
     * @name Adding numbers
     * @{
     */

    /**
     * @brief Adds a number at the end of the list, as text.
     *
     * Every whole number type and `float` and `double` are accepted. A
     * `float` keeps 3 decimals and a `double` keeps 7.
     *
     * @param value  The number to add.
     *
     * @code
     * void sendThree()
     * {
     *   char buffer[32];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add(1);
     *   list.add(2.5f);
     *   list.add(70000L);
     *   Plynx.virtualWrite(V4, list);
     * }
     * @endcode
     *
     * @warning Size the buffer for everything the list will hold, each value
     * plus 1 byte. On AVR boards such as the Uno and the Mega a number that
     * does not fit is left out. On the other boards, ESP32 and ESP8266 among
     * them, a number that does not fit is cut, and the next number added
     * writes past the end of the buffer and corrupts the memory of the board.
     *
     * @see PlynxApi::virtualWrite
     */
    void add(int value);
    void add(unsigned int value);
    void add(long value);
    void add(unsigned long value);
    void add(long long value);
    void add(unsigned long long value);
#ifndef PLYNX_NO_FLOAT
    void add(float value);
    void add(double value);
#endif
    /** @} */

    /**
     * @brief Adds a text at the end of the list.
     *
     * @param str  The text to add. NULL adds an empty value.
     *
     * @code
     * void sendStatus()
     * {
     *   char buffer[32];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add("door");
     *   list.add("open");
     *   Plynx.virtualWrite(V4, list);
     * }
     * @endcode
     *
     * @warning A text that does not fit in the buffer is left out.
     */
    void add(const char* str);

    /**
     * @brief Adds raw bytes at the end of the list, as they are.
     *
     * @param b  The first byte to add.
     * @param l  How many bytes.
     *
     * @code
     * void sendRaw()
     * {
     *   char buffer[16];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add("ab", 3);   // "ab" and its zero byte
     * }
     * @endcode
     *
     * @warning Bytes that do not fit in the buffer are left out.
     */
    void add(const void* b, size_t l);
#if defined(ARDUINO) || defined(SPARK) || defined(PARTICLE)
    /**
     * @brief Adds the text of a `String` at the end of the list.
     *
     * @param str  The text to add.
     *
     * @code
     * void sendName()
     * {
     *   char buffer[32];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   String room = "kitchen";
     *   list.add(room);
     * }
     * @endcode
     */
    void add(const String& str);
#if defined(PLYNX_HAS_PROGMEM)
    /**
     * @brief Adds a text kept in flash with F("...") at the end of the list.
     * Only on AVR boards.
     *
     * @param str  The text to add.
     *
     * @code
     * void sendLabel()
     * {
     *   char buffer[32];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add(F("garage"));
     * }
     * @endcode
     */
    void add(const __FlashStringHelper* str);
#endif
#endif

    /**
     * @brief Adds one value of any type, used by add_multi().
     *
     * @param last  The value to add.
     *
     * @code
     * void addOne()
     * {
     *   char buffer[16];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add_multi(42);
     * }
     * @endcode
     */
    template<typename T, typename... Args>
    void add_multi(T last) {
        add(last);
    }

    /**
     * @brief Adds several values of any type, in order.
     *
     * @param head  The first value.
     * @param tail  The other values.
     *
     * @code
     * void sendAll()
     * {
     *   char buffer[48];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add_multi("kitchen", 21.5, 48);
     *   Plynx.virtualWrite(V4, list);
     * }
     * @endcode
     */
    template<typename T, typename... Args>
    void add_multi(T head, Args... tail) {
        add(head);
        add_multi(tail...);
    }

    /**
     * @brief Adds a name and its value, for lists read with
     * `param["name"]`.
     *
     * @param key  The name.
     * @param val  The value.
     *
     * @code
     * void sendSettings()
     * {
     *   char buffer[48];
     *   PlynxParam list(buffer, 0, sizeof(buffer));
     *   list.add_key("mode", "auto");
     *   list.add_key("speed", 3);
     *   Plynx.virtualWrite(V9, list);
     * }
     * @endcode
     *
     * @see operator[]
     */
    template <typename TV>
    void add_key(const char* key, const TV& val) {
        add(key);
        add(val);
    }

protected:
    char*    buff;
    size_t   len;
    size_t   buff_size;
};


/**
 * @brief A PlynxParam that takes its buffer from the heap and gives it back
 * when it goes out of scope.
 *
 * @code
 * void sendMany()
 * {
 *   PlynxParamAllocated list(256);
 *   for (int i = 0; i < 10; i++) {
 *     list.add(i * i);
 *   }
 *   Plynx.virtualWrite(V10, list);
 * }
 * @endcode
 *
 * @warning On small boards the heap is small. The allocation is not checked,
 * so a size the heap cannot give makes the board misbehave or crash when
 * values are added.
 *
 * @see PlynxParam
 */
class PlynxParamAllocated
    : public PlynxParam
{
public:
    /**
     * @brief Takes `size` bytes from the heap for the list.
     * @param size  The size of the buffer in bytes.
     *
     * @code
     * void prepare()
     * {
     *   PlynxParamAllocated list(64);
     *   list.add("ready");
     * }
     * @endcode
     */
    PlynxParamAllocated(size_t size)
        : PlynxParam(malloc(size), 0, size)
    {}
    ~PlynxParamAllocated() {
        free(buff);
    }
};

inline
PlynxParam::iterator PlynxParam::operator[](int index) const
{
    const iterator e = end();
    for (iterator it = begin(); it < e; ++it) {
        if (!index--) {
            return it;
        }
    }
    return iterator::invalid();
}

inline
PlynxParam::iterator PlynxParam::operator[](const char* key) const
{
    const iterator e = end();
    for (iterator it = begin(); it < e; ++it) {
        if (!strcmp(it.asStr(), key)) {
            return ++it;
        }
        ++it;
        if (it >= e) break;
    }
    return iterator::invalid();
}

inline
void PlynxParam::add(const void* b, size_t l)
{
    if (len + l > buff_size)
        return;
    memcpy(buff+len, b, l);
    len += l;
}

inline
void PlynxParam::add(const char* str)
{
    if (str == NULL) {
        buff[len++] = '\0';
        return;
    }
    add(str, strlen(str)+1);
}

#if defined(ARDUINO) || defined(SPARK) || defined(PARTICLE)
inline
void PlynxParam::add(const String& str)
{
#if defined(ARDUINO_AVR_DIGISPARK) \
    || defined(__ARDUINO_X86__) \
    || defined(__RFduino__)

    size_t len = str.length()+1;
    char buff[len];
    const_cast<String&>(str).toCharArray(buff, len);
    add(buff, len);
#else
    add(str.c_str());
#endif
}

#if defined(PLYNX_HAS_PROGMEM)

inline
void PlynxParam::add(const __FlashStringHelper* ifsh)
{
    PGM_P p = reinterpret_cast<PGM_P>(ifsh);
    size_t l = strlen_P(p) + 1;
    if (len + l > buff_size)
        return;
    memcpy_P(buff+len, p, l);
    len += l;
    buff[len] = '\0';
}

#endif

#endif

#if defined(__AVR__) || defined (ARDUINO_ARCH_ARC32)

    #include <stdlib.h>

    inline
    void PlynxParam::add(int value)
    {
        char str[2 + 8 * sizeof(value)];
        itoa(value, str, 10);
        add(str);
    }

    inline
    void PlynxParam::add(unsigned int value)
    {
        char str[1 + 8 * sizeof(value)];
        utoa(value, str, 10);
        add(str);
    }

    inline
    void PlynxParam::add(long value)
    {
        char str[2 + 8 * sizeof(value)];
        ltoa(value, str, 10);
        add(str);
    }

    inline
    void PlynxParam::add(unsigned long value)
    {
        char str[1 + 8 * sizeof(value)];
        ultoa(value, str, 10);
        add(str);
    }

    inline
    void PlynxParam::add(long long value)  // TODO: this currently adds just a long
    {
        char str[2 + 8 * sizeof(value)];
        ltoa(value, str, 10);
        add(str);
    }

    inline
    void PlynxParam::add(unsigned long long value) // TODO: this currently adds just a long
    {
        char str[1 + 8 * sizeof(value)];
        ultoa(value, str, 10);
        add(str);
    }

#ifndef PLYNX_NO_FLOAT

    inline
    void PlynxParam::add(float value)
    {
        char str[33];
        dtostrf(value, 5, 3, str);
        add(str);
    }

    inline
    void PlynxParam::add(double value)
    {
        char str[33];
        dtostrf(value, 5, 7, str);
        add(str);
    }
#endif

#else

    #include <stdio.h>

    inline
    void PlynxParam::add(int value)
    {
        len += snprintf(buff+len, buff_size-len, "%i", value)+1;
    }

    inline
    void PlynxParam::add(unsigned int value)
    {
        len += snprintf(buff+len, buff_size-len, "%u", value)+1;
    }

    inline
    void PlynxParam::add(long value)
    {
        len += snprintf(buff+len, buff_size-len, "%li", value)+1;
    }

    inline
    void PlynxParam::add(unsigned long value)
    {
        len += snprintf(buff+len, buff_size-len, "%lu", value)+1;
    }

    inline
    void PlynxParam::add(long long value)
    {
        len += snprintf(buff+len, buff_size-len, "%lli", value)+1;
    }

    inline
    void PlynxParam::add(unsigned long long value)
    {
        len += snprintf(buff+len, buff_size-len, "%llu", value)+1;
    }

#ifndef PLYNX_NO_FLOAT

#if defined(PLYNX_USE_INTERNAL_DTOSTRF)

    inline
    void PlynxParam::add(float value)
    {
        char str[33];
        dtostrf_internal(value, 5, 3, str);
        add(str);
    }

    inline
    void PlynxParam::add(double value)
    {
        char str[33];
        dtostrf_internal(value, 5, 7, str);
        add(str);
    }

#else

    inline
    void PlynxParam::add(float value)
    {
        len += snprintf(buff+len, buff_size-len, "%2.3f", value)+1;
    }

    inline
    void PlynxParam::add(double value)
    {
        len += snprintf(buff+len, buff_size-len, "%2.7f", value)+1;
    }

#endif

#endif

#endif


#endif
