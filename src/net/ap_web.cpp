#include "ap_web.h"
#include "web_assets.h"
#include "../../include/config_build.h"
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
    json += "\"dirSign\":" + String(s.dirSign);
    json += "}";
    _srv.send(200, "application/json", json);
  });

  _srv.begin();
}

void ApWeb::tick(){
  _srv.handleClient();
}