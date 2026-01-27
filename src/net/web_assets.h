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
  :root{ --bg:#0b1220; --card:#0f1a33; --text:#e6edf7; --muted:#9fb0d0; --acc:#4f8cff; --ok:#28d17c; --bad:#ff5b6e; }
  body{ margin:0; font-family:system-ui,-apple-system,Segoe UI,Roboto; background:linear-gradient(180deg,#070c16,#0b1220); color:var(--text); }
  .wrap{ max-width:900px; margin:0 auto; padding:18px; }
  .row{ display:grid; grid-template-columns:1fr; gap:14px; }
  .card{ background:rgba(15,26,51,.75); border:1px solid rgba(255,255,255,.06); border-radius:16px; padding:14px; box-shadow:0 16px 40px rgba(0,0,0,.25); }
  h1{ font-size:18px; margin:0 0 10px; color:#fff; }
  .kv{ display:grid; grid-template-columns:140px 1fr; gap:8px; color:var(--muted); font-size:13px; }
  .kv b{ color:var(--text); font-weight:600; }
  .btns{ display:flex; flex-wrap:wrap; gap:10px; margin-top:10px; }
  button{ border:0; padding:10px 12px; border-radius:12px; background:rgba(255,255,255,.08); color:var(--text); cursor:pointer; }
  button.primary{ background:var(--acc); color:#031027; font-weight:700; }
  button.good{ background:var(--ok); color:#031027; font-weight:700; }
  button.bad{ background:var(--bad); color:#2a070c; font-weight:700; }
  .range{ display:flex; align-items:center; gap:12px; }
  input[type=range]{ width:100%; }
  .tag{ display:inline-block; padding:4px 8px; border-radius:999px; background:rgba(255,255,255,.08); color:var(--muted); font-size:12px; }
</style>
</head>
<body>
<div class="wrap">
  <div class="row">
    <div class="card">
      <h1>Electric Actuator <span class="tag" id="ver">--</span></h1>
      <div class="kv">
        <span>Enabled</span><b id="en">--</b>
        <span>Running</span><b id="run">--</b>
        <span>Angle</span><b id="deg">--</b>
        <span>Target</span><b id="tdeg">--</b>
        <span>Hall</span><b id="hall">--</b>
        <span>DirKnown</span><b id="dirk">--</b>
        <span>DirSign</span><b id="dirs">--</b>
      </div>
      <div class="btns">
        <button class="good" onclick="api('/api/enable?en=1')">Enable</button>
        <button class="bad" onclick="api('/api/enable?en=0')">Disable</button>
        <button onclick="api('/api/detect_dir')">Detect Dir</button>
      </div>
    </div>

    <div class="card">
      <h1>Move</h1>
      <div class="range">
        <input id="r" type="range" min="0" max="90" value="0" step="1" oninput="v.innerText=this.value"/>
        <b id="v">0</b><span class="tag">deg</span>
      </div>
      <div class="btns">
        <button class="primary" onclick="move()">Move</button>
        <button onclick="api('/api/stop')">Stop</button>
      </div>
      <div style="color:var(--muted);font-size:12px;line-height:1.5;margin-top:8px;">
        使用流程：Enable → Detect Dir（只需做一次）→ 调角度 → Move
      </div>
    </div>
  </div>
</div>

<script>
async function api(path){
  try{ await fetch(path); }catch(e){}
}
async function move(){
  const t = document.getElementById('r').value;
  await fetch('/api/target?deg='+t);
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
    r.value = Math.round(j.targetDeg);
    v.innerText = r.value;
  }catch(e){}
}
setInterval(poll, 250);
poll();
</script>
</body>
</html>
)HTML";