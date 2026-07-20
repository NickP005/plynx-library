/**
 * @file       PlynxSimpleMKRNB.h
 * @author     Riccardo Rizzo
 * @license    This project is released under the MIT License (MIT)
 * @date       Dec 2018
 * @brief
 *
 */

#ifndef PlynxSimpleMKRNB_h
#define PlynxSimpleMKRNB_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "MKRNB"
#endif

#include <MKRNB.h>
#include <Adapters/PlynxArduinoNB.h>

static PlynxArduinoClient _plynxTransport;
PlynxSIM Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
