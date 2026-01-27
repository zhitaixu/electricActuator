#include "drivers/keys.h"

void Keys::begin(int pinMinus, int pinPlus, int pinOk){
  _m=pinMinus; _p=pinPlus; _o=pinOk;
  pinMode(_m, INPUT);
  pinMode(_p, INPUT);
  pinMode(_o, INPUT);
}

KeyState Keys::readRaw(){
  KeyState s;
  s.minus = (digitalRead(_m) == LOW);
  s.plus  = (digitalRead(_p) == LOW);
  s.ok    = (digitalRead(_o) == LOW);
  return s;
}