#pragma once
#include "drivers/drv8313_hall.h"
#include "drivers/hall.h"
#include "drivers/pot_adc.h"

struct ControlStatus {
  bool enabled=false;
  bool running=false;
  float targetDeg=0;
  float curDeg=0;
  uint8_t hall=0;
  int dirSign=+1; // +1: 正向，-1: 反向（由 Detect Dir 自动识别）
  bool dirKnown=false;
};

class Control {
public:
  void begin(Drv8313Hall* drv, HallReader* hall, PotAdc* pot);
  void setEnabled(bool en);
  void setTarget(float deg);
  void requestMove(bool on);
  void detectDirection(); // 小步试转，判断 pot 变大还是变小
  void tick();            // 1~2ms 调用一次即可
  ControlStatus status() const { return _st; }

private:
  Drv8313Hall* _drv=nullptr;
  HallReader* _hall=nullptr;
  PotAdc* _pot=nullptr;

  ControlStatus _st{};
  bool _moveReq=false;

  uint32_t _tHallChange=0;
  uint8_t _lastHall=0;

  float computeDuty(float errDeg);
};