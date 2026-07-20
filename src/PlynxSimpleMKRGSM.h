/**
 * @file       PlynxSimpleMKRGSM.h
 * @author     Riccardo Rizzo
 * @license    This project is released under the MIT License (MIT)
 * @date       Dec 2018
 * @brief
 *
 */

#ifndef PlynxSimpleMKRGSM_h
#define PlynxSimpleMKRGSM_h

#ifndef PLYNX_INFO_CONNECTION
#define PLYNX_INFO_CONNECTION  "MKRGSM"
#endif

#include <MKRGSM.h>
#include <Adapters/PlynxArduinoGSM.h>

static PlynxArduinoClient _plynxTransport;
PlynxSIM Plynx(_plynxTransport);

#include <PlynxWidgets.h>

#endif
