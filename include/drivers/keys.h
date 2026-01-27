#pragma once
#include <Arduino.h>

struct KeyState {
  bool minus=false;
  bool plus=false;
  bool ok=false;
};

class Keys {
public:
  void begin(int pinMinus, int pinPlus, int pinOk);
  KeyState readRaw(); // 外部上拉，按下=0
private:
  int _m=-1,_p=-1,_o=-1;
};