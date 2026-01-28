#include <Arduino.h>

#include "config_pins.h"
#include "config_build.h"
#include "config_calib.h"

#include "utils/logger.h"
#include "utils/timebase.h"

#include "drivers/drv8313_hall.h"
#include "drivers/hall.h"
#include "drivers/pot_adc.h"
#include "drivers/shift595.h"
#include "drivers/keys.h"

#include "app/control.h"
#include "app/ui_led.h"

#include "net/ap_web.h"

Drv8313Hall drv;
HallReader   hall;
PotAdc       pot;
Shift595     led;
Keys         keys;

Control      ctl;
UiLed        ui;
ApWeb        web;

void setup(){
  Serial.begin(115200);
  delay(150);

  LOGI("%s %s", FW_NAME, FW_VERSION);

  drv.begin();
  hall.begin(PIN_HALL_U, PIN_HALL_V, PIN_HALL_W);
  pot.begin(PIN_POT_ADC);
  keys.begin(PIN_KEY_MINUS, PIN_KEY_PLUS, PIN_KEY_OK);

  led.begin(PIN_595_SER, PIN_595_SRCLK, PIN_595_RCLK, PIN_595_OE);

  ctl.begin(&drv, &hall, &pot);
  ui.begin(&led);

  web.begin(&ctl);

  float cur = pot.readDegFiltered();
  ctl.setTarget(cur);

  LOGI("Boot curDeg=%.1f", cur);
}

void loop(){
  static uint32_t tCtl=0;
  if(elapsed(tCtl, 2)){
    ctl.tick();
  }

  static bool displayOn = true;
  static bool adjustMode = false;
  static uint32_t tLastAct = 0;
  static uint32_t tPage = 0;
  static bool pageValve = true;
  static int kvsPct = 0;
  static bool keyEvMinus=false, keyEvPlus=false, keyEvOk=false;
  static uint32_t tKeyEv=0;

  uint32_t now = millis();

  if(tLastAct == 0) tLastAct = now;
  if((int)(now - tLastAct) > 60000){
    displayOn = false;
    adjustMode = false;
  }

  static uint32_t tUi=0;
  if(elapsed(tUi, 50)){
    auto s = ctl.status();
    bool runBlink = s.running && ((now / 300) % 2 == 0);

    int displayValue = 0;
    bool ledRun=false, ledValve=false, ledKvs=false, ledTemp=false, ledPct=false;

    if(displayOn){
      if((int)(now - tPage) > 1200){
        tPage = now;
        pageValve = !pageValve;
      }
      bool kvsBlink = adjustMode && ((now / 300) % 2 == 0);
      if(adjustMode){
        displayValue = kvsPct;
      }else if(pageValve){
        displayValue = (int)(s.curDeg + 0.5f);
      }else{
        displayValue = 25;
      }
      if(displayValue < 0) displayValue = 0;
      if(displayValue > 99) displayValue = 99;
      ledRun = runBlink;
      ledValve = adjustMode ? kvsBlink : pageValve;
      ledKvs = adjustMode ? kvsBlink : false;
      ledTemp = (!adjustMode && !pageValve);
      ledPct = adjustMode;

      ui.setRunning(ledRun);
      ui.setValve(ledValve);
      ui.setKvs(ledKvs);
      ui.setTemp(ledTemp);
      ui.setPct(ledPct);
      ui.setDigits(displayValue/10, displayValue%10);
    }else{
      ui.setRunning(false);
      ui.setValve(false);
      ui.setKvs(false);
      ui.setTemp(false);
      ui.setPct(false);
      ui.setDigits(-1, -1);
    }
    ui.apply();

    ctl.setUiState(displayOn, adjustMode, pageValve, kvsPct, displayValue,
                   ledRun, ledValve, ledKvs, ledTemp, ledPct);
  }

  web.tick();

  static uint32_t tLog=0;
  if(elapsed(tLog, 300)){
    auto s = ctl.status();
    auto hs = hall.read();
    int raw = pot.readRaw();
    LOGI("en=%d run=%d deg=%.1f tgt=%.1f adc=%d hall=%u fault=%d dirKnown=%d dirSign=%d spd=%d map=%u",
      s.enabled, s.running, s.curDeg, s.targetDeg, raw, (unsigned)hs.code, drv.faulted()?1:0, s.dirKnown?1:0, s.dirSign, s.speedPct, s.hallMap);
  }

  static uint32_t tKey=0;
  if(elapsed(tKey, 10)){
    keys.update();
    bool evPlus = keys.pressedPlus();
    bool evMinus = keys.pressedMinus();
    bool evOk = keys.pressedOk();
    bool anyKey = evPlus || evMinus || evOk;
    if(anyKey){
      if(!displayOn){
        displayOn = true;
        tLastAct = now;
        return;
      }
      tLastAct = now;
    }

    if(evOk){
      if(!adjustMode){
        adjustMode = true;
        kvsPct = (int)(ctl.status().targetDeg / LOGIC_MAX_DEG * 100.0f + 0.5f);
      }else{
        adjustMode = false;
        float tgt = LOGIC_MAX_DEG * (kvsPct / 100.0f);
        ctl.setTarget(tgt);
        ctl.requestMove(true);
      }
    }

    if(adjustMode){
      if(evPlus){
        kvsPct += 1;
        if(kvsPct > 100) kvsPct = 100;
      }
      if(evMinus){
        kvsPct -= 1;
        if(kvsPct < 0) kvsPct = 0;
      }
    }

    if(anyKey){
      keyEvPlus = evPlus;
      keyEvMinus = evMinus;
      keyEvOk = evOk;
      tKeyEv = now;
    }
  }

  if((int)(now - tKeyEv) > 300){
    keyEvPlus = keyEvMinus = keyEvOk = false;
  }

  ctl.setKeyState(keys.isDownMinus(), keys.isDownPlus(), keys.isDownOk(),
                  keyEvMinus, keyEvPlus, keyEvOk);
}
