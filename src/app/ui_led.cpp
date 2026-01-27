#include "ui_led.h" 



void UiLed::begin(Shift595* s){ _s=s; }
void UiLed::setRunning(bool on){ _run=on; }
void UiLed::apply(){
  if(!_s) return;
  _s->setAllOff();
  if(_run) _s->setLed(1, true); // LED1 = RUN
  _s->apply();
}