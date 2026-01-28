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
  KeyState readRaw(); // external pull-up, pressed = LOW
  void update();
  bool pressedMinus() const;
  bool pressedPlus() const;
  bool pressedOk() const;
  bool isDownMinus() const { return _stable.minus; }
  bool isDownPlus() const { return _stable.plus; }
  bool isDownOk() const { return _stable.ok; }
private:
  int _m=-1,_p=-1,_o=-1;
  KeyState _raw{}, _stable{}, _prevStable{};
  uint32_t _tChange=0;
  static const int DEBOUNCE_MS = 30;
};
