#include "ui_led.h" 



void UiLed::begin(Shift595* s){ _s=s; }
void UiLed::setRunning(bool on){ _run=on; }
void UiLed::setValve(bool on){ _valve=on; }
void UiLed::setKvs(bool on){ _kvs=on; }
void UiLed::setTemp(bool on){ _temp=on; }
void UiLed::setPct(bool on){ _pct=on; }
void UiLed::setDigits(int tens, int ones){ _tens=tens; _ones=ones; }
void UiLed::apply(){
  if(!_s) return;
  _s->setAllOff();
  if(_run)   _s->setLed(1, true); // LED1 = RUN
  if(_valve) _s->setLed(2, true); // LED2 = VALVE
  if(_kvs)   _s->setLed(3, true); // LED3 = KVS
  if(_temp)  _s->setLed(4, true); // LED4 = TEMP
  if(_pct)   _s->setLed(5, true); // LED5 = PCT
  _s->setDigit(0, _tens);
  _s->setDigit(1, _ones);
  _s->apply();
}
