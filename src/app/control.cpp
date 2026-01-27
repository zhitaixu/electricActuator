#include "control.h"
#include "../../include/config_calib.h"

void Control::begin(Drv8313Hall* drv, HallReader* hall, PotAdc* pot){
  _drv=drv; _hall=hall; _pot=pot;
  _tHallChange = millis();
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

float Control::computeDuty(float errDeg){
  float duty = fabs(errDeg) * KP_DUTY_PER_DEG;
  if(duty < DUTY_MIN) duty = DUTY_MIN;
  if(duty > DUTY_MAX) duty = DUTY_MAX;
  return duty;
}

void Control::detectDirection(){
  if(!_st.enabled) return;

  float d0 = _pot->readDegFiltered();

  // 轻轻 kick 一下“正向”，再读角度
  _drv->kickOpenLoop(+1, 0.18f, 140);
  delay(80);
  float d1 = _pot->readDegFiltered();

  // 角度增大，说明 dirSign=+1 代表“角度增大方向”
  // 角度减小，说明相反
  if (d1 > d0 + 0.2f) { _st.dirSign = +1; _st.dirKnown = true; }
  else if (d1 < d0 - 0.2f) { _st.dirSign = -1; _st.dirKnown = true; }
  else {
    // 可能没动起来，给一次更强的kick
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

  // 记录 hall 是否变化，用于“卡住时kick”
  if(hs.code != _lastHall){
    _lastHall = hs.code;
    _tHallChange = millis();
  }

  if(!_moveReq){
    _st.running=false;
    _drv->driveHall(hs.code, +1, 0); // 相当于 floatAll（内部会处理无效码）
    return;
  }

  float err = _st.targetDeg - _st.curDeg;
  if(fabs(err) <= DEG_DEADBAND){
    _st.running=false;
    _moveReq=false;
    _drv->driveHall(hs.code, +1, 0);
    return;
  }

  // 没做方向识别之前，不允许乱跑（否则你会以为“代码错”，其实是方向猜错）
  if(!_st.dirKnown){
    _st.running=false;
    _drv->driveHall(hs.code, +1, 0);
    return;
  }

  int dir = (err > 0) ? _st.dirSign : -_st.dirSign;
  float duty = computeDuty(err);

  // Hall 无效或卡住：先 kick，再回来闭环
  if(!_hall->isValid(hs.code) || (int)(millis() - _tHallChange) > HALL_STUCK_MS){
    _drv->kickOpenLoop(dir, KICK_DUTY, KICK_MS);
    _tHallChange = millis();
    return;
  }

  _st.running=true;
  _drv->driveHall(hs.code, dir, duty);
}