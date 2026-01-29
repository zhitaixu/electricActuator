#pragma once
#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="zh-CN">
<head>
<meta charset="utf-8"/>
<meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Electric Actuator</title>
<style>
  :root{ --bg:#0b1220; --card:#101a33; --text:#e9eef9; --muted:#9db0d6; --acc:#5b8cff; --ok:#2ed188; --bad:#ff5b6e; --line:rgba(255,255,255,.07); }
  body{ margin:0; font-family:system-ui,-apple-system,Segoe UI,Roboto; background:linear-gradient(180deg,#0a1122,#0b1220); color:var(--text); }
  .wrap{ max-width:980px; margin:0 auto; padding:20px; }
  .row{ display:grid; grid-template-columns:1fr; gap:16px; }
  @media (min-width:900px){ .row{ grid-template-columns:1.2fr 1fr; } }
  .card{ background:rgba(16,26,51,.85); border:1px solid var(--line); border-radius:18px; padding:16px; box-shadow:0 20px 48px rgba(0,0,0,.28); }
  h1{ font-size:19px; margin:0 0 12px; color:#fff; }
  .kv{ display:grid; grid-template-columns:150px 1fr; gap:8px 12px; color:var(--muted); font-size:13px; }
  .kv b{ color:var(--text); font-weight:600; }
  .btns{ display:flex; flex-wrap:wrap; gap:10px; margin-top:12px; }
  button{ border:1px solid var(--line); padding:10px 14px; border-radius:12px; background:rgba(255,255,255,.06); color:var(--text); cursor:pointer; }
  button.primary{ background:var(--acc); color:#07122a; font-weight:700; }
  button.good{ background:var(--ok); color:#062016; font-weight:700; }
  button.bad{ background:var(--bad); color:#2a070c; font-weight:700; }
  .field{ display:flex; align-items:center; gap:10px; margin:8px 0; flex-wrap:wrap; }
  input[type=range]{ width:220px; }
  input[type=number]{ width:110px; padding:8px 10px; border-radius:10px; border:1px solid var(--line); background:#0d162c; color:var(--text); }
  .tag{ display:inline-block; padding:4px 8px; border-radius:999px; background:rgba(255,255,255,.08); color:var(--muted); font-size:12px; }
</style>
</head>
<body>
<div class="wrap">
  <div class="row">
    <div class="card">
      <h1>Electric Actuator <span class="tag" id="ver">--</span> <span class="tag">OTA TEST</span></h1>
      <div class="kv">
        <span>Enabled</span><b id="en">--</b>
        <span>Running</span><b id="run">--</b>
        <span>Angle</span><b id="deg">--</b>
        <span>Target</span><b id="tdeg">--</b>
        <span>Hall</span><b id="hall">--</b>
        <span>DirKnown</span><b id="dirk">--</b>
        <span>DirSign</span><b id="dirs">--</b>
        <span>Speed</span><b id="spdval">--</b>
        <span>HallMap</span><b id="hmap">--</b>
        <span>HallSeq</span><b id="hseq">--</b>
        <span>Display</span><b id="disp">--</b>
        <span>Mode</span><b id="mode">--</b>
        <span>Page</span><b id="page">--</b>
        <span>Value</span><b id="dval2">--</b>
      </div>
      <div class="btns">
        <button class="good" onclick="api('/api/enable?en=1')">Enable</button>
        <button class="bad" onclick="api('/api/enable?en=0')">Disable</button>
        <button onclick="api('/api/detect_dir')">Detect Dir</button>
        <button onclick="api('/api/auto_scan')">Auto Scan</button>
        <button onclick="api('/api/capture_hall?ms=2000')">Capture Hall</button>
        <button onclick="api('/api/preset?mode=ccw')">Preset CCW</button>
        <button onclick="api('/api/preset?mode=cw')">Preset CW</button>
      </div>
      <div class="field">
        <span class="tag">Hall Map</span>
        <input id="mapInput" type="range" min="0" max="5" value="0" step="1" oninput="mval.innerText=this.value" onchange="setMap()"/>
        <b id="mval">0</b>
      </div>
    </div>

    <div class="card">
      <h1>Move</h1>
      <div class="field">
        <span class="tag">Target deg</span>
        <input id="degInput" type="range" min="0" max="90" value="0" step="1" oninput="dval.innerText=this.value" onchange="setTarget()"/>
        <b id="dval">0</b>
      </div>
      <div class="field">
        <span class="tag">Speed %</span>
        <input id="spdInput" type="range" min="0" max="100" value="100" step="1" oninput="sval.innerText=this.value" onchange="setSpeed()"/>
        <b id="sval">100</b>
      </div>
      <div class="field">
        <span class="tag">Torque %</span>
        <input id="torInput" type="range" min="10" max="300" value="100" step="1" oninput="tval.innerText=this.value" onchange="setTorque()"/>
        <b id="tval">100</b>
      </div>
      <div class="btns">
        <button class="primary" onclick="move()">Move</button>
        <button onclick="api('/api/stop')">Stop</button>
      </div>
      <div style="color:var(--muted);font-size:12px;line-height:1.5;margin-top:8px;">
        流程：Enable → Capture Hall（手转顺时针 1 圈）→ 可选 Auto Scan → 设置 Target/Speed → Move
      </div>
    </div>

    <div class="card">
      <h1>LED / Keys</h1>
      <div class="btns">
        <button onclick="api('/api/key?minus=1')">Key -</button>
        <button onclick="api('/api/key?plus=1')">Key +</button>
        <button onclick="api('/api/key?ok=1')">Key OK</button>
      </div>
      <div class="kv">
        <span>LED RUN</span><b id="lrun">--</b>
        <span>LED VALVE</span><b id="lval">--</b>
        <span>LED KVS</span><b id="lkvs">--</b>
        <span>LED TEMP</span><b id="ltemp">--</b>
        <span>LED PCT</span><b id="lpct">--</b>
        <span>Key -</span><b id="kminus">--</b>
        <span>Key +</span><b id="kplus">--</b>
        <span>Key OK</span><b id="kok">--</b>
        <span>Press -</span><b id="pminus">--</b>
        <span>Press +</span><b id="pplus">--</b>
        <span>Press OK</span><b id="pok">--</b>
      </div>
    </div>

    <div class="card">
      <h1>OTA Upgrade</h1>
      <div style="color:var(--muted);font-size:12px;line-height:1.5;margin-bottom:8px;">
        选择固件 bin 上传，完成后设备自动重启。
      </div>
      <form id="otaForm" method="POST" action="/api/ota" enctype="multipart/form-data">
        <input type="file" name="update" accept=".bin"/>
        <button class="primary" type="submit">Upload</button>
      </form>
    </div>
  </div>
</div>

<script>
async function api(path){
  try{ await fetch(path); }catch(e){}
}
async function setTarget(){
  const t = document.getElementById('degInput').value;
  await fetch('/api/target?deg='+t);
}
async function setSpeed(){
  const s = document.getElementById('spdInput').value;
  await fetch('/api/speed?pct='+s);
}
async function setMap(){
  const m = document.getElementById('mapInput').value;
  await fetch('/api/hall_map?m='+m);
}
async function setTorque(){
  const t = document.getElementById('torInput').value;
  await fetch('/api/torque?pct='+t);
}
async function move(){
  const t = document.getElementById('degInput').value;
  const s = document.getElementById('spdInput').value;
  await fetch('/api/target?deg='+t);
  await fetch('/api/speed?pct='+s);
  await fetch('/api/move?on=1');
}
async function poll(){
  try{
    const j = await (await fetch('/api/status')).json();
    ver.innerText = j.version;
    en.innerText = j.enabled? 'YES':'NO';
    run.innerText = j.running? 'YES':'NO';
    deg.innerText = j.curDeg.toFixed(1);
    tdeg.innerText = j.targetDeg.toFixed(1);
    hall.innerText = j.hall;
    dirk.innerText = j.dirKnown? 'YES':'NO';
    dirs.innerText = j.dirSign;
    spdval.innerText = j.speedPct;
    hmap.innerText = j.hallMap;
    hseq.innerText = j.hallSeq || '--';
    disp.innerText = j.displayOn ? 'ON' : 'OFF';
    mode.innerText = j.adjustMode ? 'KVS' : 'NORMAL';
    page.innerText = j.pageValve ? 'VALVE' : 'TEMP';
    dval2.innerText = j.displayValue;
    lrun.innerText = j.ledRun ? 'ON' : 'OFF';
    lval.innerText = j.ledValve ? 'ON' : 'OFF';
    lkvs.innerText = j.ledKvs ? 'ON' : 'OFF';
    ltemp.innerText = j.ledTemp ? 'ON' : 'OFF';
    lpct.innerText = j.ledPct ? 'ON' : 'OFF';
    kminus.innerText = j.keyDownMinus ? 'DOWN' : 'UP';
    kplus.innerText = j.keyDownPlus ? 'DOWN' : 'UP';
    kok.innerText = j.keyDownOk ? 'DOWN' : 'UP';
    pminus.innerText = j.keyPressMinus ? 'YES' : 'NO';
    pplus.innerText = j.keyPressPlus ? 'YES' : 'NO';
    pok.innerText = j.keyPressOk ? 'YES' : 'NO';
    if(document.activeElement !== degInput){
      degInput.value = Math.round(j.targetDeg);
      dval.innerText = degInput.value;
    }
    if(document.activeElement !== spdInput){
      spdInput.value = j.speedPct;
      sval.innerText = spdInput.value;
    }
    if(document.activeElement !== torInput){
      torInput.value = j.torquePct || 100;
      tval.innerText = torInput.value;
    }
    if(document.activeElement !== mapInput){
      mapInput.value = j.hallMap;
      mval.innerText = mapInput.value;
    }
  }catch(e){}
}
setInterval(poll, 250);
poll();
</script>
</body>
</html>
)HTML";
