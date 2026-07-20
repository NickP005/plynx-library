/**
 * @file       PlynxSimpleWildFire.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 *
 */

#ifndef PlynxSimpleWildFire_h
#define PlynxSimpleWildFire_h

#include <Adapters/PlynxWildFire.h>

WildFire_CC3000 cc3000;
static PlynxTransportWildFire _plynxTransport(cc3000);
PlynxWildFire Plynx(cc3000, _plynxTransport);

#include <PlynxWidgets.h>

#endif
