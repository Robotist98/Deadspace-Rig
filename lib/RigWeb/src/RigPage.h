#pragma once

#include <pgmspace.h>

static const char RIG_PAGE[] PROGMEM = R"RIGPAGE(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="theme-color" content="#091519">
<title>RIG Control</title>
<style>
:root{color-scheme:dark;--bg:#091519;--panel:#102126;--line:#284047;--text:#e6f4f4;--muted:#91afb5;--cyan:#68f1de;--blue:#69b7ff;--red:#ff7c76}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at 10% 0,#123b41,transparent 60%),var(--bg);color:var(--text);font:15px/1.5 system-ui,sans-serif;min-height:100vh}
main{max-width:1050px;margin:auto;padding:36px 24px 24px}header{display:flex;justify-content:space-between;align-items:center;gap:16px;margin-bottom:28px}.eyebrow{text-transform:uppercase;letter-spacing:.22em;font-size:11px;color:var(--muted);margin:0 0 6px}h1{font-size:clamp(28px,5vw,40px);line-height:1.1;letter-spacing:.06em;margin:0;font-weight:650}h2{font-size:17px;margin:0}p{margin:6px 0 0;color:var(--muted)}.connection{font-size:12px;white-space:nowrap;color:var(--muted);border:1px solid var(--line);border-radius:20px;padding:7px 12px}.connection.online{color:var(--cyan)}.connection:before{content:'\25cf';margin-right:7px}.layout{display:grid;grid-template-columns:300px 1fr;gap:20px}.card{background:linear-gradient(140deg,#142b30dd,var(--panel));border:1px solid var(--line);border-radius:14px;padding:23px}.stack{display:grid;gap:20px}.card-head{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:22px}.badge{font-size:11px;padding:4px 9px;border:1px solid #38716e;border-radius:5px;color:var(--cyan);text-transform:uppercase;letter-spacing:.07em}.rig-preview{display:flex;flex-direction:column;align-items:center;gap:20px;padding:15px 0 20px}.status-leds{display:flex;gap:5px;border:1px solid var(--line);padding:10px;border-radius:10px}.status-leds i{height:12px;width:24px;background:#1a3543;border-radius:3px}.spine{display:flex;flex-direction:column-reverse;gap:4px;padding:13px 20px;border:1px solid var(--line);background:#071215;border-radius:18px}.spine i{display:block;height:11px;width:68px;background:#1a3337;border-radius:3px;transform:perspective(50px) rotateX(7deg)}.preview-label{font-size:10px;letter-spacing:.13em;text-transform:uppercase;color:var(--muted);text-align:center;margin-top:8px}.legend{border-top:1px solid var(--line);padding-top:15px;display:flex;justify-content:space-between;color:var(--muted);font-size:12px}.field+.field{margin-top:23px}.field-head{display:flex;align-items:center;justify-content:space-between;margin-bottom:7px}label{font-size:14px}output{font-variant-numeric:tabular-nums;color:var(--cyan);font-size:14px}input[type=range]{width:100%;height:28px;margin:0;accent-color:var(--cyan);cursor:pointer}#stasis{accent-color:var(--blue)}.buttons{display:grid;grid-template-columns:repeat(3,1fr);gap:9px}button{font:inherit;font-size:13px;min-height:44px;cursor:pointer;border-radius:7px;padding:9px 11px;background:#1a3238;color:var(--text);border:1px solid #36555c;transition:background .15s,transform .15s}button:hover{background:#26464b}button:active{transform:translateY(1px)}button:focus-visible,input:focus-visible{outline:2px solid var(--cyan);outline-offset:3px}button.danger{color:#ffc0bc;border-color:#754747}button.primary{background:#174c48;border-color:#408d81;color:#c3fff1}button:disabled{opacity:.5;cursor:wait}.stasis-actions{display:flex;gap:9px;margin-top:17px}.stasis-actions button{flex:1}.demo-row{display:flex;align-items:center;justify-content:space-between;gap:18px;margin-top:22px;padding-top:20px;border-top:1px solid var(--line)}.demo-row p{font-size:12px}.demo-row button{min-width:108px}.note{font-size:12px;line-height:1.6;margin-top:18px}.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%);background:#203f45;border:1px solid #4a7277;border-radius:8px;padding:12px 18px;max-width:90vw;color:#fff;z-index:2}.toast[hidden]{display:none}footer{margin-top:20px;color:var(--muted);font-size:11px;letter-spacing:.08em;display:flex;justify-content:space-between}.critical .spine{animation:heartbeat 1.4s infinite}.malfunction .rig-preview{animation:flicker .3s steps(2) infinite}.off .rig-preview{opacity:.25}@keyframes heartbeat{0%,50%,100%{filter:brightness(.45)}9%,27%{filter:brightness(1)}}@keyframes flicker{50%{opacity:.35}}@media(max-width:720px){main{padding:22px 16px}.layout{grid-template-columns:1fr}.preview-card{order:2}.rig-preview{flex-direction:row;justify-content:center;gap:35px;padding:5px 0 20px}.spine{gap:2px;padding:9px 16px}.spine i{height:5px;width:55px}.buttons{grid-template-columns:repeat(2,1fr)}.card{padding:20px}.eyebrow{font-size:9px}footer{font-size:9px}}@media(prefers-reduced-motion:reduce){*,*:before,*:after{animation:none!important;transition:none!important}}
.tabs{display:flex;gap:8px;margin-bottom:20px}.tabs a{padding:8px 16px;color:var(--muted);text-decoration:none;border:1px solid var(--line);border-radius:7px;font-size:13px}.tabs a[aria-current=page]{color:var(--cyan);background:#174c4833}.mode-banner{display:flex;align-items:center;justify-content:space-between;gap:16px;padding:15px 20px;margin-bottom:20px;border:1px solid var(--line);border-radius:10px}.mode-banner[hidden]{display:none}.mode-banner p{margin:0;font-size:13px}.mode-banner button{flex-shrink:0}
</style>
</head>
<body>
<main>
<header><div><p class="eyebrow">CEC / Suit systems</p><h1>RIG CONTROL</h1></div><span id="connection" class="connection" role="status">Connecting</span></header>
<nav class="tabs" aria-label="Lighting pages"><a href="/" aria-current="page">RIG control</a><a href="/party">Party lights</a></nav>
<div class="mode-banner" id="modeBanner" hidden><p>Party lights are active. Your RIG levels are saved.</p><button id="activateRig">Activate RIG</button></div>
<div class="layout">
<section class="card preview-card" id="preview" aria-label="RIG preview">
<div class="card-head"><h2>Suit status</h2><span class="badge" id="effect">Standby</span></div>
<div class="rig-preview"><div><div class="status-leds" id="statusLeds" aria-hidden="true"></div><div class="preview-label">Status / LEDs 1–5</div></div><div><div class="spine" id="spineLeds" aria-hidden="true"></div><div class="preview-label">Spine / LEDs 6–24</div></div></div>
<div class="legend"><span id="healthReadout">Health —</span><span id="stasisReadout">Stasis —</span></div>
<p class="note">Preview shows current levels. The RIG plays the full animations.</p>
</section>
<div class="stack">
<section class="card" aria-labelledby="levelsTitle"><div class="card-head"><h2 id="levelsTitle">System levels</h2><span class="eyebrow">Live control</span></div>
<div class="field"><div class="field-head"><label for="health">Spine health</label><output id="healthValue" for="health">100%</output></div><input id="health" type="range" min="0" max="100" value="100"></div>
<div class="field"><div class="field-head"><label for="stasis">Stasis charge</label><output id="stasisValue" for="stasis">100%</output></div><input id="stasis" type="range" min="0" max="100" value="100"></div>
<div class="field"><div class="field-head"><label for="brightness">Brightness</label><output id="brightnessValue" for="brightness">100%</output></div><input id="brightness" type="range" min="0" max="255" value="255"></div>
<div class="stasis-actions"><button data-action="stasis-drain">Drain stasis</button><button data-action="stasis-recharge">Recharge stasis</button></div>
</section>
<section class="card" aria-labelledby="effectsTitle"><div class="card-head"><h2 id="effectsTitle">Animation controls</h2></div>
<div class="buttons"><button class="primary" data-action="power-up">Power up</button><button class="danger" data-action="damage">Take damage −25%</button><button data-action="heal">Heal to full</button><button class="danger" data-action="critical">Critical heartbeat</button><button data-action="malfunction">Malfunction</button><button data-action="power-down">Power down</button></div>
<div class="demo-row"><div><h2>Suit demonstration</h2><p>Cycle through every animation.</p></div><button class="primary" id="demo" disabled>Start demo</button></div>
<p class="note">Health, stasis and animation controls stop the demo. Brightness can be adjusted while it runs. Settings reset when the RIG restarts.</p>
</section>
</div></div>
<footer><span>RESOURCE INTEGRATION GEAR</span><span>LOCAL SUIT CONTROL</span></footer>
</main>
<div class="toast" id="toast" role="status" hidden></div>
<script>
'use strict';
const $=id=>document.getElementById(id);
let current=null,version=0,pending=0,queue=Promise.resolve(),toastTimer;
const names={startup:'Powering up',idle:'Online',damage:'Damage',healing:'Healing',critical:'Critical',malfunction:'Fault', 'power-down':'Powering down',off:'Offline'};
const statusPixels=Array.from({length:5},()=>{const x=document.createElement('i');$('statusLeds').appendChild(x);return x;});
const spinePixels=Array.from({length:19},()=>{const x=document.createElement('i');$('spineLeds').appendChild(x);return x;});
function online(ok){$('connection').textContent=ok?'Connected':'Reconnecting';$('connection').classList.toggle('online',ok);}
function notice(message){$('toast').textContent=message;$('toast').hidden=false;clearTimeout(toastTimer);toastTimer=setTimeout(()=>$('toast').hidden=true,4000);}
function buttons(){document.querySelectorAll('button').forEach(b=>b.disabled=!current||pending>0);}
function levelLabel(id,value){$(id+'Value').value=(id==='brightness'?Math.round(value*100/255):value)+'%';}
function paint(pixels,value,colour,powered,reverse=false){pixels.forEach((pixel,i)=>{const position=reverse?pixels.length-1-i:i;const lit=powered&&position<pixels.length*value/100;pixel.style.background=lit?colour:'';pixel.style.boxShadow=lit?'0 0 9px '+colour+'77':'';});}
function apply(state){current=state;online(true);['health','stasis','brightness'].forEach(id=>{if(document.activeElement!==$(id)){$(id).value=state[id];levelLabel(id,state[id]);}});const rigActive=state.mode==='rig';$('modeBanner').hidden=rigActive;$('effect').textContent=rigActive?(names[state.effect]||state.effect):'Party mode';$('preview').className='card preview-card '+(rigActive?state.effect:'off');$('healthReadout').textContent='Health '+state.health+'%';$('stasisReadout').textContent='Stasis '+state.stasis+'%';$('demo').textContent=state.demo?'Stop demo':'Start demo';$('demo').setAttribute('aria-pressed',String(state.demo));paint(statusPixels,state.stasis,'#69b7ff',state.powered&&rigActive,state.statusReversed);paint(spinePixels,state.health,state.health<=25?'#ff6058':state.health<=50?'#ffb84a':'#68f1de',state.powered&&rigActive,state.spineReversed);buttons();}
async function request(path,data){const options={cache:'no-store',signal:AbortSignal.timeout(5000)};if(data){options.method='POST';options.headers={'Content-Type':'application/x-www-form-urlencoded'};options.body=new URLSearchParams(data);}const response=await fetch(path,options);const result=await response.json();if(!response.ok)throw new Error(result.error||'Control failed');return result;}
function submit(path,data){const token=++version;pending++;buttons();queue=queue.then(async()=>{const state=await request(path,data);if(token===version)apply(state);}).catch(error=>{online(false);notice(error.message);}).finally(()=>{pending--;buttons();});}
['health','stasis','brightness'].forEach(id=>{$(id).addEventListener('input',()=>levelLabel(id,Number($(id).value)));$(id).addEventListener('change',()=>submit('/api/settings',{[id]:$(id).value}));});
document.querySelectorAll('[data-action]').forEach(button=>button.addEventListener('click',()=>submit('/api/action',{action:button.dataset.action})));
$('demo').addEventListener('click',()=>{if(current)submit('/api/action',{action:current.demo?'demo-stop':'demo-start'});});
async function poll(){const token=version;try{if(!pending){const state=await request('/api/state');if(token===version&&!pending)apply(state);}}catch(error){online(false);}finally{setTimeout(poll,700);}}
$('activateRig').addEventListener('click',()=>submit('/api/mode',{mode:'rig'}));
buttons();poll();
</script>
</body>
</html>)RIGPAGE";
