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
  int dirSign=+1;
  bool dirKnown=false;
  int speedPct=0;
  int torquePct=100;
  uint8_t hallMap=0;
  // UI/Keys telemetry
  bool displayOn=false;
  bool adjustMode=false;
  bool pageValve=false;
  int kvsPct=0;
  int displayValue=0;
  bool ledRun=false;
  bool ledValve=false;
  bool ledKvs=false;
  bool ledTemp=false;
  bool ledPct=false;
  bool keyDownMinus=false;
  bool keyDownPlus=false;
  bool keyDownOk=false;
  bool keyPressMinus=false;
  bool keyPressPlus=false;
  bool keyPressOk=false;
};

class Control {
public:
  void begin(Drv8313Hall* drv, HallReader* hall, PotAdc* pot);
  void setEnabled(bool en);
  void setTarget(float deg);
  void requestMove(bool on);
  void detectDirection();
  void setDirSign(int sign);
  void setSpeedPct(int pct);
  void setTorquePct(int pct);
  void setHallMap(uint8_t map);
  void autoScanHall();
  void startHallCapture(uint32_t ms);
  const char* hallSeq() const { return _hallSeqStr; }
  void setUiState(bool displayOn, bool adjustMode, bool pageValve, int kvsPct, int displayValue,
                  bool ledRun, bool ledValve, bool ledKvs, bool ledTemp, bool ledPct);
  void setKeyState(bool downMinus, bool downPlus, bool downOk,
                   bool pressMinus, bool pressPlus, bool pressOk);
  void injectKeyPress(bool minus, bool plus, bool ok);
  void consumeInjected(bool &minus, bool &plus, bool &ok);
  void tick();
  ControlStatus status() const { return _st; }

private:
  Drv8313Hall* _drv=nullptr;
  HallReader* _hall=nullptr;
  PotAdc* _pot=nullptr;

  ControlStatus _st{};
  bool _moveReq=false;

  uint32_t _tHallChange=0;
  uint8_t _lastHall=0;
  uint32_t _tLastDetect=0;
  float _speedDuty=0.0f;

  bool _capActive=false;
  uint32_t _capEnd=0;
  uint8_t _capLast=0xFF;
  uint8_t _capSeq[16]{};
  uint8_t _capLen=0;
  char _hallSeqStr[64]{};
  bool _vkMinus=false, _vkPlus=false, _vkOk=false;

  float computeDuty(float errDeg);
};
