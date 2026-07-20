/**************************************************************
 * This is just a test of Plynx library build in Arduino IDE
 *
 *         You should NOT flash this program
 *         to your hardware or try to run it.
 *
 **************************************************************/

#if defined(ARDUINO_AVR_GEMMA) \
  || defined(ARDUINO_attiny) \
  || defined(ARDUINO_AVR_TRINKET3) \
  || defined(ARDUINO_AVR_TRINKET5) \
  || defined(ARDUINO_AVR_DIGISPARK)

  #define PLYNX_NO_INFO
  #define PLYNX_NO_BUILTIN

  #define SKIP_WRITES_TEST

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

PLYNX_WRITE(V3)
{
  test = param.asInt();
}

PLYNX_READ(V4)
{
  Plynx.virtualWrite(V10, millis(), PlynxFreeRam());

  Plynx.virtualWrite(V10, 1,   1U);
  Plynx.virtualWrite(V10, 1L,  1UL);
  Plynx.virtualWrite(V10, 1LL, 1ULL);

#ifndef SKIP_WRITES_TEST

  Plynx.virtualWrite(V10,  (int8_t)1,  (uint8_t)1);
  Plynx.virtualWrite(V10, (int16_t)1, (uint16_t)1);
  Plynx.virtualWrite(V10, (int32_t)1, (uint32_t)1);
  //Plynx.virtualWrite(V10, (int64_t)1, (uint64_t)1);

  Plynx.virtualWrite(V10, (size_t)1);

#ifndef PLYNX_NO_FLOAT
  Plynx.virtualWrite(V10, (float)1.0F);
  Plynx.virtualWrite(V10, (double)1.0);
#endif

  Plynx.virtualWrite(V10, String("Some string as String)"));
  Plynx.virtualWrite(V10, "Some string from RAM");

  Plynx.virtualWrite(V10, PLYNX_F("Some string from Flash"));

  PlynxParamAllocated param(128);
  Plynx.virtualWrite(V10, param);

  Plynx.virtualWriteBinary(V10, "buffer", 6);

#endif

}

void loop()
{
  bool hasIncomingData = (test > 0);

  if (!Plynx.run(hasIncomingData)) {

  }
}

