/**
 * @file       PlynxProtocol.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Plynx protocol implementation
 *
 */

#ifndef PlynxProtocol_h
#define PlynxProtocol_h

#include <string.h>
#include <stdlib.h>
#include <Plynx/PlynxDebug.h>
#include <Plynx/PlynxProtocolDefs.h>
#include <Plynx/PlynxApi.h>
#include <utility/PlynxUtility.h>

// Built-in OTA hooks (Plynx/PlynxOta.h, ESP32/ESP8266 only; the ESP shells
// pull the implementation in at the bottom, after the global Plynx object
// exists). Other platforms: the call sites below vanish, zero cost.
//
// Read this before touching the condition. This file has an include guard, so
// whatever is decided here is decided at the FIRST inclusion in a translation
// unit. PLYNX_OTA_HAS_DEFAULT is defined by a HEADER, so it can be on in one
// translation unit and off in another -- and then PlynxProtocol<Transp> gets
// two different bodies of the same COMDAT symbol. That is an ODR violation
// neither the compiler nor the linker reports: the linker keeps one body,
// picked by link order, so a two-file sketch lost its OTA dispatch depending
// on the alphabetical order of the file names. Measured, not feared.
//
// So the condition here uses only macros that come from the build flags and
// are therefore identical in every translation unit, and the call sites are
// compiled unconditionally. Whether the OTA code is really there is settled at
// LINK time instead: without PlynxOta.h the weak no-ops below are what the
// calls resolve to.
#if defined(ESP32) || defined(ESP8266)
#define PLYNX_OTA_CALLS 1
void PlynxOtaTick();                                      // pending-confirm watchdog
void PlynxOtaAuthOk();                                    // authenticated handshake -> confirm
void PlynxOtaDispatch(PlynxReq& req, const PlynxParam& param); // default OTA handler (user override wins)
// 🔴 A transport calls this when its receive queue could NOT take everything
// that arrived. Nobody downstream can notice that loss: the queue hands the
// parser a byte stream, so a frame cut in half by a full queue reappears as a
// plausible frame of the wrong length, and during an OTA the offset check
// counts bytes, not frames. This is the only place in the program where the
// loss is still a known fact, so it has to be reported from here.
void PlynxOtaRxLost(size_t lost);

#ifdef PLYNX_OTA_HAS_DEFAULT
// PlynxOta.h will be compiled in this translation unit and provides the strong
// definitions. Also a marker for PlynxOta.h itself: see the #error there.
#define PLYNX_OTA_PROTOCOL_WIRED 1
#else
// No built-in OTA here (PLYNX_NO_OTA, or a shell that does not carry the
// handler, or this translation unit simply included us first). Weak no-ops so
// the identical call sites still link; a translation unit that does compile
// PlynxOta.h overrides them, for the whole program.
__attribute__((weak)) void PlynxOtaTick() {}
__attribute__((weak)) void PlynxOtaAuthOk() {}
__attribute__((weak)) void PlynxOtaRxLost(size_t lost) { (void)lost; }
__attribute__((weak)) void PlynxOtaDispatch(PlynxReq& req, const PlynxParam& param)
{
    // Exactly what the switch used to do without the built-in handler: hand the
    // frame to the sketch's PLYNX_WRITE(InternalPinOTA), if it defined one.
    PlynxWidgetWriteInternalPinOTA(req, param);
}
#endif
#endif // ESP32 || ESP8266

/**
 * @brief Connects the board to the server and keeps it online. You call these
 * functions as `Plynx.run()`, `Plynx.connected()` and so on.
 *
 * The functions that send values are in PlynxApi. `begin()` and `config()`
 * are in the header of your board.
 *
 * @see PlynxApi, PlynxWifi
 */
template <class Transp>
class PlynxProtocol
    : public PlynxApi< PlynxProtocol<Transp> >
{
    friend class PlynxApi< PlynxProtocol<Transp> >;
public:
    enum PlynxState {
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
        TOKEN_INVALID,
    };

    PlynxProtocol(Transp& transp)
        : conn(transp)
        , authkey(NULL)
        , redir_serv(NULL)
        , lastActivityIn(0)
        , lastActivityOut(0)
        , lastHeartbeat(0)
        , msgIdOut(0)
        , msgIdOutOverride(0)
        , nesting(0)
        , state(CONNECTING)
    {}

    /**
     * @brief Tells whether the board is connected to the server right now.
     *
     * It is false as soon as the board notices the connection dropped. The WiFi
     * can be up while the server is unreachable, and then it is false too.
     *
     * @return true when the board is connected and logged in to the server.
     *
     * For example, an LED that shows the link, or skipping work that needs the
     * connection.
     *
     * @code
     * PlynxTimer timer;
     * const int linkLed = 2;   // the pin of your LED
     *
     * void showLink()
     * {
     *   digitalWrite(linkLed, Plynx.connected() ? HIGH : LOW);
     * }
     *
     * void setup()
     * {
     *   pinMode(linkLed, OUTPUT);
     *   // Plynx.begin(...) as in the example for your board
     *   timer.setInterval(1000L, showLink);
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     *   timer.run();
     * }
     * @endcode
     *
     * @see PLYNX_CONNECTED, PLYNX_DISCONNECTED, isTokenInvalid
     */
    bool connected() const { return state == CONNECTED; }

    /**
     * @brief Tells whether the server refused the token of this board.
     *
     * It turns true when the server answers that the auth token does not
     * exist. The library then stops trying, until connect() is called again.
     * With PLYNX_PRINT defined the serial monitor shows "Invalid auth token".
     *
     * @return true after the server refused the token.
     *
     * Call it after connect() returns false, to tell a wrong token apart from a
     * network that does not work.
     *
     * @code
     * void tryOnce()
     * {
     *   if (!Plynx.connect()) {
     *     if (Plynx.isTokenInvalid()) {
     *       Serial.println("Check the auth token in the Plynx app");
     *     }
     *   }
     * }
     * @endcode
     *
     * @warning A token copied with a missing letter or an extra space is the
     * usual cause. The token is the one shown for this board in the Plynx app.
     *
     * @see connect, connected
     */
    bool isTokenInvalid() const { return state == TOKEN_INVALID; }

#ifdef PLYNX_OTA_CALLS
    // Read-only view of the transport for the built-in OTA (PlynxOta.h): the
    // expected firmware download host is the configured server endpoint. Not
    // under PLYNX_OTA_HAS_DEFAULT: whether this member exists must not depend
    // on include order either -- its absence used to be the only thing that
    // caught a misbuilt sketch, by accident, and it stopped catching it as
    // soon as the user defined PLYNX_OTA_HOST.
    const Transp& transportConn() const { return conn; }
#endif

    /**
     * @brief Connects to the server and waits until it is done or the time is
     * over.
     *
     * It closes any open connection, then tries again and again until the
     * server accepts the token or `timeout` milliseconds have passed. During
     * the wait it runs Plynx.run(), so PLYNX_CONNECTED() runs as soon as the
     * board is in.
     *
     * @param timeout  The longest wait in milliseconds. The default is three
     *                 times PLYNX_TIMEOUT_MS, 18 seconds.
     * @return true when the board is connected.
     *
     * Call it in setup() after config(), when the sketch has to keep working without a
     * connection. Plynx.begin() calls it for you and waits without a limit.
     *
     * @code
     * #include <PlynxSimpleEsp32.h>
     *
     * char auth[] = "YourAuthToken";
     *
     * void setup()
     * {
     *   WiFi.begin("YourWiFiName", "YourWiFiPassword");
     *   Plynx.config(auth);
     *   if (!Plynx.connect(10000)) {
     *     // offline for now, Plynx.run() keeps trying in loop()
     *   }
     * }
     *
     * void loop()
     * {
     *   Plynx.run();
     * }
     * @endcode
     *
     * @see config, run, disconnect, isTokenInvalid
     */
    bool connect(uint32_t timeout = PLYNX_TIMEOUT_MS*3) {
        conn.disconnect();
        state = CONNECTING;
        millis_time_t started = PlynxMillis();
        while ((state != CONNECTED) &&
               (PlynxMillis() - started < timeout))
        {
            run();
        }
        return state == CONNECTED;
    }

    /**
     * @brief Closes the connection to the server and keeps it closed.
     *
     * After this call Plynx.run() does nothing and the board does not
     * reconnect by itself, until connect() is called. PLYNX_DISCONNECTED()
     * does not run for a disconnect asked by the sketch.
     *
     * Call it before a long task that must not be disturbed, before deep sleep, or to
     * save power on a battery board.
     *
     * @code
     * void goOffline()
     * {
     *   Plynx.disconnect();
     * }
     *
     * void goOnline()
     * {
     *   Plynx.connect();
     * }
     * @endcode
     *
     * @see connect, connected, PLYNX_DISCONNECTED
     */
    void disconnect() {
        conn.disconnect();
        state = DISCONNECTED;
        PLYNX_LOG1(PLYNX_F("Disconnected"));
    }

    /**
     * @brief Keeps the board online and runs your PLYNX_WRITE() code when the app
     * sends a value. Call it at every turn of loop().
     *
     * Each call reads the messages that arrived, runs the matching
     * PLYNX_WRITE() and PLYNX_READ() handlers, sends the heartbeat that keeps
     * the connection open and, when the connection dropped, tries again every
     * 5 seconds. Handlers run only inside this call.
     *
     * @param avail  true when the sketch already knows that data is waiting.
     *               Leave it out.
     * @return false in the call where the connection drops, an attempt to
     *         connect fails, or the server refuses or does not answer the
     *         login, and in every call after disconnect(). true in all other
     *         calls, even while the board is offline waiting for its next
     *         attempt and after the server refused the token. To know whether
     *         the board is online, call connected().
     *
     * @code
     * PlynxTimer timer;
     *
     * void loop()
     * {
     *   Plynx.run();
     *   timer.run();
     * }
     * @endcode
     *
     * @warning A long delay() in loop() stops Plynx.run(). The app waits for
     * the board, and after about a minute without news from the server
     * (PLYNX_HEARTBEAT plus three times PLYNX_TIMEOUT_MS) the board drops the
     * connection and reconnects. Use a PlynxTimer to do things later.
     *
     * @warning A `while` loop that waits for a button or a sensor blocks
     * Plynx.run() in the same way.
     *
     * @see PlynxTimer, connect, connected, PLYNX_HEARTBEAT
     */
    bool run(bool avail = false);

    // TODO: Fixme
    void startSession() {
        conn.connect();
        state = CONNECTING;
        msgIdOut = 0;
        lastHeartbeat = lastActivityIn = lastActivityOut = (PlynxMillis() - 5000UL);
    }

    void sendCmd(uint8_t cmd, uint16_t id = 0, const void* data = NULL, size_t length = 0, const void* data2 = NULL, size_t length2 = 0);

    void sendResponse(PlynxStatus rsp, uint16_t id = 0) {
        sendCmd(PLYNX_CMD_RESPONSE, id, NULL, rsp);
    }

    void printBanner() {
#if defined(PLYNX_NO_FANCY_LOGO)
        PLYNX_LOG1(PLYNX_F("Plynx v" PLYNX_VERSION " on " PLYNX_INFO_DEVICE));
#else
        PLYNX_LOG1(PLYNX_F(PLYNX_NEWLINE
            "    ___  __          __" PLYNX_NEWLINE
            "   / _ )/ /_ _____  / /__" PLYNX_NEWLINE
            "  / _  / / // / _ \\/  '_/" PLYNX_NEWLINE
            " /____/_/\\_, /_//_/_/\\_\\" PLYNX_NEWLINE
            "        /___/ v" PLYNX_VERSION " on " PLYNX_INFO_DEVICE PLYNX_NEWLINE
        ));
#endif
    }

private:

    void internalReconnect() {
        state = CONNECTING;
        conn.disconnect();
        PlynxOnDisconnected();
    }

    int readHeader(PlynxHeader& hdr);
    uint16_t getNextMsgId();

protected:
    void begin(const char* auth) {
        this->authkey = auth;
        lastHeartbeat = lastActivityIn = lastActivityOut = (PlynxMillis() - 5000UL);
#if !defined(PLYNX_NO_DEFAULT_BANNER)
        printBanner();
#endif
    }

    bool processInput(void);

    Transp& conn;

private:
    const char* authkey;
    char*       redir_serv;
    millis_time_t lastActivityIn;
    millis_time_t lastActivityOut;
    union {
        millis_time_t lastHeartbeat;
        millis_time_t lastLogin;
    };
    uint16_t msgIdOut;
    uint16_t msgIdOutOverride;
    uint8_t  nesting;
protected:
    PlynxState state;
};

template <class Transp>
bool PlynxProtocol<Transp>::run(bool avail)
{
    PLYNX_RUN_YIELD();

#ifdef PLYNX_OTA_CALLS
    PlynxOtaTick();
#endif

    if (state == DISCONNECTED) {
        return false;
    }

    // Detect nesting
    PlynxHelperAutoInc guard(nesting);
    if (msgIdOutOverride || nesting > 2) {
      //PLYNX_LOG1(PLYNX_F("Nested run() skipped"));
      return true;
    }

    if (conn.connected()) {
        while (avail || conn.available() > 0) {
            //PLYNX_LOG2(PLYNX_F("Available: "), conn.available());
            //const unsigned long t = micros();
            if (!processInput()) {
                conn.disconnect();
// TODO: Only when in direct mode?
#ifdef PLYNX_USE_DIRECT_CONNECT
                state = CONNECTING;
#endif
                PlynxOnDisconnected();
                return false;
            }
            avail = false;
            //PLYNX_LOG2(PLYNX_F("Proc time: "), micros() - t);
        }
    }

    const millis_time_t t = PlynxMillis();

    // Update connection status after running commands
    const bool tconn = conn.connected();

    if (state == CONNECTED) {
        if (!tconn) {
            lastHeartbeat = t;
            internalReconnect();
            return false;
        }

        if (t - lastActivityIn > (1000UL * PLYNX_HEARTBEAT + PLYNX_TIMEOUT_MS*3)) {
#ifdef PLYNX_DEBUG
            PLYNX_LOG6(PLYNX_F("Heartbeat timeout: "), t, PLYNX_F(", "), lastActivityIn, PLYNX_F(", "), lastHeartbeat);
#else
            PLYNX_LOG1(PLYNX_F("Heartbeat timeout"));
#endif
            internalReconnect();
            return false;
        } else if ((t - lastActivityIn  > 1000UL * PLYNX_HEARTBEAT ||
                    t - lastActivityOut > 1000UL * PLYNX_HEARTBEAT) &&
                    t - lastHeartbeat   > PLYNX_TIMEOUT_MS)
        {
            // Send ping if we didn't either send or receive something
            // for PLYNX_HEARTBEAT seconds
            sendCmd(PLYNX_CMD_PING);
            lastHeartbeat = t;
        }
    } else if (state == CONNECTING) {
#ifdef PLYNX_USE_DIRECT_CONNECT
        if (!tconn)
            conn.connect();
#else
        if (tconn && (t - lastLogin > PLYNX_TIMEOUT_MS)) {
            PLYNX_LOG1(PLYNX_F("Login timeout"));
            conn.disconnect();
            state = CONNECTING;
            return false;
        } else if (!tconn && (t - lastLogin > 5000UL)) {
            conn.disconnect();
            if (!conn.connect()) {
                lastLogin = t;
                return false;
            }

            msgIdOut = 1;
            sendCmd(PLYNX_CMD_HW_LOGIN, 1, authkey, strlen(authkey));
            lastLogin = lastActivityOut;
            return true;
        }
#endif
    }
    return true;
}

template <class Transp>
PLYNX_FORCE_INLINE
bool PlynxProtocol<Transp>::processInput(void)
{
    PlynxHeader hdr;
    const int ret = readHeader(hdr);

    if (ret == 0) {
        return true; // Considered OK (no data on input)
    }

    if (ret < 0 || hdr.msg_id == 0) {
#ifdef PLYNX_DEBUG
        PLYNX_LOG2(PLYNX_F("Bad hdr len: "), ret);
#endif
        return false;
    }

    if (hdr.type == PLYNX_CMD_RESPONSE) {
        lastActivityIn = PlynxMillis();

#ifndef PLYNX_USE_DIRECT_CONNECT
        if (state == CONNECTING && (1 == hdr.msg_id)) {
            switch (hdr.length) {
            case PLYNX_SUCCESS:
            case PLYNX_ALREADY_REGISTERED:
                PLYNX_LOG3(PLYNX_F("Ready (ping: "), lastActivityIn-lastHeartbeat, PLYNX_F("ms)."));
                lastHeartbeat = lastActivityIn;
                state = CONNECTED;
#ifdef PLYNX_OTA_CALLS
                PlynxOtaAuthOk();
#endif
#ifdef PLYNX_DEBUG
                if (size_t ram = PlynxFreeRam()) {
                    PLYNX_LOG2(PLYNX_F("Free RAM: "), ram);
                }
#endif
                this->sendInfo();
                PLYNX_RUN_YIELD();
                PlynxOnConnected();
                return true;
            case PLYNX_INVALID_TOKEN:
                PLYNX_LOG1(PLYNX_F("Invalid auth token"));
                state = TOKEN_INVALID;
                break;
            default:
                PLYNX_LOG2(PLYNX_F("Connect failed. code: "), hdr.length);
            }
            return false;
        }
        if (PLYNX_NOT_AUTHENTICATED == hdr.length) {
            return false;
        }
#endif
        // TODO: return code may indicate App presence
        return true;
    }

    if (hdr.length > PLYNX_MAX_READBYTES) {
        PLYNX_LOG2(PLYNX_F("Packet too big: "), hdr.length);
        // TODO: Flush
        internalReconnect();
        return true;
    }

    uint8_t inputBuffer[hdr.length+1]; // Add 1 to zero-terminate
    if (hdr.length != conn.read(inputBuffer, hdr.length)) {
#ifdef PLYNX_DEBUG
        PLYNX_LOG1(PLYNX_F("Can't read body"));
#endif
        return false;
    }
    inputBuffer[hdr.length] = '\0';

    PLYNX_DBG_DUMP(">", inputBuffer, hdr.length);

    lastActivityIn = PlynxMillis();

    switch (hdr.type)
    {
    case PLYNX_CMD_LOGIN:
    case PLYNX_CMD_HW_LOGIN: {
#ifdef PLYNX_USE_DIRECT_CONNECT
        if (strncmp(authkey, (char*)inputBuffer, 32)) {
            PLYNX_LOG1(PLYNX_F("Invalid token"));
            sendResponse(PLYNX_INVALID_TOKEN, hdr.msg_id);
            break;
        }
#endif
        if (state == CONNECTING) {
            PLYNX_LOG1(PLYNX_F("Ready"));
            state = CONNECTED;
#ifdef PLYNX_OTA_CALLS
            PlynxOtaAuthOk();
#endif
#ifdef PLYNX_DEBUG
            if (size_t ram = PlynxFreeRam()) {
                PLYNX_LOG2(PLYNX_F("Free RAM: "), ram);
            }
#endif
            this->sendInfo();
            PLYNX_RUN_YIELD();
            PlynxOnConnected();
        }
        sendResponse(PLYNX_SUCCESS, hdr.msg_id);
    } break;
    case PLYNX_CMD_PING: {
        sendResponse(PLYNX_SUCCESS, hdr.msg_id);
    } break;
    case PLYNX_CMD_REDIRECT: {
        if (!redir_serv) {
             redir_serv = (char*)malloc(64);
        }
        PlynxParam param(inputBuffer, hdr.length);
        uint16_t redir_port = PLYNX_DEFAULT_PORT;

        PlynxParam::iterator it = param.begin();
        if (it >= param.end())
            return false;

        strncpy(redir_serv, it.asStr(), 64);
        redir_serv[63] = '\0';

        if (++it < param.end())
            redir_port = it.asLong();
        PLYNX_LOG4(PLYNX_F("Redirecting to "), redir_serv, ':', redir_port);
        conn.disconnect();
        conn.begin(redir_serv, redir_port);
        state = CONNECTING;
        lastHeartbeat = lastActivityIn = lastActivityOut = (PlynxMillis() - 5000UL);
    } break;
    case PLYNX_CMD_HARDWARE:
    case PLYNX_CMD_BRIDGE: {
        msgIdOutOverride = hdr.msg_id;
        this->processCmd(inputBuffer, hdr.length);
        msgIdOutOverride = 0;
    } break;
    case PLYNX_CMD_INTERNAL: {
        PlynxReq req = { 0 };
        PlynxParam param(inputBuffer, hdr.length);
        PlynxParam::iterator it = param.begin();
        if (it >= param.end())
            return true;

        uint32_t cmd32;
        memcpy(&cmd32, it.asStr(), sizeof(cmd32));

        ++it;
        char* start = (char*)(it).asStr();
        unsigned length = hdr.length - (start - (char*)inputBuffer);
        PlynxParam param2(start, length);

        msgIdOutOverride = hdr.msg_id;
        switch (cmd32) {
        case PLYNX_INT_RTC:  PlynxWidgetWriteInternalPinRTC(req, param2);    break;
        case PLYNX_INT_UTC:  PlynxWidgetWriteInternalPinUTC(req, param2);    break;
#ifdef PLYNX_OTA_CALLS
        // Built-in OTA; a sketch-level PLYNX_WRITE(InternalPinOTA) override
        // is detected inside the dispatch and still wins. Without the built-in
        // handler this resolves to the weak no-op at the top of this file,
        // which forwards to that same override.
        case PLYNX_INT_OTA:  PlynxOtaDispatch(req, param2);                  break;
#else
        case PLYNX_INT_OTA:  PlynxWidgetWriteInternalPinOTA(req, param2);    break;
#endif
        case PLYNX_INT_ACON: PlynxWidgetWriteInternalPinACON(req, param2);   break;
        case PLYNX_INT_ADIS: PlynxWidgetWriteInternalPinADIS(req, param2);   break;
        case PLYNX_INT_META: PlynxWidgetWriteInternalPinMETA(req, param2);   break;
        case PLYNX_INT_VFS:  PlynxWidgetWriteInternalPinVFS(req, param2);    break;
        case PLYNX_INT_DBG:  PlynxWidgetWriteInternalPinDBG(req, param2);    break;
#ifdef PLYNX_DEBUG
        default:             PLYNX_LOG2(PLYNX_F("Invalid internal cmd:"), param.asStr());
#endif
        }
        msgIdOutOverride = 0;
    } break;
    case PLYNX_CMD_DEBUG_PRINT: {
        if (hdr.length) {
            PLYNX_LOG2(PLYNX_F("Server: "), (char*)inputBuffer);
        }
    } break;
    default: {
#ifdef PLYNX_DEBUG
        PLYNX_LOG2(PLYNX_F("Invalid header type: "), hdr.type);
#endif
        // TODO: Flush
        internalReconnect();
    } break;
    }

    return true;
}

template <class Transp>
int PlynxProtocol<Transp>::readHeader(PlynxHeader& hdr)
{
    size_t rlen = conn.read(&hdr, sizeof(hdr));
    if (rlen == 0) {
        return 0;
    }

    if (sizeof(hdr) != rlen) {
        return -1;
    }

    PLYNX_DBG_DUMP(">", &hdr, sizeof(PlynxHeader));

    hdr.msg_id = ntohs(hdr.msg_id);
    hdr.length = ntohs(hdr.length);

    return rlen;
}

#ifndef PLYNX_SEND_THROTTLE
/**
 * @brief The pause in milliseconds after each piece of data the board sends.
 *
 * The default is 0. Some Bluetooth and serial modules lose data when it
 * arrives too fast, and the shells for those boards set it for you.
 *
 * @code
 * #define PLYNX_SEND_THROTTLE 20
 * @endcode
 *
 * @see PLYNX_SEND_CHUNK
 */
#define PLYNX_SEND_THROTTLE 0
#endif

#ifndef PLYNX_SEND_CHUNK
/**
 * @brief The largest piece of data, in bytes, the board writes in one go.
 *
 * It applies to the boards that build each message whole before sending it
 * (ESP32, ESP8266 and any board with PLYNX_SEND_ATOMIC). A longer message goes
 * out in several pieces, each followed by the PLYNX_SEND_THROTTLE pause. The
 * default is large enough to send every message whole. The ESP32 BLE shells
 * set 20, the size of one BLE packet.
 *
 * @code
 * #define PLYNX_SEND_CHUNK 20
 * @endcode
 *
 * @see PLYNX_SEND_THROTTLE
 */
#define PLYNX_SEND_CHUNK 1024 // Just a big number
#endif

template <class Transp>
void PlynxProtocol<Transp>::sendCmd(uint8_t cmd, uint16_t id, const void* data, size_t length, const void* data2, size_t length2)
{
    if (!conn.connected() || (cmd != PLYNX_CMD_RESPONSE && cmd != PLYNX_CMD_PING && cmd != PLYNX_CMD_LOGIN && cmd != PLYNX_CMD_HW_LOGIN && state != CONNECTED) ) {
#ifdef PLYNX_DEBUG_ALL
        PLYNX_LOG2(PLYNX_F("Cmd skipped:"), cmd);
#endif
        return;
    }

    if (0 == id) {
        id = getNextMsgId();
    }

#if defined(PLYNX_MSG_LIMIT) && PLYNX_MSG_LIMIT > 0
    if (cmd >= PLYNX_CMD_TWEET && cmd <= PLYNX_CMD_HARDWARE) {
        const millis_time_t allowed_time = PlynxMax(lastActivityOut, lastActivityIn) + 1000/PLYNX_MSG_LIMIT;
        int32_t wait_time = allowed_time - PlynxMillis();
        if (wait_time >= 0) {
#ifdef PLYNX_DEBUG_ALL
            PLYNX_LOG2(PLYNX_F("Waiting:"), wait_time);
#endif
            while (wait_time >= 0) {
                run();
                wait_time = allowed_time - PlynxMillis();
            }
        } else if (nesting == 0) {
            run();
        }
    }
#endif

    const size_t full_length = (sizeof(PlynxHeader)) +
                               (data  ? length  : 0) +
                               (data2 ? length2 : 0);

#if defined(PLYNX_SEND_ATOMIC) || defined(ESP8266) || defined(ESP32) || defined(SPARK) || defined(PARTICLE) || defined(ENERGIA)
    // Those have more RAM and like single write at a time...

    uint8_t buff[full_length];

    PlynxHeader* hdr = (PlynxHeader*)buff;
    hdr->type = cmd;
    hdr->msg_id = htons(id);
    hdr->length = htons(length+length2);

    size_t pos = sizeof(PlynxHeader);
    if (data && length) {
        memcpy(buff + pos, data, length);
        pos += length;
    }
    if (data2 && length2) {
        memcpy(buff + pos, data2, length2);
    }

    size_t wlen = 0;
    while (wlen < full_length) {
        const size_t chunk = PlynxMin(size_t(PLYNX_SEND_CHUNK), full_length - wlen);
        PLYNX_DBG_DUMP("<", buff + wlen, chunk);
        const size_t w = conn.write(buff + wlen, chunk);
        PlynxDelay(PLYNX_SEND_THROTTLE);
        if (w == 0) {
#ifdef PLYNX_DEBUG
            PLYNX_LOG1(PLYNX_F("Cmd error"));
#endif
            conn.disconnect();
            state = CONNECTING;
            PlynxOnDisconnected();
            return;
        }
        wlen += w;
    }

#else

    PlynxHeader hdr;
    hdr.type = cmd;
    hdr.msg_id = htons(id);
    hdr.length = htons(length+length2);

    PLYNX_DBG_DUMP("<", &hdr, sizeof(hdr));
    size_t wlen = conn.write(&hdr, sizeof(hdr));
    PlynxDelay(PLYNX_SEND_THROTTLE);

    if (cmd != PLYNX_CMD_RESPONSE) {
        if (length) {
            PLYNX_DBG_DUMP("<", data, length);
            wlen += conn.write(data, length);
            PlynxDelay(PLYNX_SEND_THROTTLE);
        }
        if (length2) {
            PLYNX_DBG_DUMP("<", data2, length2);
            wlen += conn.write(data2, length2);
            PlynxDelay(PLYNX_SEND_THROTTLE);
        }
    }

#endif

    if (wlen != full_length) {
#ifdef PLYNX_DEBUG
        PLYNX_LOG4(PLYNX_F("Sent "), wlen, '/', full_length);
#endif
        internalReconnect();
        return;
    }

    lastActivityOut = PlynxMillis();

}

template <class Transp>
uint16_t PlynxProtocol<Transp>::getNextMsgId()
{
    if (msgIdOutOverride != 0)
        return msgIdOutOverride;
    if (++msgIdOut == 0)
        msgIdOut = 1;
    return msgIdOut;
}

#endif
