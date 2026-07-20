/**
 * @file       PlynxSimpleTinyDuino.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 */

#ifndef PlynxSimpleTinyDuino_h
#define PlynxSimpleTinyDuino_h

#ifndef PLYNX_INFO_DEVICE
#define PLYNX_INFO_DEVICE  "TinyDuino"
#endif

#include <Adapters/PlynxCC3000.h>
#include <Adafruit_CC3000.h>

// Pin definitions for the TinyCircuits WiFi TinyShield
#define ADAFRUIT_CC3000_IRQ  2
#define ADAFRUIT_CC3000_VBAT A3
#define ADAFRUIT_CC3000_CS   8

// Use hardware SPI for the remaining pins (on an UNO, SCK = 13, MISO = 12, and MOSI = 11)
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT, SPI_CLOCK_DIVIDER);

static PlynxTransportCC3000 _plynxTransport(cc3000);
PlynxCC3000 Plynx(cc3000, _plynxTransport);

#include <PlynxWidgets.h>

#endif
