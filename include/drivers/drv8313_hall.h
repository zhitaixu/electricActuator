#pragma once
#include <Arduino.h>

class Drv8313Hall {
public:
  static const int PWM_BITS = 10;
  static const int PWM_MAX  = (1<<PWM_BITS)-1;
  void begin();
  void enable(bool en);
  bool faulted() const;
  void setHallMap(uint8_t map);
  uint8_t hallMap() const { return _hallMap; }
  uint8_t remapHall(uint8_t code) const;

  void driveHall(uint8_t hallCode, int dir, float duty);
  void kickOpenLoop(int dir, float duty, int ms);

private:
  void setPWM(float duty1, float duty2, float duty3);
  void floatAll();
  void drive2(int highPh, int lowPh, int floatPh, float duty);

  static const int PH1=1, PH2=2, PH3=3;
  static const int PWM_FREQ = 20000;
  static const int CH1=0, CH2=1, CH3=2;

  bool _enabled=false;
  uint8_t _hallMap=0;
};
