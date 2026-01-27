#include <Arduino.h>

#include "../include/config_pins.h"
#include "../include/config_build.h"

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

  // drivers
  drv.begin();
  hall.begin(PIN_HALL_U, PIN_HALL_V, PIN_HALL_W);
  pot.begin(PIN_POT_ADC);
  keys.begin(PIN_KEY_MINUS, PIN_KEY_PLUS, PIN_KEY_OK);

  led.begin(PIN_595_SER, PIN_595_SRCLK, PIN_595_RCLK, PIN_595_OE);

  // app
  ctl.begin(&drv, &hall, &pot);
  ui.begin(&led);

  // net
  web.begin(&ctl);

  // 默认 target=当前角度（上电不乱跑）
  float cur = pot.readDegFiltered();
  ctl.setTarget(cur);

  LOGI("Boot curDeg=%.1f", cur);
}

void loop(){
  // 1) 控制 tick：尽量高频一点（不要 delay 堵塞）
  static uint32_t tCtl=0;
  if(elapsed(tCtl, 2)){ // 2ms
    ctl.tick();
  }

  // 2) LED UI：只显示 RUN（运动中亮）
  static uint32_t tUi=0;
  if(elapsed(tUi, 50)){
    ui.setRunning(ctl.status().running);
    ui.apply();
  }

  // 3) Web
  web.tick();

  // 4) 串口监控（低频）
  static uint32_t tLog=0;
  if(elapsed(tLog, 300)){
    auto s = ctl.status();
    auto hs = hall.read();
    int raw = pot.readRaw();
    LOGI("en=%d run=%d deg=%.1f tgt=%.1f adc=%d hall=%03b fault=%d dirKnown=%d dirSign=%d",
      s.enabled, s.running, s.curDeg, s.targetDeg, raw, hs.code, drv.faulted()?1:0, s.dirKnown?1:0, s.dirSign);
  }
}