#include "drivers/drv8313_hall.h"
#include "config_pins.h"
#include "config_calib.h"

static inline int dutyToRaw(float duty){
  if(duty<0) duty=0;
  if(duty>1) duty=1;
  return (int)(duty * (float)Drv8313Hall::PWM_MAX + 0.5f);
}

void Drv8313Hall::begin(){
  pinMode(PIN_IN1, OUTPUT); digitalWrite(PIN_IN1, LOW);
  pinMode(PIN_IN2, OUTPUT); digitalWrite(PIN_IN2, LOW);
  pinMode(PIN_IN3, OUTPUT); digitalWrite(PIN_IN3, LOW);

  pinMode(PIN_EN1, OUTPUT); digitalWrite(PIN_EN1, LOW);
  pinMode(PIN_EN2, OUTPUT); digitalWrite(PIN_EN2, LOW);
  pinMode(PIN_EN3, OUTPUT); digitalWrite(PIN_EN3, LOW);

  pinMode(PIN_NSLEEP, OUTPUT); digitalWrite(PIN_NSLEEP, LOW);
  pinMode(PIN_NRESET, OUTPUT); digitalWrite(PIN_NRESET, LOW);
  pinMode(PIN_NFAULT, INPUT);

  ledcSetup(CH1, PWM_FREQ, PWM_BITS);
  ledcSetup(CH2, PWM_FREQ, PWM_BITS);
  ledcSetup(CH3, PWM_FREQ, PWM_BITS);
  ledcAttachPin(PIN_IN1, CH1);
  ledcAttachPin(PIN_IN2, CH2);
  ledcAttachPin(PIN_IN3, CH3);
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

void Drv8313Hall::setHallMap(uint8_t map){
  _hallMap = (uint8_t)(map % 6);
}

uint8_t Drv8313Hall::remapHall(uint8_t code) const {
  uint8_t u = (code >> 2) & 1;
  uint8_t v = (code >> 1) & 1;
  uint8_t w = (code >> 0) & 1;

  uint8_t ru=u, rv=v, rw=w;
  switch(_hallMap){
    case 0: ru=u; rv=v; rw=w; break; // U V W
    case 1: ru=u; rv=w; rw=v; break; // U W V
    case 2: ru=v; rv=u; rw=w; break; // V U W
    case 3: ru=v; rv=w; rw=u; break; // V W U
    case 4: ru=w; rv=u; rw=v; break; // W U V
    case 5: ru=w; rv=v; rw=u; break; // W V U
  }
  return (ru<<2) | (rv<<1) | (rw);
}

void Drv8313Hall::setPWM(float duty1, float duty2, float duty3){
  ledcWrite(CH1, dutyToRaw(duty1));
  ledcWrite(CH2, dutyToRaw(duty2));
  ledcWrite(CH3, dutyToRaw(duty3));
}

void Drv8313Hall::floatAll(){
  setPWM(0,0,0);
  digitalWrite(PIN_EN1, LOW);
  digitalWrite(PIN_EN2, LOW);
  digitalWrite(PIN_EN3, LOW);
}

// DRV8313: IN=PWM (high-side), IN=0 (low-side), EN=enable
void Drv8313Hall::drive2(int highPh, int lowPh, int floatPh, float duty){
  if(floatPh==1){ digitalWrite(PIN_EN1, LOW); ledcWrite(CH1, 0); }
  if(floatPh==2){ digitalWrite(PIN_EN2, LOW); ledcWrite(CH2, 0); }
  if(floatPh==3){ digitalWrite(PIN_EN3, LOW); ledcWrite(CH3, 0); }

  if(lowPh==1){ digitalWrite(PIN_EN1, HIGH); ledcWrite(CH1, 0); }
  if(lowPh==2){ digitalWrite(PIN_EN2, HIGH); ledcWrite(CH2, 0); }
  if(lowPh==3){ digitalWrite(PIN_EN3, HIGH); ledcWrite(CH3, 0); }

  int raw = dutyToRaw(duty);
  if(highPh==1){ digitalWrite(PIN_EN1, HIGH); ledcWrite(CH1, raw); }
  if(highPh==2){ digitalWrite(PIN_EN2, HIGH); ledcWrite(CH2, raw); }
  if(highPh==3){ digitalWrite(PIN_EN3, HIGH); ledcWrite(CH3, raw); }
}

// Hall -> commutation map (120deg 6-step)
void Drv8313Hall::driveHall(uint8_t h, int dir, float duty){
  if(!_enabled) return;
  if(duty <= 0.0f){
    floatAll();
    return;
  }

  int high=0, low=0, flt=0;
  int PHU=PH1, PHV=PH2, PHW=PH3;

  uint8_t hh = remapHall(h);

  // 001: U+ V- (W float)
  // 101: U+ W-
  // 100: V+ W-
  // 110: V+ U-
  // 010: W+ U-
  // 011: W+ V-
  switch(hh){
    case 0b001: high=PHU; low=PHV; flt=PHW; break;
    case 0b101: high=PHU; low=PHW; flt=PHV; break;
    case 0b100: high=PHV; low=PHW; flt=PHU; break;
    case 0b110: high=PHV; low=PHU; flt=PHW; break;
    case 0b010: high=PHW; low=PHU; flt=PHV; break;
    case 0b011: high=PHW; low=PHV; flt=PHU; break;
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
  uint32_t t0 = millis();
  int step=0;

  while((int)(millis()-t0) < ms){
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

    delayMicroseconds(4500);
    step=(step+1)%6;
  }
  floatAll();
}
