/**
 * @file       PlynxHandlers.cpp
 * @author     Volodymyr Shymanskyy
 * @license    This project is released under the MIT License (MIT)
 * @copyright  Copyright (c) 2015 Volodymyr Shymanskyy
 * @date       Jan 2015
 * @brief      Virtual pin utilities
 */

#include <Plynx/PlynxConfig.h>
#include <Plynx/PlynxHandlers.h>
#include <Plynx/PlynxDebug.h>

void PlynxNoOpCbk()
{}

void PlynxWidgetRead(PlynxReq PLYNX_UNUSED &request)
{
    PLYNX_LOG2(PLYNX_F("No handler for reading from pin "), request.pin);
}

void PlynxWidgetWrite(PlynxReq PLYNX_UNUSED &request, const PlynxParam PLYNX_UNUSED &param)
{
    PLYNX_LOG2(PLYNX_F("No handler for writing to pin "), request.pin);
}

#define PLYNX_ON_READ_IMPL(pin)  void PlynxWidgetRead  ## pin (PlynxReq PLYNX_UNUSED &req) \
          __attribute__((weak, alias("PlynxWidgetRead")))

#define PLYNX_ON_WRITE_IMPL(pin) void PlynxWidgetWrite ## pin (PlynxReq PLYNX_UNUSED &req, const PlynxParam PLYNX_UNUSED &param) \
          __attribute__((weak, alias("PlynxWidgetWrite")))

PLYNX_CONNECTED() __attribute__((weak, alias("PlynxNoOpCbk")));
PLYNX_DISCONNECTED() __attribute__((weak, alias("PlynxNoOpCbk")));

// Internal Virtual Pins
PLYNX_ON_WRITE_IMPL(InternalPinACON);
PLYNX_ON_WRITE_IMPL(InternalPinADIS);
PLYNX_ON_WRITE_IMPL(InternalPinRTC);
PLYNX_ON_WRITE_IMPL(InternalPinUTC);
PLYNX_ON_WRITE_IMPL(InternalPinOTA);
PLYNX_ON_WRITE_IMPL(InternalPinMETA);
PLYNX_ON_WRITE_IMPL(InternalPinVFS);
PLYNX_ON_WRITE_IMPL(InternalPinDBG);

// Regular Virtual Pins
PLYNX_ON_READ_IMPL(Default);
PLYNX_ON_WRITE_IMPL(Default);

PLYNX_ON_READ_IMPL(0 );
PLYNX_ON_READ_IMPL(1 );
PLYNX_ON_READ_IMPL(2 );
PLYNX_ON_READ_IMPL(3 );
PLYNX_ON_READ_IMPL(4 );
PLYNX_ON_READ_IMPL(5 );
PLYNX_ON_READ_IMPL(6 );
PLYNX_ON_READ_IMPL(7 );
PLYNX_ON_READ_IMPL(8 );
PLYNX_ON_READ_IMPL(9 );
PLYNX_ON_READ_IMPL(10);
PLYNX_ON_READ_IMPL(11);
PLYNX_ON_READ_IMPL(12);
PLYNX_ON_READ_IMPL(13);
PLYNX_ON_READ_IMPL(14);
PLYNX_ON_READ_IMPL(15);
PLYNX_ON_READ_IMPL(16);
PLYNX_ON_READ_IMPL(17);
PLYNX_ON_READ_IMPL(18);
PLYNX_ON_READ_IMPL(19);
PLYNX_ON_READ_IMPL(20);
PLYNX_ON_READ_IMPL(21);
PLYNX_ON_READ_IMPL(22);
PLYNX_ON_READ_IMPL(23);
PLYNX_ON_READ_IMPL(24);
PLYNX_ON_READ_IMPL(25);
PLYNX_ON_READ_IMPL(26);
PLYNX_ON_READ_IMPL(27);
PLYNX_ON_READ_IMPL(28);
PLYNX_ON_READ_IMPL(29);
PLYNX_ON_READ_IMPL(30);
PLYNX_ON_READ_IMPL(31);
#ifdef PLYNX_USE_128_VPINS
  PLYNX_ON_READ_IMPL(32);
  PLYNX_ON_READ_IMPL(33);
  PLYNX_ON_READ_IMPL(34);
  PLYNX_ON_READ_IMPL(35);
  PLYNX_ON_READ_IMPL(36);
  PLYNX_ON_READ_IMPL(37);
  PLYNX_ON_READ_IMPL(38);
  PLYNX_ON_READ_IMPL(39);
  PLYNX_ON_READ_IMPL(40);
  PLYNX_ON_READ_IMPL(41);
  PLYNX_ON_READ_IMPL(42);
  PLYNX_ON_READ_IMPL(43);
  PLYNX_ON_READ_IMPL(44);
  PLYNX_ON_READ_IMPL(45);
  PLYNX_ON_READ_IMPL(46);
  PLYNX_ON_READ_IMPL(47);
  PLYNX_ON_READ_IMPL(48);
  PLYNX_ON_READ_IMPL(49);
  PLYNX_ON_READ_IMPL(50);
  PLYNX_ON_READ_IMPL(51);
  PLYNX_ON_READ_IMPL(52);
  PLYNX_ON_READ_IMPL(53);
  PLYNX_ON_READ_IMPL(54);
  PLYNX_ON_READ_IMPL(55);
  PLYNX_ON_READ_IMPL(56);
  PLYNX_ON_READ_IMPL(57);
  PLYNX_ON_READ_IMPL(58);
  PLYNX_ON_READ_IMPL(59);
  PLYNX_ON_READ_IMPL(60);
  PLYNX_ON_READ_IMPL(61);
  PLYNX_ON_READ_IMPL(62);
  PLYNX_ON_READ_IMPL(63);
  PLYNX_ON_READ_IMPL(64);
  PLYNX_ON_READ_IMPL(65);
  PLYNX_ON_READ_IMPL(66);
  PLYNX_ON_READ_IMPL(67);
  PLYNX_ON_READ_IMPL(68);
  PLYNX_ON_READ_IMPL(69);
  PLYNX_ON_READ_IMPL(70);
  PLYNX_ON_READ_IMPL(71);
  PLYNX_ON_READ_IMPL(72);
  PLYNX_ON_READ_IMPL(73);
  PLYNX_ON_READ_IMPL(74);
  PLYNX_ON_READ_IMPL(75);
  PLYNX_ON_READ_IMPL(76);
  PLYNX_ON_READ_IMPL(77);
  PLYNX_ON_READ_IMPL(78);
  PLYNX_ON_READ_IMPL(79);
  PLYNX_ON_READ_IMPL(80);
  PLYNX_ON_READ_IMPL(81);
  PLYNX_ON_READ_IMPL(82);
  PLYNX_ON_READ_IMPL(83);
  PLYNX_ON_READ_IMPL(84);
  PLYNX_ON_READ_IMPL(85);
  PLYNX_ON_READ_IMPL(86);
  PLYNX_ON_READ_IMPL(87);
  PLYNX_ON_READ_IMPL(88);
  PLYNX_ON_READ_IMPL(89);
  PLYNX_ON_READ_IMPL(90);
  PLYNX_ON_READ_IMPL(91);
  PLYNX_ON_READ_IMPL(92);
  PLYNX_ON_READ_IMPL(93);
  PLYNX_ON_READ_IMPL(94);
  PLYNX_ON_READ_IMPL(95);
  PLYNX_ON_READ_IMPL(96);
  PLYNX_ON_READ_IMPL(97);
  PLYNX_ON_READ_IMPL(98);
  PLYNX_ON_READ_IMPL(99);
  PLYNX_ON_READ_IMPL(100);
  PLYNX_ON_READ_IMPL(101);
  PLYNX_ON_READ_IMPL(102);
  PLYNX_ON_READ_IMPL(103);
  PLYNX_ON_READ_IMPL(104);
  PLYNX_ON_READ_IMPL(105);
  PLYNX_ON_READ_IMPL(106);
  PLYNX_ON_READ_IMPL(107);
  PLYNX_ON_READ_IMPL(108);
  PLYNX_ON_READ_IMPL(109);
  PLYNX_ON_READ_IMPL(110);
  PLYNX_ON_READ_IMPL(111);
  PLYNX_ON_READ_IMPL(112);
  PLYNX_ON_READ_IMPL(113);
  PLYNX_ON_READ_IMPL(114);
  PLYNX_ON_READ_IMPL(115);
  PLYNX_ON_READ_IMPL(116);
  PLYNX_ON_READ_IMPL(117);
  PLYNX_ON_READ_IMPL(118);
  PLYNX_ON_READ_IMPL(119);
  PLYNX_ON_READ_IMPL(120);
  PLYNX_ON_READ_IMPL(121);
  PLYNX_ON_READ_IMPL(122);
  PLYNX_ON_READ_IMPL(123);
  PLYNX_ON_READ_IMPL(124);
  PLYNX_ON_READ_IMPL(125);
  PLYNX_ON_READ_IMPL(126);
  PLYNX_ON_READ_IMPL(127);
#endif

PLYNX_ON_WRITE_IMPL(0 );
PLYNX_ON_WRITE_IMPL(1 );
PLYNX_ON_WRITE_IMPL(2 );
PLYNX_ON_WRITE_IMPL(3 );
PLYNX_ON_WRITE_IMPL(4 );
PLYNX_ON_WRITE_IMPL(5 );
PLYNX_ON_WRITE_IMPL(6 );
PLYNX_ON_WRITE_IMPL(7 );
PLYNX_ON_WRITE_IMPL(8 );
PLYNX_ON_WRITE_IMPL(9 );
PLYNX_ON_WRITE_IMPL(10);
PLYNX_ON_WRITE_IMPL(11);
PLYNX_ON_WRITE_IMPL(12);
PLYNX_ON_WRITE_IMPL(13);
PLYNX_ON_WRITE_IMPL(14);
PLYNX_ON_WRITE_IMPL(15);
PLYNX_ON_WRITE_IMPL(16);
PLYNX_ON_WRITE_IMPL(17);
PLYNX_ON_WRITE_IMPL(18);
PLYNX_ON_WRITE_IMPL(19);
PLYNX_ON_WRITE_IMPL(20);
PLYNX_ON_WRITE_IMPL(21);
PLYNX_ON_WRITE_IMPL(22);
PLYNX_ON_WRITE_IMPL(23);
PLYNX_ON_WRITE_IMPL(24);
PLYNX_ON_WRITE_IMPL(25);
PLYNX_ON_WRITE_IMPL(26);
PLYNX_ON_WRITE_IMPL(27);
PLYNX_ON_WRITE_IMPL(28);
PLYNX_ON_WRITE_IMPL(29);
PLYNX_ON_WRITE_IMPL(30);
PLYNX_ON_WRITE_IMPL(31);
#ifdef PLYNX_USE_128_VPINS
  PLYNX_ON_WRITE_IMPL(32);
  PLYNX_ON_WRITE_IMPL(33);
  PLYNX_ON_WRITE_IMPL(34);
  PLYNX_ON_WRITE_IMPL(35);
  PLYNX_ON_WRITE_IMPL(36);
  PLYNX_ON_WRITE_IMPL(37);
  PLYNX_ON_WRITE_IMPL(38);
  PLYNX_ON_WRITE_IMPL(39);
  PLYNX_ON_WRITE_IMPL(40);
  PLYNX_ON_WRITE_IMPL(41);
  PLYNX_ON_WRITE_IMPL(42);
  PLYNX_ON_WRITE_IMPL(43);
  PLYNX_ON_WRITE_IMPL(44);
  PLYNX_ON_WRITE_IMPL(45);
  PLYNX_ON_WRITE_IMPL(46);
  PLYNX_ON_WRITE_IMPL(47);
  PLYNX_ON_WRITE_IMPL(48);
  PLYNX_ON_WRITE_IMPL(49);
  PLYNX_ON_WRITE_IMPL(50);
  PLYNX_ON_WRITE_IMPL(51);
  PLYNX_ON_WRITE_IMPL(52);
  PLYNX_ON_WRITE_IMPL(53);
  PLYNX_ON_WRITE_IMPL(54);
  PLYNX_ON_WRITE_IMPL(55);
  PLYNX_ON_WRITE_IMPL(56);
  PLYNX_ON_WRITE_IMPL(57);
  PLYNX_ON_WRITE_IMPL(58);
  PLYNX_ON_WRITE_IMPL(59);
  PLYNX_ON_WRITE_IMPL(60);
  PLYNX_ON_WRITE_IMPL(61);
  PLYNX_ON_WRITE_IMPL(62);
  PLYNX_ON_WRITE_IMPL(63);
  PLYNX_ON_WRITE_IMPL(64);
  PLYNX_ON_WRITE_IMPL(65);
  PLYNX_ON_WRITE_IMPL(66);
  PLYNX_ON_WRITE_IMPL(67);
  PLYNX_ON_WRITE_IMPL(68);
  PLYNX_ON_WRITE_IMPL(69);
  PLYNX_ON_WRITE_IMPL(70);
  PLYNX_ON_WRITE_IMPL(71);
  PLYNX_ON_WRITE_IMPL(72);
  PLYNX_ON_WRITE_IMPL(73);
  PLYNX_ON_WRITE_IMPL(74);
  PLYNX_ON_WRITE_IMPL(75);
  PLYNX_ON_WRITE_IMPL(76);
  PLYNX_ON_WRITE_IMPL(77);
  PLYNX_ON_WRITE_IMPL(78);
  PLYNX_ON_WRITE_IMPL(79);
  PLYNX_ON_WRITE_IMPL(80);
  PLYNX_ON_WRITE_IMPL(81);
  PLYNX_ON_WRITE_IMPL(82);
  PLYNX_ON_WRITE_IMPL(83);
  PLYNX_ON_WRITE_IMPL(84);
  PLYNX_ON_WRITE_IMPL(85);
  PLYNX_ON_WRITE_IMPL(86);
  PLYNX_ON_WRITE_IMPL(87);
  PLYNX_ON_WRITE_IMPL(88);
  PLYNX_ON_WRITE_IMPL(89);
  PLYNX_ON_WRITE_IMPL(90);
  PLYNX_ON_WRITE_IMPL(91);
  PLYNX_ON_WRITE_IMPL(92);
  PLYNX_ON_WRITE_IMPL(93);
  PLYNX_ON_WRITE_IMPL(94);
  PLYNX_ON_WRITE_IMPL(95);
  PLYNX_ON_WRITE_IMPL(96);
  PLYNX_ON_WRITE_IMPL(97);
  PLYNX_ON_WRITE_IMPL(98);
  PLYNX_ON_WRITE_IMPL(99);
  PLYNX_ON_WRITE_IMPL(100);
  PLYNX_ON_WRITE_IMPL(101);
  PLYNX_ON_WRITE_IMPL(102);
  PLYNX_ON_WRITE_IMPL(103);
  PLYNX_ON_WRITE_IMPL(104);
  PLYNX_ON_WRITE_IMPL(105);
  PLYNX_ON_WRITE_IMPL(106);
  PLYNX_ON_WRITE_IMPL(107);
  PLYNX_ON_WRITE_IMPL(108);
  PLYNX_ON_WRITE_IMPL(109);
  PLYNX_ON_WRITE_IMPL(110);
  PLYNX_ON_WRITE_IMPL(111);
  PLYNX_ON_WRITE_IMPL(112);
  PLYNX_ON_WRITE_IMPL(113);
  PLYNX_ON_WRITE_IMPL(114);
  PLYNX_ON_WRITE_IMPL(115);
  PLYNX_ON_WRITE_IMPL(116);
  PLYNX_ON_WRITE_IMPL(117);
  PLYNX_ON_WRITE_IMPL(118);
  PLYNX_ON_WRITE_IMPL(119);
  PLYNX_ON_WRITE_IMPL(120);
  PLYNX_ON_WRITE_IMPL(121);
  PLYNX_ON_WRITE_IMPL(122);
  PLYNX_ON_WRITE_IMPL(123);
  PLYNX_ON_WRITE_IMPL(124);
  PLYNX_ON_WRITE_IMPL(125);
  PLYNX_ON_WRITE_IMPL(126);
  PLYNX_ON_WRITE_IMPL(127);
#endif

static const WidgetReadHandler PlynxReadHandlerVector[] PLYNX_PROGMEM = {
    PlynxWidgetRead0,   PlynxWidgetRead1,   PlynxWidgetRead2,   PlynxWidgetRead3,
    PlynxWidgetRead4,   PlynxWidgetRead5,   PlynxWidgetRead6,   PlynxWidgetRead7,
    PlynxWidgetRead8,   PlynxWidgetRead9,   PlynxWidgetRead10,  PlynxWidgetRead11,
    PlynxWidgetRead12,  PlynxWidgetRead13,  PlynxWidgetRead14,  PlynxWidgetRead15,
    PlynxWidgetRead16,  PlynxWidgetRead17,  PlynxWidgetRead18,  PlynxWidgetRead19,
    PlynxWidgetRead20,  PlynxWidgetRead21,  PlynxWidgetRead22,  PlynxWidgetRead23,
    PlynxWidgetRead24,  PlynxWidgetRead25,  PlynxWidgetRead26,  PlynxWidgetRead27,
    PlynxWidgetRead28,  PlynxWidgetRead29,  PlynxWidgetRead30,  PlynxWidgetRead31,
#ifdef PLYNX_USE_128_VPINS
    PlynxWidgetRead32,  PlynxWidgetRead33,  PlynxWidgetRead34,  PlynxWidgetRead35,
    PlynxWidgetRead36,  PlynxWidgetRead37,  PlynxWidgetRead38,  PlynxWidgetRead39,
    PlynxWidgetRead40,  PlynxWidgetRead41,  PlynxWidgetRead42,  PlynxWidgetRead43,
    PlynxWidgetRead44,  PlynxWidgetRead45,  PlynxWidgetRead46,  PlynxWidgetRead47,
    PlynxWidgetRead48,  PlynxWidgetRead49,  PlynxWidgetRead50,  PlynxWidgetRead51,
    PlynxWidgetRead52,  PlynxWidgetRead53,  PlynxWidgetRead54,  PlynxWidgetRead55,
    PlynxWidgetRead56,  PlynxWidgetRead57,  PlynxWidgetRead58,  PlynxWidgetRead59,
    PlynxWidgetRead60,  PlynxWidgetRead61,  PlynxWidgetRead62,  PlynxWidgetRead63,
    PlynxWidgetRead64,  PlynxWidgetRead65,  PlynxWidgetRead66,  PlynxWidgetRead67,
    PlynxWidgetRead68,  PlynxWidgetRead69,  PlynxWidgetRead70,  PlynxWidgetRead71,
    PlynxWidgetRead72,  PlynxWidgetRead73,  PlynxWidgetRead74,  PlynxWidgetRead75,
    PlynxWidgetRead76,  PlynxWidgetRead77,  PlynxWidgetRead78,  PlynxWidgetRead79,
    PlynxWidgetRead80,  PlynxWidgetRead81,  PlynxWidgetRead82,  PlynxWidgetRead83,
    PlynxWidgetRead84,  PlynxWidgetRead85,  PlynxWidgetRead86,  PlynxWidgetRead87,
    PlynxWidgetRead88,  PlynxWidgetRead89,  PlynxWidgetRead90,  PlynxWidgetRead91,
    PlynxWidgetRead92,  PlynxWidgetRead93,  PlynxWidgetRead94,  PlynxWidgetRead95,
    PlynxWidgetRead96,  PlynxWidgetRead97,  PlynxWidgetRead98,  PlynxWidgetRead99,
    PlynxWidgetRead100,  PlynxWidgetRead101,  PlynxWidgetRead102,  PlynxWidgetRead103,
    PlynxWidgetRead104,  PlynxWidgetRead105,  PlynxWidgetRead106,  PlynxWidgetRead107,
    PlynxWidgetRead108,  PlynxWidgetRead109,  PlynxWidgetRead110,  PlynxWidgetRead111,
    PlynxWidgetRead112,  PlynxWidgetRead113,  PlynxWidgetRead114,  PlynxWidgetRead115,
    PlynxWidgetRead116,  PlynxWidgetRead117,  PlynxWidgetRead118,  PlynxWidgetRead119,
    PlynxWidgetRead120,  PlynxWidgetRead121,  PlynxWidgetRead122,  PlynxWidgetRead123,
    PlynxWidgetRead124,  PlynxWidgetRead125,  PlynxWidgetRead126,  PlynxWidgetRead127,
#endif
};

static const WidgetWriteHandler PlynxWriteHandlerVector[] PLYNX_PROGMEM = {
    PlynxWidgetWrite0,  PlynxWidgetWrite1,  PlynxWidgetWrite2,  PlynxWidgetWrite3,
    PlynxWidgetWrite4,  PlynxWidgetWrite5,  PlynxWidgetWrite6,  PlynxWidgetWrite7,
    PlynxWidgetWrite8,  PlynxWidgetWrite9,  PlynxWidgetWrite10, PlynxWidgetWrite11,
    PlynxWidgetWrite12, PlynxWidgetWrite13, PlynxWidgetWrite14, PlynxWidgetWrite15,
    PlynxWidgetWrite16, PlynxWidgetWrite17, PlynxWidgetWrite18, PlynxWidgetWrite19,
    PlynxWidgetWrite20, PlynxWidgetWrite21, PlynxWidgetWrite22, PlynxWidgetWrite23,
    PlynxWidgetWrite24, PlynxWidgetWrite25, PlynxWidgetWrite26, PlynxWidgetWrite27,
    PlynxWidgetWrite28, PlynxWidgetWrite29, PlynxWidgetWrite30, PlynxWidgetWrite31,
#ifdef PLYNX_USE_128_VPINS
    PlynxWidgetWrite32,  PlynxWidgetWrite33,  PlynxWidgetWrite34,  PlynxWidgetWrite35,
    PlynxWidgetWrite36,  PlynxWidgetWrite37,  PlynxWidgetWrite38,  PlynxWidgetWrite39,
    PlynxWidgetWrite40,  PlynxWidgetWrite41,  PlynxWidgetWrite42,  PlynxWidgetWrite43,
    PlynxWidgetWrite44,  PlynxWidgetWrite45,  PlynxWidgetWrite46,  PlynxWidgetWrite47,
    PlynxWidgetWrite48,  PlynxWidgetWrite49,  PlynxWidgetWrite50,  PlynxWidgetWrite51,
    PlynxWidgetWrite52,  PlynxWidgetWrite53,  PlynxWidgetWrite54,  PlynxWidgetWrite55,
    PlynxWidgetWrite56,  PlynxWidgetWrite57,  PlynxWidgetWrite58,  PlynxWidgetWrite59,
    PlynxWidgetWrite60,  PlynxWidgetWrite61,  PlynxWidgetWrite62,  PlynxWidgetWrite63,
    PlynxWidgetWrite64,  PlynxWidgetWrite65,  PlynxWidgetWrite66,  PlynxWidgetWrite67,
    PlynxWidgetWrite68,  PlynxWidgetWrite69,  PlynxWidgetWrite70,  PlynxWidgetWrite71,
    PlynxWidgetWrite72,  PlynxWidgetWrite73,  PlynxWidgetWrite74,  PlynxWidgetWrite75,
    PlynxWidgetWrite76,  PlynxWidgetWrite77,  PlynxWidgetWrite78,  PlynxWidgetWrite79,
    PlynxWidgetWrite80,  PlynxWidgetWrite81,  PlynxWidgetWrite82,  PlynxWidgetWrite83,
    PlynxWidgetWrite84,  PlynxWidgetWrite85,  PlynxWidgetWrite86,  PlynxWidgetWrite87,
    PlynxWidgetWrite88,  PlynxWidgetWrite89,  PlynxWidgetWrite90,  PlynxWidgetWrite91,
    PlynxWidgetWrite92,  PlynxWidgetWrite93,  PlynxWidgetWrite94,  PlynxWidgetWrite95,
    PlynxWidgetWrite96,  PlynxWidgetWrite97,  PlynxWidgetWrite98,  PlynxWidgetWrite99,
    PlynxWidgetWrite100,  PlynxWidgetWrite101,  PlynxWidgetWrite102,  PlynxWidgetWrite103,
    PlynxWidgetWrite104,  PlynxWidgetWrite105,  PlynxWidgetWrite106,  PlynxWidgetWrite107,
    PlynxWidgetWrite108,  PlynxWidgetWrite109,  PlynxWidgetWrite110,  PlynxWidgetWrite111,
    PlynxWidgetWrite112,  PlynxWidgetWrite113,  PlynxWidgetWrite114,  PlynxWidgetWrite115,
    PlynxWidgetWrite116,  PlynxWidgetWrite117,  PlynxWidgetWrite118,  PlynxWidgetWrite119,
    PlynxWidgetWrite120,  PlynxWidgetWrite121,  PlynxWidgetWrite122,  PlynxWidgetWrite123,
    PlynxWidgetWrite124,  PlynxWidgetWrite125,  PlynxWidgetWrite126,  PlynxWidgetWrite127,
#endif
};

WidgetReadHandler GetReadHandler(uint8_t pin)
{
    if (pin >= PLYNX_COUNT_OF(PlynxReadHandlerVector))
        return NULL;
#ifdef PLYNX_HAS_PROGMEM
    return (WidgetReadHandler)pgm_read_word(&PlynxReadHandlerVector[pin]);
#else
    return PlynxReadHandlerVector[pin];
#endif
}

WidgetWriteHandler GetWriteHandler(uint8_t pin)
{
    if (pin >= PLYNX_COUNT_OF(PlynxWriteHandlerVector))
        return NULL;
#ifdef PLYNX_HAS_PROGMEM
    return (WidgetWriteHandler)pgm_read_word(&PlynxWriteHandlerVector[pin]);
#else
    return PlynxWriteHandlerVector[pin];
#endif
}
