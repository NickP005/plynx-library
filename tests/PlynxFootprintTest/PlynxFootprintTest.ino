/**************************************************************
 * This is just a test of Plynx library memory footprint.
 * It doesn't use serial, wifi, ethernet or any other library.
 * It uses a volatile variable to simulate server stream.
 *
 *         You should NOT flash this program
 *         to your hardware or try to run it.
 *
 **************************************************************/

//#define ENABLE_MINIMIZATION
#define ENABLE_HANDLERS

#ifdef ENABLE_MINIMIZATION
  #define PLYNX_NO_BUILTIN
  #define PLYNX_NO_INFO
  #define PLYNX_NO_FLOAT
#endif

#include <PlynxSimpleUserDefined.h>

char auth[] = "12345678901234567890123456789012";

volatile uint8_t test;

// This function is used by Plynx to receive data
size_t PlynxStreamRead(void* buf, size_t len)
{
  uint8_t* byte_buff = (uint8_t*)buf;
  size_t res = len;
  while (len--) {
    *byte_buff++ = test;
  }
  return res;
}

// This function is used by Plynx to send data
size_t PlynxStreamWrite(const void* buf, size_t len)
{
  uint8_t* byte_buff = (uint8_t*)buf;
  size_t res = len;
  while (len--) {
    test = *byte_buff++;
  }
  return res;
}

void setup()
{
  Plynx.begin(auth);
  Plynx.connect();
}

#ifdef ENABLE_HANDLERS

PLYNX_CONNECTED()
{
  test = 1;
}

PLYNX_DISCONNECTED()
{
  test = 2;
}

PLYNX_WRITE(V3)
{
  test = param.asInt();
}

PLYNX_READ(V4)
{
  Plynx.virtualWrite(V4, millis(), PlynxFreeRam());
}

#endif /* ENABLE_HANDLERS */

void loop()
{
  bool hasIncomingData = (test > 0);

  if (!Plynx.run(hasIncomingData)) {

  }
}

