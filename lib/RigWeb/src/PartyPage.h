#pragma once

#include <pgmspace.h>

static const char PARTY_PAGE[] PROGMEM = R"PARTYPAGE(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="theme-color" content="#120e23">
<title>Party Lights</title>
<style>
:root{color-scheme:dark;--bg:#120e23;--panel:#1c172d;--line:#3c3152;--text:#f4edff;--muted:#b6a8cb;--accent:#e6b6ff}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(ellipse at 5% 0,#49245280,transparent 65%),var(--bg);color:var(--text);font:15px/1.5 system-ui,sans-serif;min-height:100vh}main{max-width:1050px;margin:auto;padding:36px 24px 24px}header{display:flex;justify-content:space-between;align-items:center;gap:16px;margin-bottom:28px}.eyebrow{text-transform:uppercase;letter-spacing:.22em;font-size:11px;color:var(--muted);margin:0 0 6px}h1{font-size:clamp(28px,5vw,40px);line-height:1.1;letter-spacing:.06em;margin:0;font-weight:650}h2{font-size:17px;margin:0}p{color:var(--muted);margin:6px 0 0}.connection{font-size:12px;white-space:nowrap;border:1px solid var(--line);border-radius:20px;padding:7px 12px;color:var(--muted)}.connection.online{color:var(--accent)}.connection:before{content:'\25cf';margin-right:7px}.tabs{display:flex;gap:8px;margin-bottom:20px}.tabs a{padding:8px 16px;color:var(--muted);text-decoration:none;border:1px solid var(--line);border-radius:7px;font-size:13px}.tabs a[aria-current=page]{color:var(--accent);background:#69437933}.card{background:linear-gradient(135deg,#291e38cc,var(--panel));border:1px solid var(--line);border-radius:14px;padding:23px}.card-head{display:flex;align-items:center;justify-content:space-between;gap:12px;margin-bottom:20px}.badge{font-size:11px;padding:4px 9px;border:1px solid #765087;border-radius:5px;color:var(--accent);text-transform:uppercase;letter-spacing:.07em}.strip-card{margin-bottom:20px}.strip{display:grid;grid-template-columns:repeat(12,minmax(0,1fr));gap:7px;margin:25px 0}.strip i{height:20px;border-radius:4px;background:#31243f}.strip-meta{display:flex;justify-content:space-between;gap:10px;color:var(--muted);font-size:12px}.columns{display:grid;grid-template-columns:1.15fr 1fr;gap:20px;align-items:start}.columns>*{min-width:0}.patterns{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px}button{font:inherit;font-size:13px;min-height:44px;cursor:pointer;border-radius:8px;padding:10px 13px;background:#30233f;color:var(--text);border:1px solid #59446f;transition:background .15s,transform .15s}button:hover{background:#483157}button:active{transform:translateY(1px)}button:disabled{opacity:.5;cursor:wait}button:focus-visible,input:focus-visible,a:focus-visible{outline:2px solid var(--accent);outline-offset:3px}.pattern{min-height:82px;text-align:left;display:flex;flex-direction:column;justify-content:center;gap:3px}.pattern strong{font-size:14px;font-weight:600}.pattern small{color:var(--muted);font-size:11px}.pattern[aria-pressed=true]{border-color:#dba7ed;background:#75468b55;box-shadow:inset 3px 0 var(--accent)}.field+.field{margin-top:23px}.field-head{display:flex;justify-content:space-between;gap:10px;margin-bottom:7px}label{font-size:14px}output{font-size:14px;color:var(--accent);font-variant-numeric:tabular-nums}input[type=range]{width:100%;height:28px;margin:0;accent-color:#d3a0f7;cursor:pointer}.colour-row{display:flex;align-items:center;gap:15px}input[type=color]{width:70px;height:48px;border:1px solid var(--line);border-radius:7px;padding:4px;background:#120e23;cursor:pointer}.colour-row p{font-size:12px}.actions{display:flex;gap:9px;margin-top:24px;flex-wrap:wrap}.actions button{flex:1}.primary{background:#633e79;border-color:#aa7bc0}.playlist{margin-top:24px;padding-top:21px;border-top:1px solid var(--line);display:flex;align-items:center;justify-content:space-between;gap:14px}.playlist p{font-size:12px}.playlist button{min-width:110px}.note{font-size:12px;line-height:1.6;margin-top:18px}.toast{position:fixed;bottom:20px;left:50%;transform:translateX(-50%);background:#49335b;border:1px solid #9672ac;border-radius:8px;padding:12px 18px;max-width:90vw;z-index:2}.toast[hidden]{display:none}footer{margin-top:20px;color:var(--muted);font-size:11px;letter-spacing:.08em;display:flex;justify-content:space-between}@media(max-width:720px){main{padding:22px 16px}.columns{grid-template-columns:1fr}.card{padding:20px}.eyebrow{font-size:9px}.strip{gap:5px}.strip i{height:17px}footer{font-size:9px}}@media(prefers-reduced-motion:reduce){*{animation:none!important;transition:none!important}}
.pattern[data-pattern=strobe]{grid-column:1/-1}
</style>
</head>
<body><main>
<header><div><p class="eyebrow">Make some atmosphere</p><h1>PARTY LIGHTS</h1></div><span id="connection" class="connection" role="status">Connecting</span></header>
<nav class="tabs" aria-label="Lighting pages"><a href="/">RIG control</a><a href="/party" aria-current="page">Party lights</a></nav>
<section class="card strip-card" aria-label="Party preview"><div class="card-head"><h2>All 24 LEDs</h2><span class="badge" id="mode">Standby</span></div><div class="strip" id="strip" aria-hidden="true"></div><div class="strip-meta"><span id="playing">Choose an effect to start</span><span id="cycleStatus">Manual selection</span></div><p class="note" id="modeNote">Choosing an effect activates Party mode. Your RIG levels are kept.</p></section>
<div class="columns">
<section class="card" aria-labelledby="effectsTitle"><div class="card-head"><h2 id="effectsTitle">Pick an effect</h2><span class="eyebrow">Full strip</span></div>
<div class="patterns">
<button class="pattern" data-pattern="rainbow"><strong>Rainbow</strong><small>Rolling spectrum</small></button>
<button class="pattern" data-pattern="wave"><strong>Colour wave</strong><small>Flowing rainbow ribbons</small></button>
<button class="pattern" data-pattern="chase"><strong>Chase</strong><small>Marching colour dots</small></button>
<button class="pattern" data-pattern="sparkle"><strong>Sparkle</strong><small>Colour with white glints</small></button>
<button class="pattern" data-pattern="breathe"><strong>Breathing</strong><small>Soft colour pulses</small></button>
<button class="pattern" data-pattern="scanner"><strong>Scanner</strong><small>A bouncing beam</small></button>
<button class="pattern" data-pattern="comet"><strong>Comet</strong><small>A glowing trail</small></button>
<button class="pattern" data-pattern="solid"><strong>Solid colour</strong><small>Keep your favourite colour</small></button>
<button class="pattern" data-pattern="strobe"><strong>Strobe</strong><small>Colour flashes ? adjustable speed</small></button>
</div></section>
<section class="card" aria-labelledby="controlsTitle"><div class="card-head"><h2 id="controlsTitle">Set the mood</h2></div>
<div class="field"><div class="field-head"><label for="colour">Effect colour</label><output id="colourValue" for="colour">#FF35A1</output></div><div class="colour-row"><input type="color" id="colour" value="#ff35a1"><p>Used by chase, sparkle, breathing, scanner, comet, solid and strobe.</p></div></div>
<div class="field"><div class="field-head"><label for="speed">Speed</label><output id="speedValue" for="speed">50%</output></div><input type="range" id="speed" min="1" max="100" value="50"></div>
<div class="field"><div class="field-head"><label for="brightness">Brightness</label><output id="brightnessValue" for="brightness">100%</output></div><input type="range" id="brightness" min="0" max="255" value="255"></div>
<div class="playlist"><div><h2>Party playlist</h2><p>Change effects every 10 seconds.</p></div><button id="cycle">Start playlist</button></div>
<div class="actions"><button class="primary" id="start">Start party</button><button id="rig">Activate RIG</button></div>
<p class="note">Opening either page keeps the lights playing. Activate RIG to return to the suit effects. Settings reset when the ESP restarts.</p>
</section></div>
<footer><span>YOUR COLOUR. YOUR MOOD.</span><span>LOCAL LIGHT CONTROL</span></footer>
</main><div class="toast" id="toast" role="status" hidden></div>
<script>
'use strict';
const $=id=>document.getElementById(id);
const names={rainbow:'Rainbow',wave:'Colour wave',chase:'Chase',sparkle:'Sparkle',breathe:'Breathing',scanner:'Scanner',comet:'Comet',solid:'Solid colour',strobe:'Strobe'};
const reduceMotion=matchMedia('(prefers-reduced-motion: reduce)').matches;
let current=null,pending=0,version=0,queue=Promise.resolve(),toastTimer,lastPaint=0;
const pixels=Array.from({length:24},()=>{const p=document.createElement('i');$('strip').appendChild(p);return p;});
function online(ok){$('connection').textContent=ok?'Connected':'Reconnecting';$('connection').classList.toggle('online',ok);}
function buttons(){document.querySelectorAll('button').forEach(b=>b.disabled=!current||pending>0);if(current&&current.mode==='party')$('start').disabled=true;}
function notice(message){$('toast').textContent=message;$('toast').hidden=false;clearTimeout(toastTimer);toastTimer=setTimeout(()=>$('toast').hidden=true,4000);}
function label(id,value){$(id+'Value').value=id==='colour'?value.toUpperCase():(id==='brightness'?Math.round(value*100/255):value)+'%';}
function apply(state){current=state;online(true);const active=state.mode==='party';$('mode').textContent=active?'Party active':'RIG active';$('playing').textContent=active?names[state.partyPattern]:'RIG mode is playing';$('cycleStatus').textContent=state.partyCycle?'Playlist on':'Manual selection';$('cycle').textContent=state.partyCycle?'Stop playlist':'Start playlist';$('cycle').setAttribute('aria-pressed',String(state.partyCycle));$('start').textContent=active?'Party active':'Start party';$('modeNote').textContent=active?'Party effects use the entire strip. Your RIG levels are kept.':'Choose an effect or press Start party to switch from RIG mode.';document.querySelectorAll('[data-pattern]').forEach(b=>b.setAttribute('aria-pressed',String(active&&b.dataset.pattern===state.partyPattern)));const values={colour:state.partyColour,speed:state.partySpeed,brightness:state.brightness};Object.entries(values).forEach(([id,value])=>{if(document.activeElement!==$(id)){$(id).value=value;label(id,value);}});buttons();}
async function request(path,data){const options={cache:'no-store',signal:AbortSignal.timeout(5000)};if(data){options.method='POST';options.headers={'Content-Type':'application/x-www-form-urlencoded'};options.body=new URLSearchParams(data);}const response=await fetch(path,options);const state=await response.json();if(!response.ok)throw new Error(state.error||'Control failed');return state;}
function submit(path,data){const token=++version;pending++;buttons();queue=queue.then(async()=>{const state=await request(path,data);if(token===version)apply(state);}).catch(error=>{online(false);notice(error.message);}).finally(()=>{pending--;buttons();});}
document.querySelectorAll('[data-pattern]').forEach(b=>b.addEventListener('click',()=>submit('/api/party',{pattern:b.dataset.pattern})));
['colour','speed','brightness'].forEach(id=>{$(id).addEventListener('input',()=>label(id,$(id).value));$(id).addEventListener('change',()=>submit(id==='brightness'?'/api/settings':'/api/party',{[id]:$(id).value}));});
$('cycle').addEventListener('click',()=>{if(current)submit('/api/party',{cycle:current.partyCycle?'0':'1'});});
$('start').addEventListener('click',()=>submit('/api/mode',{mode:'party'}));$('rig').addEventListener('click',()=>submit('/api/mode',{mode:'rig'}));
async function poll(){const token=version;try{if(!pending){const state=await request('/api/state');if(token===version&&!pending)apply(state);}}catch(error){online(false);}finally{setTimeout(poll,700);}}
function draw(now){requestAnimationFrame(draw);if(now-lastPaint<55)return;lastPaint=now;const active=current&&current.mode==='party';const speed=current?current.partySpeed:50;const phase=now/(12000-speed*100);const step=Math.floor(now/(230-speed*2));pixels.forEach((p,i)=>{let colour=current?current.partyColour:'#ff35a1',glow=1;const pattern=current?current.partyPattern:'rainbow';if(pattern==='rainbow'||pattern==='wave'){colour='hsl('+((phase*360+i*360/24)%360)+' 90% 65%)';if(pattern==='wave')glow=.2+.8*(1+Math.sin(phase*Math.PI*4+i*.42))/2;}else if(pattern==='chase')glow=(i+step)%3===0?1:0;else if(pattern==='sparkle')glow=((step+1)*73+i*47)%13===0?1:.08;else if(pattern==='breathe')glow=.15+.85*(1+Math.sin(phase*Math.PI*2))/2;else if(pattern==='strobe'){const period=1000-speed*9;glow=reduceMotion||now%period<period/4?1:0;}else if(pattern==='scanner'){let head=step%46;if(head>23)head=46-head;glow=Math.max(0,1-Math.abs(i-head)/5);}else if(pattern==='comet')glow=Math.pow(.55,(step%24+24-i)%24);glow*=current?current.brightness/255:1;p.style.background=active&&glow>.02?colour:'#31243f';p.style.opacity=active?Math.max(.12,glow):1;p.style.boxShadow=active&&glow>.3?'0 0 12px '+colour:'none';});}
buttons();poll();requestAnimationFrame(draw);
</script></body></html>)PARTYPAGE";
