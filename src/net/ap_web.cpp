#include "ap_web.h"
#include "web_assets.h"
#include "config_build.h"
#include <WiFi.h>

void ApWeb::begin(Control* ctl){
  _ctl = ctl;

  WiFi.mode(WIFI_AP);
  WiFi.softAP("test111111"); // 无密码
  delay(50);

  _srv.on("/", HTTP_GET, [&](){
    _srv.send(200, "text/html; charset=utf-8", FPSTR(INDEX_HTML));
  });

  _srv.on("/api/enable", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    bool en = _srv.hasArg("en") ? (_srv.arg("en").toInt()!=0) : false;
    _ctl->setEnabled(en);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/target", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    float deg = _srv.hasArg("deg") ? _srv.arg("deg").toFloat() : 0;
    _ctl->setTarget(deg);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/speed", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    int pct = _srv.hasArg("pct") ? _srv.arg("pct").toInt() : 0;
    _ctl->setSpeedPct(pct);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/torque", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    int pct = _srv.hasArg("pct") ? _srv.arg("pct").toInt() : 100;
    _ctl->setTorquePct(pct);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/dir", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    int sign = _srv.hasArg("sign") ? _srv.arg("sign").toInt() : 1;
    _ctl->setDirSign(sign);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/preset", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    String mode = _srv.hasArg("mode") ? _srv.arg("mode") : "";
    if(mode == "ccw"){
      _ctl->setHallMap(0);
      _ctl->setDirSign(-1);
    }else if(mode == "cw"){
      _ctl->setHallMap(3);
      _ctl->setDirSign(+1);
    }
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/hall_map", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    int map = _srv.hasArg("m") ? _srv.arg("m").toInt() : 0;
    _ctl->setHallMap((uint8_t)map);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/auto_scan", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    _ctl->autoScanHall();
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/capture_hall", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    int ms = _srv.hasArg("ms") ? _srv.arg("ms").toInt() : 1500;
    if(ms < 200) ms = 200;
    if(ms > 8000) ms = 8000;
    _ctl->startHallCapture((uint32_t)ms);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/hall_seq", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain",""); return; }
    _srv.send(200,"text/plain", _ctl->hallSeq());
  });

  _srv.on("/api/move", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    bool on = _srv.hasArg("on") ? (_srv.arg("on").toInt()!=0) : false;
    _ctl->requestMove(on);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/stop", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    _ctl->requestMove(false);
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/detect_dir", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"text/plain","no ctl"); return; }
    _ctl->detectDirection();
    _srv.send(200,"text/plain","ok");
  });

  _srv.on("/api/status", HTTP_GET, [&](){
    if(!_ctl){ _srv.send(500,"application/json","{}"); return; }
    ControlStatus s = _ctl->status();
    String json = "{";
    json += "\"name\":\"" + String(FW_NAME) + "\",";
    json += "\"version\":\"" + String(FW_VERSION) + "\",";
    json += "\"enabled\":" + String(s.enabled?1:0) + ",";
    json += "\"running\":" + String(s.running?1:0) + ",";
    json += "\"targetDeg\":" + String(s.targetDeg,1) + ",";
    json += "\"curDeg\":" + String(s.curDeg,1) + ",";
    json += "\"hall\":" + String((int)s.hall) + ",";
    json += "\"dirKnown\":" + String(s.dirKnown?1:0) + ",";
    json += "\"dirSign\":" + String(s.dirSign) + ",";
    json += "\"speedPct\":" + String(s.speedPct) + ",";
    json += "\"torquePct\":" + String(s.torquePct) + ",";
    json += "\"hallMap\":" + String(s.hallMap) + ",";
    json += "\"hallSeq\":\"" + String(_ctl->hallSeq()) + "\",";
    json += "\"displayOn\":" + String(s.displayOn?1:0) + ",";
    json += "\"adjustMode\":" + String(s.adjustMode?1:0) + ",";
    json += "\"pageValve\":" + String(s.pageValve?1:0) + ",";
    json += "\"kvsPct\":" + String(s.kvsPct) + ",";
    json += "\"displayValue\":" + String(s.displayValue) + ",";
    json += "\"ledRun\":" + String(s.ledRun?1:0) + ",";
    json += "\"ledValve\":" + String(s.ledValve?1:0) + ",";
    json += "\"ledKvs\":" + String(s.ledKvs?1:0) + ",";
    json += "\"ledTemp\":" + String(s.ledTemp?1:0) + ",";
    json += "\"ledPct\":" + String(s.ledPct?1:0) + ",";
    json += "\"keyDownMinus\":" + String(s.keyDownMinus?1:0) + ",";
    json += "\"keyDownPlus\":" + String(s.keyDownPlus?1:0) + ",";
    json += "\"keyDownOk\":" + String(s.keyDownOk?1:0) + ",";
    json += "\"keyPressMinus\":" + String(s.keyPressMinus?1:0) + ",";
    json += "\"keyPressPlus\":" + String(s.keyPressPlus?1:0) + ",";
    json += "\"keyPressOk\":" + String(s.keyPressOk?1:0);
    json += "}";
    _srv.send(200, "application/json", json);
  });

  _srv.begin();
}

void ApWeb::tick(){
  _srv.handleClient();
}
