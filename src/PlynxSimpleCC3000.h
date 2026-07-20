/**
 * @file       PlynxSimpleCC3000.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 */

#ifndef PlynxSimpleCC3000_h
#define PlynxSimpleCC3000_h

#include <Adapters/PlynxCC3000.h>
#include <Adafruit_CC3000.h>

// Use hardware SPI for the remaining pins
// SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

static PlynxTransportCC3000 _plynxTransport(cc3000);
PlynxCC3000 Plynx(cc3000, _plynxTransport);

#include <PlynxWidgets.h>

#endif
