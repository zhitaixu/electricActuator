#pragma once
#include <Arduino.h>

struct HallSample {
  uint8_t u:1;
  uint8_t v:1;
  uint8_t w:1;
  uint8_t code;   // (u<<2)|(v<<1)|w
};

class HallReader {
public:
  void begin(int pinU, int pinV, int pinW);
  HallSample read();
  bool isValid(uint8_t code) const; // 有效: 001/010/011/100/101/110
};