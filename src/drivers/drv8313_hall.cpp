#include "drivers/drv8313_hall.h"
#include "../../include/config_pins.h"
#include "../../include/config_calib.h"

static inline int dutyToRaw(float duty){
  if(duty<0) duty=0;
  if(duty>1) duty=1;
  return (int)(duty * (float)Drv8313Hall::PWM_MAX + 0.5f);
}

void Drv8313Hall::begin(){
  pinMode(PIN_IN1, OUTPUT); digitalWrite(PIN_IN1, LOW);
  pinMode(PIN_IN2, OUTPUT); digitalWrite(PIN_IN2, LOW);
  pinMode(PIN_IN3, OUTPUT); digitalWrite(PIN_IN3, LOW);

  pinMode(PIN_NSLEEP, OUTPUT); digitalWrite(PIN_NSLEEP, LOW);
  pinMode(PIN_NRESET, OUTPUT); digitalWrite(PIN_NRESET, LOW);
  pinMode(PIN_NFAULT, INPUT);

  ledcSetup(CH1, PWM_FREQ, PWM_BITS);
  ledcSetup(CH2, PWM_FREQ, PWM_BITS);
  ledcSetup(CH3, PWM_FREQ, PWM_BITS);
  ledcAttachPin(PIN_EN1, CH1);
  ledcAttachPin(PIN_EN2, CH2);
  ledcAttachPin(PIN_EN3, CH3);
  floatAll();
}

void Drv8313Hall::enable(bool en){
  if(en){
    digitalWrite(PIN_NRESET, HIGH);
    delay(2);
    digitalWrite(PIN_NSLEEP, HIGH);
    delay(2);
    _enabled=true;
  }else{
    floatAll();
    digitalWrite(PIN_NSLEEP, LOW);
    digitalWrite(PIN_NRESET, LOW);
    _enabled=false;
  }
}

bool Drv8313Hall::faulted() const {
  return digitalRead(PIN_NFAULT) == LOW;
}

void Drv8313Hall::setPWM(float duty1, float duty2, float duty3){
  ledcWrite(CH1, dutyToRaw(duty1));
  ledcWrite(CH2, dutyToRaw(duty2));
  ledcWrite(CH3, dutyToRaw(duty3));
}

void Drv8313Hall::floatAll(){
  setPWM(0,0,0);
}

// DRV8313：IN=1 高侧，IN=0 低侧；EN=PWM
void Drv8313Hall::drive2(int highPh, int lowPh, int floatPh, float duty){
  // float 相 PWM=0
  if(floatPh==1) ledcWrite(CH1, 0);
  if(floatPh==2) ledcWrite(CH2, 0);
  if(floatPh==3) ledcWrite(CH3, 0);

  // high 相 IN=1
  if(highPh==1) digitalWrite(PIN_IN1, HIGH);
  if(highPh==2) digitalWrite(PIN_IN2, HIGH);
  if(highPh==3) digitalWrite(PIN_IN3, HIGH);

  // low 相 IN=0
  if(lowPh==1) digitalWrite(PIN_IN1, LOW);
  if(lowPh==2) digitalWrite(PIN_IN2, LOW);
  if(lowPh==3) digitalWrite(PIN_IN3, LOW);

  // high/low 相 PWM=duty
  int raw = dutyToRaw(duty);
  if(highPh==1 || lowPh==1) ledcWrite(CH1, raw);
  if(highPh==2 || lowPh==2) ledcWrite(CH2, raw);
  if(highPh==3 || lowPh==3) ledcWrite(CH3, raw);
}

// Hall -> 两相通电表（这是标准 120°六步的一种映射）
// 你最终 U=ph1, V=ph2, W=ph3 已固定；方向用 dir 反转高低即可。
void Drv8313Hall::driveHall(uint8_t h, int dir, float duty){
  if(!_enabled) return;

  // 对应关系：hall code -> (high, low, float)
  int high=0, low=0, flt=0;

  // 一套常用映射（若能转就不要动；若以后出现“抖但不转”，再换表）
  // 001: U+ V- (W float)
  // 101: U+ W-
  // 100: V+ W-
  // 110: V+ U-
  // 010: W+ U-
  // 011: W+ V-
  switch(h){
    case 0b001: high=PH1; low=PH2; flt=PH3; break;
    case 0b101: high=PH1; low=PH3; flt=PH2; break;
    case 0b100: high=PH2; low=PH3; flt=PH1; break;
    case 0b110: high=PH2; low=PH1; flt=PH3; break;
    case 0b010: high=PH3; low=PH1; flt=PH2; break;
    case 0b011: high=PH3; low=PH2; flt=PH1; break;
    default:
      floatAll();
      return;
  }

  if(dir < 0){
    int t=high; high=low; low=t;
  }

  drive2(high, low, flt, duty);
}

void Drv8313Hall::kickOpenLoop(int dir, float duty, int ms){
  // 简化版：固定 6 步推进，给 Hall 一个“动起来的机会”
  uint32_t t0 = millis();
  int step=0;

  while((int)(millis()-t0) < ms){
    // 6步：A+B-, A+C-, B+C-, B+A-, C+A-, C+B-
    int A=PH1, B=PH2, C=PH3;
    int high=0, low=0, flt=0;
    switch(step){
      case 0: high=A; low=B; flt=C; break;
      case 1: high=A; low=C; flt=B; break;
      case 2: high=B; low=C; flt=A; break;
      case 3: high=B; low=A; flt=C; break;
      case 4: high=C; low=A; flt=B; break;
      case 5: high=C; low=B; flt=A; break;
    }
    if(dir<0){ int t=high; high=low; low=t; }
    drive2(high, low, flt, duty);

    delayMicroseconds(4500); // 先别太快
    step=(step+1)%6;
  }
  floatAll();
}