/**
 * @file       PlynxProtocolDefs.h
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Plynx protocol definitions
 *
 */

#ifndef PlynxProtocolDefs_h
#define PlynxProtocolDefs_h

// Command definitions from:
//   https://github.com/NickP005/plynx-server/blob/master/server/core/src/main/java/cc/plynx/server/core/protocol/enums/Command.java

enum PlynxCmd
{
    PLYNX_CMD_RESPONSE       = 0,

    PLYNX_CMD_LOGIN          = 2,
    PLYNX_CMD_PING           = 6,

    PLYNX_CMD_TWEET          = 12,
    PLYNX_CMD_EMAIL          = 13,
    PLYNX_CMD_NOTIFY         = 14,
    PLYNX_CMD_BRIDGE         = 15,
    PLYNX_CMD_HARDWARE_SYNC  = 16,
    PLYNX_CMD_INTERNAL       = 17,
    PLYNX_CMD_SMS            = 18,
    PLYNX_CMD_PROPERTY       = 19,
    PLYNX_CMD_HARDWARE       = 20,
    PLYNX_CMD_HW_LOGIN       = 29,

    PLYNX_CMD_REDIRECT       = 41,
    PLYNX_CMD_DEBUG_PRINT    = 55,
    PLYNX_CMD_EVENT_LOG      = 64,
    PLYNX_CMD_EVENT_CLEAR    = 65
};

enum PlynxStatus
{
    PLYNX_SUCCESS                = 200,
    PLYNX_QUOTA_LIMIT_EXCEPTION  = 1,
    PLYNX_ILLEGAL_COMMAND        = 2,
    PLYNX_NOT_REGISTERED         = 3,
    PLYNX_ALREADY_REGISTERED     = 4,
    PLYNX_NOT_AUTHENTICATED      = 5,
    PLYNX_NOT_ALLOWED            = 6,
    PLYNX_DEVICE_NOT_IN_NETWORK  = 7,
    PLYNX_NO_ACTIVE_DASHBOARD    = 8,
    PLYNX_INVALID_TOKEN          = 9,
    PLYNX_ILLEGAL_COMMAND_BODY   = 11,
    PLYNX_GET_GRAPH_DATA_EXCEPTION = 12,
    PLYNX_NO_DATA_EXCEPTION      = 17,
    PLYNX_DEVICE_WENT_OFFLINE    = 18,
    PLYNX_SERVER_EXCEPTION       = 19,

    PLYNX_NTF_INVALID_BODY       = 13,
    PLYNX_NTF_NOT_AUTHORIZED     = 14,
    PLYNX_NTF_ECXEPTION          = 15,

    PLYNX_TIMEOUT                = 16,

    PLYNX_NOT_SUPPORTED_VERSION  = 20,
    PLYNX_ENERGY_LIMIT           = 21,

    PLYNX_OPERATION_ACCEPT       = 23,
    PLYNX_OPERATION_DECLINE      = 24,
};

struct PlynxHeader
{
    uint8_t  type;
    uint16_t msg_id;
    uint16_t length;
}
PLYNX_ATTR_PACKED;

#if defined(ESP32)
    #include <lwip/ip_addr.h>
#elif !defined(htons) && (defined(ARDUINO) || defined(ESP8266) || defined(PARTICLE) || defined(__MBED__))
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define htons(x) ( ((x)<<8) | (((x)>>8)&0xFF) )
        #define htonl(x) ( ((x)<<24 & 0xFF000000UL) | \
                           ((x)<< 8 & 0x00FF0000UL) | \
                           ((x)>> 8 & 0x0000FF00UL) | \
                           ((x)>>24 & 0x000000FFUL) )
        #define ntohs(x) htons(x)
        #define ntohl(x) htonl(x)
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define htons(x) (x)
        #define htonl(x) (x)
        #define ntohs(x) (x)
        #define ntohl(x) (x)
    #else
        #error byte order problem
    #endif
#endif

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #define PLYNX_STR_16(a,b)     ((uint16_t(a) << 0) | (uint16_t(b) << 8))
    #define PLYNX_STR_32(a,b,c,d) ((uint32_t(a) << 0) | (uint32_t(b) << 8) | (uint32_t(c) << 16) | (uint32_t(d) << 24))
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #define PLYNX_STR_16(a,b)     ((uint16_t(b) << 0) | (uint16_t(a) << 8))
    #define PLYNX_STR_32(a,b,c,d) ((uint32_t(d) << 0) | (uint32_t(c) << 8) | (uint32_t(b) << 16) | (uint32_t(a) << 24))
#else
    #error byte order problem
#endif

#define PLYNX_HW_PM PLYNX_STR_16('p','m')
#define PLYNX_HW_DW PLYNX_STR_16('d','w')
#define PLYNX_HW_DR PLYNX_STR_16('d','r')
#define PLYNX_HW_AW PLYNX_STR_16('a','w')
#define PLYNX_HW_AR PLYNX_STR_16('a','r')
#define PLYNX_HW_VW PLYNX_STR_16('v','w')
#define PLYNX_HW_VR PLYNX_STR_16('v','r')

#define PLYNX_INT_RTC  PLYNX_STR_32('r','t','c',0)
#define PLYNX_INT_UTC  PLYNX_STR_32('u','t','c',0)
#define PLYNX_INT_OTA  PLYNX_STR_32('o','t','a',0)
#define PLYNX_INT_VFS  PLYNX_STR_32('v','f','s',0)
#define PLYNX_INT_DBG  PLYNX_STR_32('d','b','g',0)
#define PLYNX_INT_ACON PLYNX_STR_32('a','c','o','n')
#define PLYNX_INT_ADIS PLYNX_STR_32('a','d','i','s')
#define PLYNX_INT_META PLYNX_STR_32('m','e','t','a')

#endif
