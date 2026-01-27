#pragma once
#include <Arduino.h>

static inline uint32_t ms() { return millis(); }
static inline bool elapsed(uint32_t &t, uint32_t period_ms){
  uint32_t now = ms();
  if ((uint32_t)(now - t) >= period_ms) { t = now; return true; }
  return false;
}