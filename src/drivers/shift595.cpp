#include "drivers/shift595.h"

// 关键：按你“最终映射”写死表（QA不用，但仍占 bit0）
// 约定 bit0=QA, bit1=QB ... bit7=QH

// 返回：byteIndex(0..2), bit(0..7)
static inline void mapLed(int led, int &bi, int &bit){
  // 595#1：QF(5)->LED1 ... QB(1)->LED5
  // LED1=QF(bit5), LED2=QE(bit4), LED3=QD(bit3), LED4=QC(bit2), LED5=QB(bit1)
  if (led>=1 && led<=5){
    bi = 0;
    static const int bits[5] = {5,4,3,2,1};
    bit = bits[led-1];
    return;
  }
  // 595#2：QH(7)->LED6 ... QB(1)->LED12
  // LED6=QH(bit7) ... LED12=QB(bit1)
  if (led>=6 && led<=12){
    bi = 1;
    static const int bits[7] = {7,6,5,4,3,2,1};
    bit = bits[led-6];
    return;
  }
  // 595#3：QH(7)->LED13 ... QB(1)->LED19
  if (led>=13 && led<=19){
    bi = 2;
    static const int bits[7] = {7,6,5,4,3,2,1};
    bit = bits[led-13];
    return;
  }
  bi = 0; bit = 0;
}

void Shift595::begin(int ser, int srclk, int rclk, int oe){
  _ser=ser; _srclk=srclk; _rclk=rclk; _oe=oe;
  pinMode(_ser, OUTPUT);
  pinMode(_srclk, OUTPUT);
  pinMode(_rclk, OUTPUT);
  pinMode(_oe, OUTPUT);

  digitalWrite(_ser, LOW);
  digitalWrite(_srclk, LOW);
  digitalWrite(_rclk, LOW);
  digitalWrite(_oe, LOW); // 低使能

  setAllOff();
  apply();
}

void Shift595::setAllOff(){
  _b[0]=_b[1]=_b[2]=0;
}

void Shift595::setLed(int ledIndex1to19, bool on){
  int bi, bit;
  mapLed(ledIndex1to19, bi, bit);
  if(on) _b[bi] |=  (1u<<bit);
  else   _b[bi] &= ~(1u<<bit);
}

void Shift595::apply(){
  // 级联：先移入最后一片，再到第一片（最终第一片最近 MCU）
  digitalWrite(_rclk, LOW);
  for(int i=2;i>=0;i--){
    shiftOut(_ser, _srclk, MSBFIRST, _b[i]);
  }
  digitalWrite(_rclk, HIGH);
}