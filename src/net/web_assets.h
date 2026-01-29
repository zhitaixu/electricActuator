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
  :root{ --bg:#0b1220; --card:#101a33; --text:#e9eef9; --muted:#9db0d6; --acc:#5b8cff; --ok:#2ed188; --bad:#ff5b6e; --line:rgba(255,255,255,.07); --segOff:#1a233f; --segOn:#ffcc66; }
  body{ margin:0; font-family:system-ui,-apple-system,Segoe UI,Roboto; background:linear-gradient(180deg,#0a1122,#0b1220); color:var(--text); }
  .wrap{ max-width:1100px; margin:0 auto; padding:18px; }
  .row{ display:grid; grid-template-columns:1fr; gap:14px; }
  @media (min-width:900px){ .row{ grid-template-columns:1.2fr 1fr; } }
  .card{ background:rgba(16,26,51,.85); border:1px solid var(--line); border-radius:18px; padding:16px; box-shadow:0 20px 48px rgba(0,0,0,.28); }
  h1{ font-size:18px; margin:0 0 10px; color:#fff; }
  .kv{ display:grid; grid-template-columns:160px 1fr; gap:8px 12px; color:var(--muted); font-size:13px; }
  .kv b{ color:var(--text); font-weight:600; }
  .btns{ display:flex; flex-wrap:wrap; gap:10px; margin-top:10px; }
  button{ border:1px solid var(--line); padding:9px 12px; border-radius:10px; background:rgba(255,255,255,.06); color:var(--text); cursor:pointer; }
  button.primary{ background:var(--acc); color:#07122a; font-weight:700; }
  button.good{ background:var(--ok); color:#062016; font-weight:700; }
  button.bad{ background:var(--bad); color:#2a070c; font-weight:700; }
  .field{ display:flex; align-items:center; gap:10px; margin:8px 0; flex-wrap:wrap; }
  input[type=range]{ width:260px; }
  .tag{ display:inline-block; padding:4px 8px; border-radius:999px; background:rgba(255,255,255,.08); color:var(--muted); font-size:12px; }

  .leds{ display:flex; gap:8px; margin-top:8px; }
  .led-dot{ width:14px; height:14px; border-radius:50%; background:#1a233f; border:1px solid var(--line); }
  .led-dot.on{ background:#45f1a4; }

  .digits{ display:flex; gap:10px; margin-top:8px; }
  .digit{ position:relative; width:36px; height:60px; }
  .seg{ position:absolute; background:var(--segOff); border-radius:3px; }
  .seg.on{ background:var(--segOn); box-shadow:0 0 8px rgba(255,204,102,.6); }
  .seg.a{ top:2px; left:6px; width:24px; height:4px; }
  .seg.b{ top:6px; right:2px; width:4px; height:20px; }
  .seg.c{ bottom:6px; right:2px; width:4px; height:20px; }
  .seg.d{ bottom:2px; left:6px; width:24px; height:4px; }
  .seg.e{ bottom:6px; left:2px; width:4px; height:20px; }
  .seg.f{ top:6px; left:2px; width:4px; height:20px; }
  .seg.g{ top:28px; left:6px; width:24px; height:4px; }
</style>
</head>
<body>
<div class="wrap">
  <div class="row">
    <div class="card">
      <h1>运行控制 <span class="tag" id="ver">--</span></h1>
      <div class="kv">
        <span>Enabled</span><b id="en">--</b>
        <span>Running</span><b id="run">--</b>
        <span>Angle</span><b id="deg">--</b>
        <span>Target</span><b id="tdeg">--</b>
        <span>Hall</span><b id="hall">--</b>
        <span>DirSign</span><b id="dirs">--</b>
        <span>Speed</span><b id="spdval">--</b>
        <span>Torque</span><b id="torval">--</b>
        <span>HallMap</span><b id="hmap">--</b>
      </div>
      <div class="btns">
        <button class="good" onclick="api('/api/enable?en=1')">Enable</button>
        <button class="bad" onclick="api('/api/enable?en=0')">Disable</button>
      </div>
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
    </div>

    <div class="card">
      <h1>LED / 按键 / 段码</h1>
      <div class="btns">
        <button onclick="api('/api/key?minus=1')">Key -</button>
        <button onclick="api('/api/key?plus=1')">Key +</button>
        <button onclick="api('/api/key?ok=1')">Key OK</button>
      </div>
      <div class="kv">
        <span>Display</span><b id="disp">--</b>
        <span>Mode</span><b id="mode">--</b>
        <span>Page</span><b id="page">--</b>
        <span>Value</span><b id="dval2">--</b>
      </div>
      <div class="leds">
        <span class="led-dot" id="lrun"></span>
        <span class="led-dot" id="lval"></span>
        <span class="led-dot" id="lkvs"></span>
        <span class="led-dot" id="ltemp"></span>
        <span class="led-dot" id="lpct"></span>
      </div>
      <div class="digits">
        <div class="digit" id="dig0">
          <span class="seg a"></span><span class="seg b"></span><span class="seg c"></span>
          <span class="seg d"></span><span class="seg e"></span><span class="seg f"></span><span class="seg g"></span>
        </div>
        <div class="digit" id="dig1">
          <span class="seg a"></span><span class="seg b"></span><span class="seg c"></span>
          <span class="seg d"></span><span class="seg e"></span><span class="seg f"></span><span class="seg g"></span>
        </div>
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
async function api(path){ try{ await fetch(path); }catch(e){} }
async function setTarget(){ await fetch('/api/target?deg='+degInput.value); }
async function setSpeed(){ await fetch('/api/speed?pct='+spdInput.value); }
async function setTorque(){ await fetch('/api/torque?pct='+torInput.value); }
async function move(){
  await fetch('/api/target?deg='+degInput.value);
  await fetch('/api/speed?pct='+spdInput.value);
  await fetch('/api/move?on=1');
}
function setDigit(el, val){
  const map = [
    [1,1,1,1,1,1,0], [0,1,1,0,0,0,0], [1,1,0,1,1,0,1],
    [1,1,1,1,0,0,1], [0,1,1,0,0,1,1], [1,0,1,1,0,1,1],
    [1,0,1,1,1,1,1], [1,1,1,0,0,0,0], [1,1,1,1,1,1,1],
    [1,1,1,1,0,1,1]
  ];
  const segs = el.querySelectorAll('.seg');
  segs.forEach((s,i)=>{ s.classList.toggle('on', val>=0 && val<=9 ? map[val][i] : false); });
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
    dirs.innerText = j.dirSign;
    spdval.innerText = j.speedPct;
    torval.innerText = j.torquePct;
    hmap.innerText = j.hallMap;
    disp.innerText = j.displayOn ? 'ON' : 'OFF';
    mode.innerText = j.adjustMode ? 'KVS' : 'NORMAL';
    page.innerText = j.pageValve ? 'VALVE' : 'TEMP';
    dval2.innerText = j.displayValue;
    lrun.classList.toggle('on', j.ledRun);
    lval.classList.toggle('on', j.ledValve);
    lkvs.classList.toggle('on', j.ledKvs);
    ltemp.classList.toggle('on', j.ledTemp);
    lpct.classList.toggle('on', j.ledPct);
    setDigit(dig0, Math.floor(j.displayValue/10));
    setDigit(dig1, j.displayValue%10);
    if(document.activeElement !== degInput){
      degInput.value = Math.round(j.targetDeg); dval.innerText = degInput.value;
    }
    if(document.activeElement !== spdInput){
      spdInput.value = j.speedPct; sval.innerText = spdInput.value;
    }
    if(document.activeElement !== torInput){
      torInput.value = j.torquePct || 100; tval.innerText = torInput.value;
    }
  }catch(e){}
}
setInterval(poll, 250);
poll();
</script>
</body>
</html>
)HTML";
