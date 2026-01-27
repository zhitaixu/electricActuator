// LED UI layer: simple wrapper around Shift595 to show run status.
#pragma once

#include "drivers/shift595.h"

class UiLed {
public:
  void begin(Shift595* s);
  void setRunning(bool on);
  void apply();

private:
  Shift595* _s = nullptr;
  bool _run = false;
};
