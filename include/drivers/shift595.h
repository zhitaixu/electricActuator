#pragma once
#include <Arduino.h>

class Shift595 {
public:
  void begin(int ser, int srclk, int rclk, int oe);
  void setLed(int ledIndex1to19, bool on); // LED1..LED19
  void apply();

  void setAllOff();
private:
  int _ser=-1,_srclk=-1,_rclk=-1,_oe=-1;
  uint8_t _b[3] = {0,0,0}; // 595#1, #2, #3
};