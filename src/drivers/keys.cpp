#include "drivers/keys.h"

static inline bool sameState(const KeyState& a, const KeyState& b){
  return a.minus==b.minus && a.plus==b.plus && a.ok==b.ok;
}

void Keys::begin(int pinMinus, int pinPlus, int pinOk){
  _m=pinMinus; _p=pinPlus; _o=pinOk;
  pinMode(_m, INPUT);
  pinMode(_p, INPUT);
  pinMode(_o, INPUT);
  _raw = readRaw();
  _stable = _raw;
  _prevStable = _stable;
  _tChange = millis();
}

KeyState Keys::readRaw(){
  KeyState s;
  s.minus = (digitalRead(_m) == LOW);
  s.plus  = (digitalRead(_p) == LOW);
  s.ok    = (digitalRead(_o) == LOW);
  return s;
}

void Keys::update(){
  KeyState r = readRaw();
  if(!sameState(r, _raw)){
    _raw = r;
    _tChange = millis();
  }
  if((int)(millis() - _tChange) > DEBOUNCE_MS){
    if(!sameState(_stable, _raw)){
      _prevStable = _stable;
      _stable = _raw;
    }
  }
}

bool Keys::pressedMinus() const { return _stable.minus && !_prevStable.minus; }
bool Keys::pressedPlus()  const { return _stable.plus  && !_prevStable.plus; }
bool Keys::pressedOk()    const { return _stable.ok    && !_prevStable.ok; }
