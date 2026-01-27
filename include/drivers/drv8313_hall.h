#pragma once
#include <Arduino.h>

class Drv8313Hall {
public:
  static const int PWM_BITS = 10;
  static const int PWM_MAX  = (1<<PWM_BITS)-1;
  void begin();
  void enable(bool en);
  bool faulted() const;

  // 设置方向（dir=+1 或 -1），占空比(0..1)
  void driveHall(uint8_t hallCode, int dir, float duty);

  // 开环 kick：6步序列推进
  void kickOpenLoop(int dir, float duty, int ms);

private:
  void setPWM(float duty1, float duty2, float duty3);
  void floatAll();
  void drive2(int highPh, int lowPh, int floatPh, float duty);

  // 相序已验证：phase_map=1-2-3，invert=0
  static const int PH1=1, PH2=2, PH3=3;

  // PWM
  static const int PWM_FREQ = 20000;
  static const int CH1=0, CH2=1, CH3=2;

  bool _enabled=false;
};