// LED UI layer: simple wrapper around Shift595 to show run status.
#pragma once

#include "drivers/shift595.h"

class UiLed {
public:
  void begin(Shift595* s);
  void setRunning(bool on);
  void setValve(bool on);
  void setKvs(bool on);
  void setTemp(bool on);
  void setPct(bool on);
  void setDigits(int tens, int ones);
  void apply();

private:
  Shift595* _s = nullptr;
  bool _run = false;
  bool _valve = false;
  bool _kvs = false;
  bool _temp = false;
  bool _pct = false;
  int _tens = -1;
  int _ones = -1;
};
