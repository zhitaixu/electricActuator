#include "drivers/pot_adc.h"
#include "config_calib.h"

void PotAdc::begin(int pinAdc){
  _pin = pinAdc;
  analogReadResolution(12); // 0..4095
  _f = (float)analogRead(_pin);
  _inited = true;
}

int PotAdc::readRaw(){
  return analogRead(_pin);
}

float PotAdc::rawToDeg(int raw) const {
  // raw: 3900 -> 0deg, 2630 -> 93deg （raw 下降代表角度增大）
  float t = (float)(ADC_AT_0DEG - raw) / (float)(ADC_AT_0DEG - ADC_AT_93DEG); // 0..1
  if (t < 0) t = 0;
  if (t > 1) t = 1;

  float phys = t * PHYS_MAX_DEG;          // 0..93
  float logic = phys * (LOGIC_MAX_DEG / PHYS_MAX_DEG); // 缩到 0..90
  return logic;
}

float PotAdc::readDegFiltered(){
  int r = readRaw();
  if(!_inited){ _f = (float)r; _inited=true; }
  // 简单 IIR：越大越稳、越小越灵敏
  const float alpha = 0.15f;
  _f = _f + alpha * ((float)r - _f);
  return rawToDeg((int)(_f + 0.5f));
}
