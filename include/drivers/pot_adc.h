#pragma once
#include <Arduino.h>

class PotAdc {
public:
  void begin(int pinAdc);
  int  readRaw();
  float rawToDeg(int raw) const;     // 0..93(物理) -> 映射到 0..90(逻辑)
  float readDegFiltered();           // 简单低通
private:
  int _pin = -1;
  float _f = 0;
  bool _inited = false;
};