/**
 * @file       main.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Mar 2015
 * @brief
 */

//#define PLYNX_DEBUG
#define PLYNX_PRINT stdout
#ifdef RASPBERRY
  #include <PlynxApiWiringPi.h>
#else
  #include <PlynxApiLinux.h>
#endif
#include <PlynxSocket.h>
#include <PlynxOptionsParser.h>

static PlynxTransportSocket _plynxTransport;
PlynxSocket Plynx(_plynxTransport);

static const char *auth, *serv;
static uint16_t port;

#include <PlynxWidgets.h>

PlynxTimer tmr;

PLYNX_WRITE(V1)
{
    printf("Got a value: %s\n", param[0].asStr());
}

void setup()
{
    Plynx.begin(auth, serv, port);
    tmr.setInterval(1000, [](){
      Plynx.virtualWrite(V0, PlynxMillis()/1000);
    });
}

void loop()
{
    Plynx.run();
    tmr.run();
}


int main(int argc, char* argv[])
{
    parse_options(argc, argv, auth, serv, port);

    setup();
    while(true) {
        loop();
    }

    return 0;
}

