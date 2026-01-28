#include "drivers/shift595.h"

// 约定 bit0=QA, bit1=QB ... bit7=QH
// 返回：byteIndex(0..2), bit(0..7)
static inline void mapLed(int led, int &bi, int &bit){
  // 595#1：QF(5)->LED1 ... QB(1)->LED5
  if (led>=1 && led<=5){
    bi = 0;
    static const int bits[5] = {5,4,3,2,1};
    bit = bits[led-1];
    return;
  }
  // 595#2：QH(7)->LED6 ... QB(1)->LED12
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
  digitalWrite(_oe, LOW);

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

static const uint8_t SEG_MASKS[10] = {
  0x3F, // 0 = a b c d e f
  0x06, // 1 = b c
  0x5B, // 2 = a b d e g
  0x4F, // 3 = a b c d g
  0x66, // 4 = f g b c
  0x6D, // 5 = a f g c d
  0x7D, // 6 = a f e d c g
  0x07, // 7 = a b c
  0x7F, // 8 = a b c d e f g
  0x6F  // 9 = a b c d f g
};

static const int SEG_LED_LEFT[7]  = {6,8,11,12,10,7,9};     // a b c d e f g
static const int SEG_LED_RIGHT[7] = {13,15,18,19,17,14,16}; // a b c d e f g

void Shift595::setDigit(int digitIndex0left1right, int value){
  const int* segLed = (digitIndex0left1right==0) ? SEG_LED_LEFT : SEG_LED_RIGHT;
  uint8_t mask = (value >=0 && value <=9) ? SEG_MASKS[value] : 0;
  for(int i=0;i<7;i++){
    bool on = (mask >> i) & 0x1;
    setLed(segLed[i], on);
  }
}

void Shift595::apply(){
  digitalWrite(_rclk, LOW);
  for(int i=2;i>=0;i--){
    shiftOut(_ser, _srclk, MSBFIRST, _b[i]);
  }
  digitalWrite(_rclk, HIGH);
}
