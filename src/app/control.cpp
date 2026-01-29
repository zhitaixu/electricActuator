#include "control.h"
#include "config_calib.h"

void Control::begin(Drv8313Hall* drv, HallReader* hall, PotAdc* pot){
  _drv=drv; _hall=hall; _pot=pot;
  _tHallChange = millis();
  _tLastDetect = 0;
  _speedDuty = 0.0f;
  _st.hallMap = DEFAULT_HALL_MAP;
  _st.dirSign = DEFAULT_DIR_SIGN;
  _st.dirKnown = true;
  if(_drv){
    _drv->setHallMap(_st.hallMap);
  }
  setSpeedPct(100);
  setTorquePct(100);
}

void Control::setEnabled(bool en){
  _st.enabled = en;
  if(_drv) _drv->enable(en);
  if(!en){
    _st.running=false;
    _moveReq=false;
  }
}

void Control::setTarget(float deg){
  if(deg<0) deg=0;
  if(deg>LOGIC_MAX_DEG) deg=LOGIC_MAX_DEG;
  _st.targetDeg = deg;
}

void Control::requestMove(bool on){
  _moveReq = on;
}

void Control::setDirSign(int sign){
  _st.dirSign = (sign >= 0) ? +1 : -1;
  _st.dirKnown = true;
}

void Control::setSpeedPct(int pct){
  if(pct < 0) pct = 0;
  if(pct > 100) pct = 100;
  _st.speedPct = pct;
  if(pct == 0){
    _speedDuty = 0.0f;
  }else{
    float k = pct / 100.0f;
    _speedDuty = DUTY_MIN + (DUTY_MAX - DUTY_MIN) * k;
  }
}

void Control::setTorquePct(int pct){
  if(pct < 10) pct = 10;
  if(pct > 300) pct = 300;
  _st.torquePct = pct;
}

void Control::setHallMap(uint8_t map){
  _st.hallMap = map;
  if(_drv) _drv->setHallMap(map);
}

void Control::autoScanHall(){
  if(!_st.enabled || !_drv || !_hall) return;
  const int testMs = 350;
  const float duty = (KICK_DUTY > 0.12f) ? KICK_DUTY : 0.18f;
  uint8_t bestMap = _st.hallMap;
  int bestScore = -1;

  for(uint8_t m=0; m<6; ++m){
    _drv->setHallMap(m);
    int score = 0;
    uint8_t last = 0xFF;
    uint32_t t0 = millis();
    while((int)(millis()-t0) < testMs){
      HallSample hs = _hall->read();
      uint8_t h = hs.code;
      if(_hall->isValid(_drv->remapHall(h))){
        if(h != last){
          score++;
          last = h;
        }
      }
      _drv->driveHall(h, _st.dirSign, duty);
      delayMicroseconds(2500);
    }
    _drv->driveHall(last, _st.dirSign, 0);
    if(score > bestScore){
      bestScore = score;
      bestMap = m;
    }
  }

  setHallMap(bestMap);
}

void Control::startHallCapture(uint32_t ms){
  _capActive = true;
  _capEnd = millis() + ms;
  _capLen = 0;
  _capLast = 0xFF;
  _hallSeqStr[0] = '\0';
}

void Control::setUiState(bool displayOn, bool adjustMode, bool pageValve, int kvsPct, int displayValue,
                         bool ledRun, bool ledValve, bool ledKvs, bool ledTemp, bool ledPct){
  _st.displayOn = displayOn;
  _st.adjustMode = adjustMode;
  _st.pageValve = pageValve;
  _st.kvsPct = kvsPct;
  _st.displayValue = displayValue;
  _st.ledRun = ledRun;
  _st.ledValve = ledValve;
  _st.ledKvs = ledKvs;
  _st.ledTemp = ledTemp;
  _st.ledPct = ledPct;
}

void Control::setKeyState(bool downMinus, bool downPlus, bool downOk,
                          bool pressMinus, bool pressPlus, bool pressOk){
  _st.keyDownMinus = downMinus;
  _st.keyDownPlus = downPlus;
  _st.keyDownOk = downOk;
  _st.keyPressMinus = pressMinus;
  _st.keyPressPlus = pressPlus;
  _st.keyPressOk = pressOk;
}

void Control::injectKeyPress(bool minus, bool plus, bool ok){
  _vkMinus = _vkMinus || minus;
  _vkPlus = _vkPlus || plus;
  _vkOk = _vkOk || ok;
}

void Control::consumeInjected(bool &minus, bool &plus, bool &ok){
  minus = _vkMinus;
  plus = _vkPlus;
  ok = _vkOk;
  _vkMinus = _vkPlus = _vkOk = false;
}

float Control::computeDuty(float errDeg){
  float duty = fabs(errDeg) * KP_DUTY_PER_DEG;
  if(duty < DUTY_MIN) duty = DUTY_MIN;
  if(duty > DUTY_MAX) duty = DUTY_MAX;
  duty *= (_st.speedPct / 100.0f);
  duty *= (_st.torquePct / 100.0f);
  if(duty > 1.0f) duty = 1.0f;
  if(duty < 0.0f) duty = 0.0f;
  return duty;
}

void Control::detectDirection(){
  if(!USE_POT_FEEDBACK) return;
  if(!_st.enabled) return;

  float d0 = _pot->readDegFiltered();

  _drv->kickOpenLoop(+1, 0.18f, 140);
  delay(80);
  float d1 = _pot->readDegFiltered();

  if (d1 > d0 + 0.2f) { _st.dirSign = +1; _st.dirKnown = true; }
  else if (d1 < d0 - 0.2f) { _st.dirSign = -1; _st.dirKnown = true; }
  else {
    _drv->kickOpenLoop(+1, 0.22f, 180);
    delay(80);
    float d2 = _pot->readDegFiltered();
    if (d2 > d0 + 0.2f) { _st.dirSign = +1; _st.dirKnown = true; }
    else if (d2 < d0 - 0.2f) { _st.dirSign = -1; _st.dirKnown = true; }
  }
}

void Control::tick(){
  if(!_st.enabled || !_drv || !_hall || !_pot) return;
  if(_drv->faulted()){
    _st.running=false;
    _moveReq=false;
    return;
  }

  _st.curDeg = _pot->readDegFiltered();

  HallSample hs = _hall->read();
  _st.hall = hs.code;

  if(_capActive){
    uint8_t h = hs.code;
    if(_hall->isValid(h) && h != _capLast){
      if(_capLen < sizeof(_capSeq)){
        _capSeq[_capLen++] = h;
      }
      _capLast = h;
    }
    if((int)(millis() - _capEnd) >= 0){
      _capActive = false;
      int off = 0;
      for(uint8_t i=0; i<_capLen; ++i){
        uint8_t c = _capSeq[i];
        int u=(c>>2)&1, v=(c>>1)&1, w=c&1;
        if(i>0 && off < (int)sizeof(_hallSeqStr)-1){
          _hallSeqStr[off++]='-';
        }
        if(off < (int)sizeof(_hallSeqStr)-3){
          _hallSeqStr[off++] = u?'1':'0';
          _hallSeqStr[off++] = v?'1':'0';
          _hallSeqStr[off++] = w?'1':'0';
        }
      }
      _hallSeqStr[off] = '\0';
    }
  }

  if(hs.code != _lastHall){
    _lastHall = hs.code;
    _tHallChange = millis();
  }

  if(!_moveReq){
    _st.running=false;
    _drv->driveHall(hs.code, +1, 0);
    return;
  }

  if(!USE_POT_FEEDBACK){
    if(!_st.dirKnown){
      _st.running=false;
      _drv->driveHall(hs.code, +1, 0);
      return;
    }
    if(!_hall->isValid(hs.code) || (int)(millis() - _tHallChange) > HALL_STUCK_MS){
      _drv->kickOpenLoop(_st.dirSign, KICK_DUTY, KICK_MS);
      _tHallChange = millis();
      return;
    }
    _st.running = (_speedDuty > 0.0f);
    _drv->driveHall(hs.code, _st.dirSign, _speedDuty);
    return;
  }

  float err = _st.targetDeg - _st.curDeg;
  if(fabs(err) <= DEG_DEADBAND){
    _st.running=false;
    _moveReq=false;
    _drv->driveHall(hs.code, +1, 0);
    return;
  }

  if(!_st.dirKnown){
    if((int)(millis() - _tLastDetect) > DETECT_RETRY_MS){
      _tLastDetect = millis();
      detectDirection();
    }
    _st.running=false;
    _drv->driveHall(hs.code, +1, 0);
    return;
  }

  int dir = (err > 0) ? _st.dirSign : -_st.dirSign;
  float duty = computeDuty(err);

  if(!_hall->isValid(hs.code) || (int)(millis() - _tHallChange) > HALL_STUCK_MS){
    float kd = KICK_DUTY * (_st.torquePct / 100.0f);
    if(kd > 1.0f) kd = 1.0f;
    _drv->kickOpenLoop(dir, kd, KICK_MS);
    _tHallChange = millis();
    return;
  }

  _st.running=true;
  _drv->driveHall(hs.code, dir, duty);
}
