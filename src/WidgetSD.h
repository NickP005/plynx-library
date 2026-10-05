/**
 * @file       WidgetSD.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef WidgetSD_h
#define WidgetSD_h

#if !defined(ARDUINO)
    #error WidgetSD is not available on this platform!
#endif

#include <Plynx/PlynxWidgetBase.h>

/**
 * @brief Lists and sends the files of an SD card when a widget asks for them
 * on a virtual pin.
 *
 * It answers two commands written to its pin. "ls" followed by a folder sends
 * one value per entry (the name, then the size or "/" for a folder). "get"
 * followed by a file sends the file in pieces of 29 bytes. It needs the SD
 * library and `#include <WidgetSD.h>` after the board header.
 *
 * @par App widget
 * The Plynx app (October 2026) has no widget that sends these commands. A
 * sketch or another board can send them through WidgetBridge.
 *
 * @code
 * #include <PlynxSimpleEthernet.h>
 * #include <SD.h>
 * #include <WidgetSD.h>
 *
 * WidgetSD sd;
 * PLYNX_ATTACH_WIDGET(sd, V15);
 *
 * void setup()
 * {
 *   SD.begin(4);   // 4 is the SD slot of the Ethernet shield
 * }
 * @endcode
 *
 * @warning In this version WidgetSD does not compile for ESP32 and ESP8266
 * boards. Their SD library reads into a `uint8_t` buffer and WidgetSD gives it
 * a `char` buffer, so the build stops with "invalid conversion from 'char*'
 * to 'uint8_t*'". It compiles for AVR boards such as the Arduino Mega.
 *
 * @see PLYNX_ATTACH_WIDGET
 */
class WidgetSD
    : public PlynxWidgetBase
{
public:
    /**
     * @brief Creates the SD object. Give the pin here, or later with
     * PLYNX_ATTACH_WIDGET().
     * @param vPin  The virtual pin that carries the commands.
     *
     * @code
     * #include <PlynxSimpleEthernet.h>
     * #include <SD.h>
     * #include <WidgetSD.h>
     *
     * WidgetSD sd(V15);
     * @endcode
     */
    WidgetSD(uint8_t vPin = -1) : PlynxWidgetBase(vPin) {}

    /**
     * @brief Answers an "ls" or a "get" command. PLYNX_ATTACH_WIDGET() calls
     * it.
     *
     * @param request  The request, with the pin to answer on.
     * @param param    The command and its folder or file.
     *
     * @code
     * #include <PlynxSimpleEthernet.h>
     * #include <SD.h>
     * #include <WidgetSD.h>
     *
     * WidgetSD sd(V15);
     *
     * PLYNX_WRITE(V15)
     * {
     *   sd.onWrite(request, param);   // what PLYNX_ATTACH_WIDGET does
     * }
     * @endcode
     */
    void onWrite(PlynxReq& request, const PlynxParam& param);
};

#include <SD.h>
void WidgetSD::onWrite(PlynxReq PLYNX_UNUSED &request, const PlynxParam& param)
{
    const char* cmd = param[0].asStr();
    if (!strcmp(cmd, "ls")) {
        if (File dir = SD.open(param[1].asStr())) {
            while (File entry = dir.openNextFile()) {
                char mem[32];
                PlynxParam result(mem, 0, sizeof(mem));
                result.add(entry.name());
                if (entry.isDirectory()) {
                    result.add("/");
                } else {
                    result.add(entry.size());
                }
                Plynx.virtualWrite(request.pin, result);
                entry.close();
            }
            dir.close();
        }
    }  else if (!strcmp(cmd, "get")) { // dc dc dc dc d[l|e]
        if (File f = SD.open(param[1].asStr())) {
            if (!f.isDirectory()) {
                char mem[32] = "dc";
                const int maxlen = sizeof(mem)-3;
                int len;
                do {
                    len = f.read(mem+3, maxlen);
                    if (len < 0) {
                        mem[1] = 'e'; // Error!
                        len = 0;
                    } else if (len < maxlen) {
                        mem[1] = 'l'; // Last chunk
                    }
                    Plynx.virtualWriteBinary(request.pin, mem, len + 3);
                } while (len == maxlen);
            } else {

            }

            f.close();
        }
    } else {
        //PLYNX_LOG2(PLYNX_F("Invalid SD command: "), cmd);
    }
}

#endif
