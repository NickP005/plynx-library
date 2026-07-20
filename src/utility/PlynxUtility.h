/**
 * @file       PlynxUtility.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jun 2015
 * @brief      Utility functions
 *
 */

#ifndef PlynxUtility_h
#define PlynxUtility_h

template<class T>
const T& PlynxMin(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template<class T>
const T& PlynxMax(const T& a, const T& b)
{
    return (b < a) ? a : b;
}


template <class T, class T2>
T PlynxMathMap(T x, T2 in_min, T2 in_max, T2 out_min, T2 out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

template <class T, class T2>
T PlynxMathClamp(T val, T2 low, T2 high)
{
  return (val < low) ? low : ((val > high) ? high : val);
}

template <class T, class T2>
T PlynxMathClampMap(T x, T2 in_min, T2 in_max, T2 out_min, T2 out_max)
{
  x = PlynxMathClamp(x, in_min, in_max);
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


template <unsigned WSIZE, typename T>
void PlynxAverageSample (T& avg, const T& input) {
    avg -= avg/WSIZE;
    const T add = input/WSIZE;
    // Fix for shorter delays
    if (add > 0)
      avg += add;
    else
      avg -= 1;
}

static inline
uint32_t PlynxCRC32(const void* data, size_t length, uint32_t previousCrc32 = 0)
{
  const uint32_t Polynomial = 0xEDB88320;
  uint32_t crc = ~previousCrc32;
  unsigned char* current = (unsigned char*)data;
  while (length--) {
    crc ^= *current++;
    for (unsigned int j = 0; j < 8; j++) {
      crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
    }
  }
  return ~crc;
}

class PlynxHelperAutoInc {
public:
    PlynxHelperAutoInc(uint8_t& counter) : c(counter) { ++c; }
    ~PlynxHelperAutoInc() { --c; }
private:
    uint8_t& c;
};

#define PlynxBitSet(value, bit)   ((value) |= (1UL << (bit)))
#define PlynxBitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define PlynxBitRead(value, bit)  (((value) >> (bit)) & 0x01)
#define PlynxBitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#endif
