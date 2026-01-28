#pragma once
#include <Arduino.h>
#include <WebServer.h>
#include "app/control.h"

class ApWeb {
public:
  void begin(Control* ctl);
  void tick();
private:
  WebServer _srv{80};
  Control* _ctl=nullptr;
};
