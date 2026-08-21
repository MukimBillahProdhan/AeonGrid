#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>AeonGrid - Analytics Dashboard</title>
  <style>
    :root {
      --bg-base: #0b0f19;
      --glow-1: rgba(59, 130, 246, 0.15);
      --glow-2: rgba(139, 92, 246, 0.15);
      --glow-3: rgba(16, 185, 129, 0.10);
      
      --card-bg: rgba(18, 24, 38, 0.85);
      --card-border: rgba(255, 255, 255, 0.08);
      --card-outer-glow: rgba(59, 130, 246, 0.15);
      --inner-bg: rgba(15, 23, 42, 0.8);
      --text-main: #e2e8f0;
      --text-sub: #94a3b8;
      --text-muted: #64748b;
      --border-color: rgba(255, 255, 255, 0.08);
      
      --color-emerald: #34d399;
      --color-rose: #fb7185;
      --color-blue: #60a5fa;
      --color-amber: #fbbf24;
      --circle-track: rgba(255, 255, 255, 0.05);
    }

    body.light-theme {
      --bg-base: #f8fafc;
      --glow-1: rgba(56, 189, 248, 0.35);
      --glow-2: rgba(192, 132, 252, 0.30);
      --glow-3: rgba(52, 211, 153, 0.30);
      
      --card-bg: rgba(255, 255, 255, 0.90);
      --card-border: rgba(0, 0, 0, 0.08);
      --card-outer-glow: rgba(56, 189, 248, 0.25);
      --inner-bg: rgba(241, 245, 249, 0.9);
      --text-main: #0f172a;
      --text-sub: #475569;
      --text-muted: #94a3b8;
      --border-color: rgba(0, 0, 0, 0.08);
      --circle-track: rgba(0, 0, 0, 0.05);
    }

    * { box-sizing: border-box; margin: 0; padding: 0; }
    
    body { 
      background-color: var(--bg-base);
      color: var(--text-main); 
      font-family: "JetBrains Mono", "SF Mono", "Cascadia Code", "Fira Code", monospace; 
      line-height: 1.5; padding: 20px; 
      transition: background-color 0.4s, color 0.4s;
      min-height: 100vh;
      overflow-x: hidden;
    }

    .glow-overlay {
      position: fixed;
      top: 0; left: 0; width: 100vw; height: 100vh;
      background-image: 
        radial-gradient(circle at 15% 20%, var(--glow-1) 0%, transparent 45%),
        radial-gradient(circle at 85% 30%, var(--glow-2) 0%, transparent 50%),
        radial-gradient(circle at 50% 85%, var(--glow-3) 0%, transparent 55%);
      background-size: 200% 200%;
      animation: roamMesh 25s infinite alternate ease-in-out;
      pointer-events: none;
      z-index: 0;
    }

    @keyframes roamMesh {
      0% { background-position: 0% 0%; }
      50% { background-position: 100% 50%; }
      100% { background-position: 0% 100%; }
    }

    .max-w-7xl, #toast-container, .modal { position: relative; z-index: 1; }

    .card-glow { 
        box-shadow: 0 10px 30px rgba(0,0,0,0.2), 0 0 12px var(--card-outer-glow); 
        backdrop-filter: blur(12px); 
        -webkit-backdrop-filter: blur(12px); 
    }

    .max-w-7xl { max-width: 80rem; margin: 0 auto; }
    .card-bg { background-color: var(--card-bg); border: 1px solid var(--card-border); border-radius: 0.75rem; padding: 1.25rem; transition: background-color 0.3s, border-color 0.3s; }
    
    .flex { display: flex; } .flex-col { flex-direction: column; } .items-center { align-items: center; } 
    .justify-between { justify-content: space-between; } .justify-center { justify-content: center; }
    
    .w-full { width: 100%; }
    .grid { display: grid; } 
    .gap-1 { gap: 0.25rem; } .gap-2 { gap: 0.5rem; } .gap-4 { gap: 1rem; } .gap-6 { gap: 1.5rem; }
    .grid-cols-1 { grid-template-columns: repeat(1, minmax(0, 1fr)); }
    .grid-cols-2 { grid-template-columns: repeat(2, minmax(0, 1fr)); }
    .grid-cols-3 { grid-template-columns: repeat(3, minmax(0, 1fr)); }
    .grid-cols-7 { grid-template-columns: repeat(7, minmax(0, 1fr)); }
    @media (min-width: 900px) { .md-grid-cols-3 { grid-template-columns: 1.35fr 1fr 1fr; } .md-flex-row { flex-direction: row; } }
    
    .text-white { color: var(--text-main); } .text-slate-400 { color: var(--text-sub); } .text-slate-300 { color: var(--text-main); }
    .text-emerald-400 { color: var(--color-emerald); } .text-rose-400 { color: var(--color-rose); } .text-blue-400 { color: var(--color-blue); } .text-amber-400 { color: var(--color-amber); }
    .text-center { text-align: center; } .text-right { text-align: right; } .text-left { text-align: left; }
    .text-[8px] { font-size: 0.5rem; } .text-[9px] { font-size: 0.5625rem; } .text-[10px] { font-size: 0.625rem; } .text-xs { font-size: 0.75rem; } .text-sm { font-size: 0.875rem; } .text-xl { font-size: 1.25rem; } .text-2xl { font-size: 1.5rem; } .text-3xl { font-size: 1.875rem; } .text-5xl { font-size: 3rem; }
    .font-bold { font-weight: 700; } .font-medium { font-weight: 500; } .font-semibold { font-weight: 600; } .tracking-wider { letter-spacing: 0.05em; }
    
    .border-b { border-bottom: 1px solid var(--border-color); } .border-t { border-top: 1px solid var(--border-color); }
    .rounded-lg { border-radius: 0.5rem; } 
    .p-1 { padding: 0.25rem; } .p-2 { padding: 0.5rem; } .p-3 { padding: 0.75rem; } .py-1 { padding-top: 0.25rem; padding-bottom: 0.25rem; } .px-2 { padding-left: 0.5rem; padding-right: 0.5rem; }
    .mt-1 { margin-top: 0.25rem; } .mt-2 { margin-top: 0.5rem; } .mt-3 { margin-top: 0.75rem; } .mt-4 { margin-top: 1rem; } .mb-2 { margin-bottom: 0.5rem; } .mb-4 { margin-bottom: 1rem; }
    
    button { cursor: pointer; border: none; font-family: inherit; background: transparent; }
    .btn { padding: 0.5rem 1rem; border-radius: 0.375rem; font-weight: 600; font-size: 0.75rem; transition: 0.2s; text-align: center;}
    .bg-slate-800 { background-color: var(--inner-bg); } .bg-slate-900 { background-color: var(--inner-bg); } .hover\:bg-slate-700:hover { opacity: 0.8; }

    .segmented-control { position: relative; display: flex; background: var(--inner-bg); border-radius: 0.5rem; padding: 4px; overflow: hidden; border: 1px solid var(--border-color); margin: 12px 0; }
    .slider-bg { position: absolute; top: 4px; bottom: 4px; width: calc(33.333% - 2.66px); border-radius: 0.375rem; transition: transform 0.3s cubic-bezier(0.4, 0, 0.2, 1), background-color 0.3s, box-shadow 0.3s; z-index: 0; }
    .seg-btn { flex: 1; position: relative; z-index: 1; color: var(--text-sub); padding: 0.5rem 0; font-size: 0.75rem; font-weight: 600; text-align: center; cursor: pointer; transition: color 0.3s; }
    .seg-btn.active { color: #fff; }
    
    input[type="text"], input[type="password"], input[type="number"], input[type="time"], input[type="file"] { width: 100%; padding: 0.6rem; border-radius: 0.375rem; border: 1px solid var(--border-color); background: var(--inner-bg); color: var(--text-main); font-family: inherit; outline: none; margin-bottom: 8px;}
    input:focus { border-color: #3b82f6; }
    
    .modal { display: none; position: fixed; inset: 0; background: rgba(0,0,0,0.6); backdrop-filter: blur(6px); align-items: center; justify-content: center; z-index: 50; padding: 1rem; overflow-y: auto;}
    .modal.show { display: flex; }
    .modal-content { background: var(--card-bg); border: 1px solid var(--card-border); border-radius: 0.75rem; padding: 1.5rem; width: 100%; max-width: 440px; margin: auto; max-height: 90vh; overflow-y: auto; color: var(--text-main); box-shadow: 0 10px 40px rgba(0,0,0,0.4);}
    
    table { width: 100%; border-collapse: collapse; text-align: left; } th, td { padding: 0.75rem; border-bottom: 1px solid var(--border-color); }
    .svg-icon { width: 20px; height: 20px; stroke: currentColor; stroke-width: 2; stroke-linecap: round; stroke-linejoin: round; fill: none; }
    
    .badge { padding: 3px 12px; border-radius: 9999px; font-size: 11px; font-weight: bold; border: 1px solid; display: inline-flex; align-items: center; justify-content: center; background: transparent;}
    .badge-large { padding: 5px 14px; border-radius: 9999px; font-size: 12px; font-weight: bold; border: 1px solid; letter-spacing: 0.5px; background: transparent;}
    
    .circle-bg { fill: none; stroke: var(--circle-track); }
    .circle-fg { fill: none; stroke-dasharray: 0, 100; transition: stroke-dasharray 0.5s ease-out, stroke 0.5s; stroke-linecap: round; }
    
    @keyframes comet-dash {
        0%   { stroke-dasharray: 0, 100; stroke-dashoffset: 0; opacity: 0; }
        10%  { opacity: 1; }
        40%  { stroke-dasharray: 20, 80; stroke-dashoffset: 0; opacity: 1; }
        90%  { opacity: 1; }
        100% { stroke-dasharray: 0, 100; stroke-dashoffset: var(--comet-end, -100); opacity: 0; }
    }
    .comet-anim { animation: comet-dash 1.8s cubic-bezier(0.4, 0, 0.2, 1) infinite; }

    .chart-container { position: relative; width: 85px; height: 85px; margin: 8px auto; display: flex; align-items: center; justify-content: center; }
    .chart-center-text { position: absolute; font-weight: 700; font-size: 14px; text-align: center; color: var(--text-main); }
    
    @keyframes pulse-dot { 0%, 100% { opacity: 1; } 50% { opacity: 0.4; } }
    .animate-pulse { animation: pulse-dot 1.5s cubic-bezier(0.4, 0, 0.6, 1) infinite; }
    
    .heat-col { display: flex; flex-direction: column; gap: 4px; align-items: center; }
    .heat-head { font-size: 10px; color: var(--text-muted); font-weight: bold; margin-bottom: 2px; }
    .heat-block { width: 100%; height: 8px; border-radius: 2px; background: rgba(0,0,0,0.1); transition: background-color 0.3s; }
    .heat-green { background: var(--color-emerald); box-shadow: 0 0 4px var(--color-emerald); }
    .heat-amber { background: var(--color-amber); box-shadow: 0 0 4px var(--color-amber); }
    .heat-red { background: var(--color-rose); box-shadow: 0 0 4px var(--color-rose); }

    #toast-container { position: fixed; top: 20px; left: 50%; transform: translateX(-50%); z-index: 9999; display: flex; flex-direction: column; gap: 10px; pointer-events: none;}
    .toast { background: var(--card-bg); color: var(--text-main); padding: 12px 24px; border-radius: 9999px; font-size: 13px; font-weight: 600; border: 1px solid var(--border-color); transform: translateY(-20px); opacity: 0; animation: slideIn 0.3s forwards, slideOut 0.3s 3s forwards; box-shadow: 0 4px 15px rgba(0,0,0,0.2);}
    .toast.success { border-left: 4px solid var(--color-emerald); } .toast.error { border-left: 4px solid var(--color-rose); } .toast.warning { border-left: 4px solid var(--color-amber); } .toast.info { border-left: 4px solid var(--color-blue); }
    @keyframes slideIn { to { transform: translateY(0); opacity: 1; } }
    @keyframes slideOut { to { transform: translateY(-20px); opacity: 0; } }

    .switch { position: relative; display: inline-block; width: 34px; height: 20px; }
    .switch input { opacity: 0; width: 0; height: 0; }
    .slider { position: absolute; cursor: pointer; top: 0; left: 0; right: 0; bottom: 0; background-color: var(--inner-bg); transition: .4s; border-radius: 34px; border: 1px solid var(--border-color);}
    .slider:before { position: absolute; content: ""; height: 14px; width: 14px; left: 2px; bottom: 2px; background-color: var(--text-sub); transition: .4s; border-radius: 50%; }
    input:checked + .slider { background-color: #3b82f6; border-color:#3b82f6; }
    input:checked + .slider:before { transform: translateX(14px); background-color: white; }
  </style>
</head>
<body>

  <div class="glow-overlay"></div>
  <div id="toast-container"></div>

  <div class="max-w-7xl flex-col gap-6">
    <header class="flex justify-between items-center border-b" style="padding-bottom: 1rem; margin-bottom: 1.5rem;">
      <div class="flex items-center" style="gap: 12px;">
        <div style="padding: 8px; background: rgba(37,99,235,0.15); border-radius: 8px; color: #60a5fa;">
          <svg class="svg-icon" viewBox="0 0 24 24"><polygon points="13 2 3 14 12 14 11 22 21 10 12 10 13 2"></polygon></svg>
        </div>
        <div>
          <h1 class="text-xl font-bold text-white">AeonGrid</h1>
          <p class="text-xs text-slate-400">Smart Battery AI Tracker</p>
        </div>
      </div>
      
      <div class="flex items-center" style="gap:10px;">
        <button onclick="toggleTheme()" class="card-bg card-glow" style="padding: 8px; display:flex; align-items:center;" title="Toggle Light/Dark Theme">
          <span id="theme-icon">☀️</span>
        </button>

        <div class="flex items-center text-xs font-bold bg-slate-800 border border-slate-700 rounded-lg p-1 card-glow" style="gap:2px;">
          <span id="uptime-text" class="text-slate-400 px-2" title="System Session Uptime">Up: --</span>
          <span class="text-slate-600">|</span>
          <span id="live-clock" class="text-white px-2" title="Local System Time">--:--</span>
          <button onclick="syncTime()" class="p-1 hover:bg-slate-700 rounded transition text-blue-400" title="Sync Time">🔄</button>
        </div>
        
        <button id="btn-buzzer" onclick="toggleBuzzer()" class="card-bg card-glow" style="padding: 8px; display:flex; align-items:center;" title="Mute Buzzer">
          <svg id="icon-buzzer" class="svg-icon text-slate-400" viewBox="0 0 24 24"><polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"></polygon><path d="M19.07 4.93a10 10 0 0 1 0 14.14M15.54 8.46a5 5 0 0 1 0 7.07"></path></svg>
        </button>
        <button onclick="toggleModal('settings-modal')" class="card-bg card-glow" style="padding: 8px;" title="Settings">
          <svg class="svg-icon text-slate-400" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
            <path d="M12.22 2h-.44a2 2 0 0 0-2 2v.18a2 2 0 0 1-1 1.73l-.43.25a2 2 0 0 1-2 0l-.15-.08a2 2 0 0 0-2.73.73l-.22.38a2 2 0 0 0 .73 2.73l.15.1a2 2 0 0 1 1 1.72v.51a2 2 0 0 1-1 1.74l-.15.09a2 2 0 0 0-.73 2.73l.22.38a2 2 0 0 0 2.73.73l.15-.08a2 2 0 0 1 2 0l.43.25a2 2 0 0 1 1 1.73V20a2 2 0 0 0 2 2h.44a2 2 0 0 0 2-2v-.18a2 2 0 0 1 1-1.73l.43-.25a2 2 0 0 1 2 0l.15.08a2 2 0 0 0 2.73-.73l.22-.39a2 2 0 0 0-.73-2.73l-.15-.08a2 2 0 0 1-1-1.74v-.5a2 2 0 0 1 1-1.74l.15-.09a2 2 0 0 0 .73-2.73l-.22-.38a2 2 0 0 0-2.73-.73l-.15.08a2 2 0 0 1-2 0l-.43-.25a2 2 0 0 1-1-1.73V4a2 2 0 0 0-2-2z"></path>
            <circle cx="12" cy="12" r="3"></circle>
          </svg>
        </button>
      </div>
    </header>

    <div class="grid grid-cols-1 md-grid-cols-3 gap-6 mb-4">
      
      <!-- 1. BATTERY TILE -->
      <div class="card-bg card-glow flex flex-col justify-between">
        <div>
          <div class="flex justify-between items-center text-xs text-slate-400 mb-2 w-full font-semibold">
            <span>BATTERY STATE</span>
          </div>
          
          <div class="flex items-center justify-center mt-4 mb-2" style="gap: 2rem;">
            <div style="position:relative; width: 130px; height: 130px; display:flex; align-items:center; justify-content:center; flex-shrink:0;">
              <svg style="width:100%; height:100%; transform: rotate(-90deg); overflow: visible;" viewBox="0 0 36 36">
                <path class="circle-bg" stroke-width="3" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" />
                <path id="soc-ring" class="circle-fg" stroke-width="3" stroke="#34d399" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" />
                <path id="soc-comet" class="circle-fg" stroke-width="3" stroke="#ffffff" style="display:none; filter: drop-shadow(0 0 5px rgba(255,255,255,0.9));" d="M18 2.0845 a 15.9155 15.9155 0 0 1 0 31.831 a 15.9155 15.9155 0 0 1 0 -31.831" />
              </svg>
              <div class="flex flex-col items-center" style="position:absolute; text-align:center;">
                <span id="soc-text" class="font-bold text-white" style="font-size: 3.5rem; line-height: 1;">--<span class="text-xl">%</span></span>
                <span id="soc-status" class="text-slate-400" style="font-size: 10px; font-weight: 600; letter-spacing: 0.15em; margin-top: 4px;">SYNC</span>
              </div>
            </div>

            <div class="text-left">
              <span id="voltage-text" class="text-5xl font-bold text-white">--.-<span class="text-2xl text-slate-400">V</span></span>
              <p id="eta-text" class="text-xs text-slate-400 mt-1 font-medium">Estimating Time...</p>
              <p id="last-full-text" class="text-slate-500 mt-2" style="font-size: 14px; font-weight: 400;">Last full: Not recorded</p>
            </div>
          </div>
        </div>
      </div>

      <!-- 2. GRID TELEMETRY TILE -->
      <div class="card-bg card-glow flex flex-col justify-between">
        <div>
          <div class="flex justify-between items-center text-xs text-slate-400 mb-2 w-full font-semibold">
            <span>GRID TELEMETRY</span>
            <span id="grid-status-badge" class="badge-large">Wait...</span>
          </div>
          
          <div class="flex justify-between items-center text-xs text-slate-400 mt-2">
            <span>7-Day Grid Matrix</span>
            <span id="live-cut-indicator" class="text-emerald-400 font-semibold">Normal</span>
          </div>

          <div class="grid grid-cols-7 gap-1 mt-2 bg-slate-900 p-2 rounded-lg" id="grid-heatmap"></div>
        </div>
        
        <div class="flex justify-between items-center border-t pt-2 w-full text-xs text-slate-400 mt-2">
          <span>Weekly Stability</span>
          <span class="text-white font-bold" id="grid-health-quick">--% Up</span>
        </div>
      </div>

      <!-- 3. CHARGER CONTROL TILE -->
      <div class="card-bg card-glow flex flex-col justify-between">
        <div>
          <div class="flex items-center justify-between text-xs text-slate-400 mb-1 w-full font-semibold">
            <span class="flex items-center" style="gap:6px;">
              <svg class="svg-icon text-blue-400" viewBox="0 0 24 24"><path d="M12 22v-5"></path><path d="M9 8V2"></path><path d="M15 8V2"></path><path d="M18 8v5a4 4 0 0 1-4 4h-4a4 4 0 0 1-4-4V8Z"></path></svg> CHARGER CONTROL
            </span>
            <span id="relay-status-badge" class="badge">--</span>
          </div>
          
          <div class="segmented-control">
            <div id="mode-slider" class="slider-bg"></div>
            <div id="btn-auto" class="seg-btn active" onclick="setChargerMode(0)">Auto</div>
            <div id="btn-on" class="seg-btn" onclick="setChargerMode(1)">Force ON</div>
            <div id="btn-off" class="seg-btn" onclick="setChargerMode(2)">Force OFF</div>
          </div>

          <p class="text-slate-400 text-center mt-2" style="font-size: 14px; font-weight: 400;">Low <span id="lim-l" class="text-white" style="font-weight: 400;">--</span>V &bull; Res <span id="lim-r" class="text-white" style="font-weight: 400;">--</span>V &bull; High <span id="lim-h" class="text-white" style="font-weight: 400;">--</span>V</p>
        </div>

        <div id="schedule-indicator" class="text-center mt-2 text-slate-400 border-t pt-2" style="font-size: 14px; font-weight: 400;">Smart Timer Disabled</div>
      </div>
    </div>

    <!-- ANALYTICS SECTION -->
    <div class="card-bg card-glow flex flex-col gap-6">
      <div class="flex md-flex-row flex-col justify-between items-center border-b w-full" style="padding-bottom:1rem;">
        <div class="flex items-center" style="gap:8px;">
          <svg class="svg-icon text-blue-400" viewBox="0 0 24 24"><line x1="18" y1="20" x2="18" y2="10"></line><line x1="12" y1="20" x2="12" y2="4"></line><line x1="6" y1="20" x2="6" y2="14"></line></svg>
          <h2 class="text-sm font-semibold text-white">AI Grid Analytics <span id="active-date-label" class="text-slate-400">(Today)</span></h2>
        </div>
        <div class="flex" style="gap:10px;">
          <a href="/api/download_csv" class="btn bg-emerald-600 text-white flex items-center" style="gap:5px; text-decoration:none; background:#059669;">
            <svg class="svg-icon" style="width:14px;height:14px;" viewBox="0 0 24 24"><path d="M21 15v4a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2v-4"></path><polyline points="7 10 12 15 17 10"></polyline><line x1="12" y1="15" x2="12" y2="3"></line></svg> CSV
          </a>
          <button onclick="openCalendar()" class="btn bg-slate-800 text-slate-300 flex items-center" style="gap:5px;">
            <svg class="svg-icon text-blue-400" style="width:16px;height:16px;" viewBox="0 0 24 24"><rect x="3" y="4" width="18" height="18" rx="2" ry="2"></rect><line x1="16" y1="2" x2="16" y2="6"></line><line x1="8" y1="2" x2="8" y2="6"></line><line x1="3" y1="10" x2="21" y2="10"></line></svg>
            <span id="date-btn-text">Select Date</span>
          </button>
        </div>
      </div>

      <div class="grid grid-cols-1 md-grid-cols-3 gap-4">
        <div class="bg-slate-900 rounded-lg flex flex-col items-center justify-center p-4 border-t">
          <div class="flex justify-between w-full items-center mb-2">
             <span class="text-xs text-slate-400 font-semibold">Selected Day</span>
             <span class="badge text-amber-400" style="border-color:rgba(251,191,36,0.3);" id="today-cuts-text">0 Cuts</span>
          </div>
          <div class="chart-container">
             <canvas id="dailyCanvas" width="85" height="85" style="position:absolute; top:0; left:0;"></canvas>
             <span id="daily-center-text" class="chart-center-text">--%</span>
          </div>
          <span class="text-xs font-bold mt-2 w-full text-center" id="daily-text-stats"><span class="text-emerald-400">--h --m</span> <span class="text-slate-600">/</span> <span class="text-rose-400">--h --m</span></span>
        </div>
        
        <div class="bg-slate-900 rounded-lg flex flex-col items-center justify-center p-4 border-t">
          <div class="flex justify-between w-full items-center mb-2">
             <span class="text-xs text-slate-400 font-semibold">Last 30 Days Score</span>
             <span class="badge" id="health-grade-text">Grade --</span>
          </div>
          <div class="chart-container">
             <canvas id="monthlyCanvas" width="85" height="85" style="position:absolute; top:0; left:0;"></canvas>
             <span id="monthly-center-text" class="chart-center-text">--%</span>
          </div>
          <span class="text-xs font-bold mt-2 w-full text-center" id="monthly-text-stats"><span class="text-emerald-400">--h --m</span> <span class="text-slate-600">/</span> <span class="text-rose-400">--h --m</span></span>
        </div>

        <div class="bg-slate-900 rounded-lg p-3 border-t flex flex-col justify-between gap-1 w-full">
          <span class="text-xs text-slate-400 font-semibold border-b border-slate-800 pb-1 mb-1">System Intelligence</span>
          <div class="flex justify-between w-full text-xs mt-1">
            <span class="text-slate-400">Trend (7d):</span> <span id="trend-text" class="text-amber-400 font-bold text-right">Analyzing...</span>
          </div>
          <div class="flex justify-between w-full text-xs mt-1">
            <span class="text-slate-400">Worst (7d):</span> <span id="worst-7-text" class="text-rose-400 font-bold text-right">--</span>
          </div>
          <div class="flex justify-between w-full text-xs mt-1">
            <span class="text-slate-400">Worst (30d):</span> <span id="worst-30-text" class="text-rose-400 font-bold text-right">--</span>
          </div>
          
          <span class="text-xs text-slate-400 font-semibold border-b border-slate-800 pb-1 mb-1 mt-3 w-full">System Totals (last 30 days)</span>
          <div class="flex justify-between w-full text-xs mt-1">
            <span class="text-emerald-400 font-bold">Total Up: <span id="total-uptime-text" class="text-white">--</span></span>
            <span class="text-rose-400 font-bold">Down: <span id="total-downtime-text" class="text-white">--</span></span>
          </div>
          <div class="flex justify-between w-full text-xs mt-1">
            <span class="text-slate-400">Total Cuts:</span> <span id="total-cuts-text" class="text-white font-bold">--</span>
          </div>
        </div>
      </div>

      <div style="overflow-x: auto;">
        <table class="text-xs w-full">
          <thead class="text-slate-400 bg-slate-900">
            <tr><th class="text-left">Power Cut Start</th><th class="text-left">Restored At</th><th class="text-right">Duration</th></tr>
          </thead>
          <tbody id="outage-log-body"></tbody>
        </table>
      </div>
    </div>
  </div>

  <!-- CALENDAR MODAL -->
  <div id="date-modal" class="modal">
    <div class="modal-content text-center">
      <div class="flex justify-between items-center border-b mb-2 w-full" style="padding-bottom:10px;">
        <button onclick="changeMonth(-1)" class="p-2 text-slate-400 bg-slate-900 rounded-lg">&#9664;</button>
        <h3 class="text-sm font-bold text-white" id="cal-month-title">--</h3>
        <button onclick="changeMonth(1)" class="p-2 text-slate-400 bg-slate-900 rounded-lg">&#9654;</button>
      </div>
      <div class="grid grid-cols-7 gap-1 text-xs font-semibold text-slate-500 mb-2">
        <span>Su</span><span>Mo</span><span>Tu</span><span>We</span><span>Th</span><span>Fr</span><span>Sa</span>
      </div>
      <div id="cal-days-grid" class="grid grid-cols-7 gap-1 text-xs"></div>
      <button onclick="toggleModal('date-modal')" class="btn bg-slate-800 text-white mt-4 w-full">Close Calendar</button>
    </div>
  </div>

  <!-- SETTINGS MODAL & OTA -->
  <div id="settings-modal" class="modal">
    <div class="modal-content">
      <div class="flex justify-between items-center border-b mb-4 w-full" style="padding-bottom:10px;">
        <h3 class="text-sm font-semibold text-white">Smart Settings & OTA</h3>
        <button onclick="toggleModal('settings-modal')" class="text-slate-400 bg-transparent text-xl">&times;</button>
      </div>
      
      <form action="/save-wifi" method="POST" class="flex flex-col border-b pb-4 mb-4">
        <label class="text-xs text-slate-400 mb-2 font-bold">Wi-Fi Setup</label>
        <div class="text-[11px] mb-2 font-bold text-slate-400">Current Network: <span id="current-wifi" class="text-emerald-400">...</span></div>
        <div class="flex gap-2">
            <input type="text" name="ssid" placeholder="SSID" required style="width:50%;">
            <input type="password" name="pass" placeholder="Password" required style="width:50%;">
        </div>
        <button type="submit" class="btn bg-blue-600 text-white mt-2 w-full" style="background:#2563eb;">Save & Connect</button>
      </form>
      
      <div class="flex flex-col border-b pb-4 mb-4">
        <label class="text-xs text-slate-400 mb-2 font-bold">Voltage Limits & Calibration</label>
        <div class="grid grid-cols-3 gap-2 mb-2">
          <input type="number" step="0.1" id="set-low" placeholder="Low">
          <input type="number" step="0.1" id="set-res" placeholder="Resume">
          <input type="number" step="0.1" id="set-high" placeholder="High">
        </div>
        <div class="flex gap-2">
            <input type="number" step="0.01" id="calib-v" placeholder="Actual Multimeter Voltage" style="flex:1;">
            <button type="button" onclick="saveLimits()" class="btn bg-emerald-600 text-white" style="background:#059669;">Save Core</button>
        </div>
      </div>
      
      <form id="tg-form" class="flex flex-col border-b pb-4 mb-4">
        <div class="flex justify-between items-center mb-2">
            <label class="text-xs font-bold text-amber-400">Telegram Alerts Setup</label>
            <div class="flex items-center gap-2">
                <span class="text-xs text-slate-300 font-semibold">Alerts</span>
                <label class="switch">
                    <input type="checkbox" name="tg_enable" id="tg-enable" value="1">
                    <span class="slider"></span>
                </label>
            </div>
        </div>
        <input type="text" name="tg_token" id="tg-token" placeholder="Bot Token" class="mb-2">
        <input type="text" name="tg_chat" id="tg-chat" placeholder="Chat ID" class="mb-2">
        <button type="submit" class="btn text-white w-full" style="background:#4f46e5;">Save Telegram Config</button>
      </form>
      
      <form id="timer-form" class="flex flex-col border-b pb-4 mb-4">
        <label class="text-xs mb-2 font-bold text-emerald-400">Smart Charging Timer</label>
        <div class="grid grid-cols-2 gap-2 mb-2">
          <div><span class="text-[10px] text-slate-500">Start Time</span><input type="time" name="sch_start" id="sch-start"></div>
          <div><span class="text-[10px] text-slate-500">End Time</span><input type="time" name="sch_end" id="sch-end"></div>
        </div>
        <div class="grid grid-cols-2 gap-2">
            <button type="submit" class="btn text-white" style="background:#059669;">Save Timer</button>
            <button type="button" onclick="disableTimer()" class="btn bg-slate-700 text-white">Disable</button>
        </div>
      </form>

      <form id="ota-form" class="flex flex-col border-b pb-4 mb-4">
        <label class="text-xs font-bold text-amber-400 mb-2">OTA Firmware Update</label>
        <input type="file" id="ota-file" accept=".bin" required>
        <button type="submit" id="ota-btn" class="btn bg-blue-600 text-white mt-1 w-full" style="background:#2563eb;">Flash .bin Firmware</button>
      </form>

      <div class="flex flex-col w-full gap-2 pt-2 border-b pb-4 mb-4">
        <label class="text-xs text-slate-400 text-center font-semibold mb-1">System Maintenance</label>
        <div class="grid grid-cols-2 gap-2">
            <button type="button" onclick="clearLogs()" class="btn bg-slate-800 text-rose-400 border border-slate-700">Clear Logs</button>
            <button type="button" onclick="restartDevice()" class="btn bg-rose-600 text-white" style="background:#dc2626;">Restart Device</button>
        </div>
      </div>

      <!-- Author Credit & GitHub Link -->
      <div class="text-center pt-2 text-[11px] text-slate-500 flex flex-col items-center gap-1">
        <span>Developed by <a href="https://github.com/MukimBillahProdhan" target="_blank" class="text-blue-400 font-semibold" style="text-decoration:none;">Mukim Billah Prodhan</a></span>
        <span class="text-[10px] text-slate-600">AeonGrid Open Source &bull; CC BY-NC 4.0</span>
      </div>
    </div>
  </div>

  <script>
    let allOutages = [];
    let sessionUptimeSecs = 0; 
    let totalUpSecs = 0, totalDownSecs = 0;
    let systemBirthEpochGlobal = 0;
    
    function getLocalDateString(d) {
        return d.getFullYear() + "-" + String(d.getMonth()+1).padStart(2,'0') + "-" + String(d.getDate()).padStart(2,'0');
    }
    
    function format12Hour(dtStr) {
        if(!dtStr) return "--";
        if(typeof dtStr === 'string' && dtStr.startsWith("Offline")) return dtStr;
        let d = (dtStr instanceof Date) ? dtStr : new Date(dtStr.replace(' ', 'T'));
        if(isNaN(d)) return typeof dtStr === 'string' && dtStr.length > 10 ? dtStr.substring(11, 19) : dtStr;
        let hh = d.getHours(), mm = String(d.getMinutes()).padStart(2, '0'), ss = String(d.getSeconds()).padStart(2, '0');
        let ampm = hh >= 12 ? 'PM' : 'AM';
        hh = hh % 12 || 12;
        return `${String(hh).padStart(2, '0')}:${mm}:${ss} ${ampm}`;
    }
    
    let selectedDateFilter = getLocalDateString(new Date());
    let calRenderDate = new Date();
    let gridState = "ONLINE";
    let liveOutageStart = "";
    let liveOutageDuration = 0;
    let isBuzzerMuted = false;
    let isSettingsOpen = false;
    
    let isSysTimeOffline = true;
    let sysTimeObj = new Date();
    let prevGridState = "", prevRelayState = "", prevSoc = 100;

    function toggleTheme() {
        document.body.classList.toggle('light-theme');
        let isLight = document.body.classList.contains('light-theme');
        document.getElementById('theme-icon').innerText = isLight ? "🌙" : "☀️";
        localStorage.setItem('aeongrid_theme', isLight ? 'light' : 'dark');
        processDataTables();
    }
    if (localStorage.getItem('aeongrid_theme') === 'light') toggleTheme();
    
    function showToast(msg, type="info") {
        const container = document.getElementById('toast-container');
        const toast = document.createElement('div');
        toast.className = `toast ${type}`;
        toast.innerText = msg;
        container.appendChild(toast);
        setTimeout(() => { toast.remove(); }, 3500);
    }

    setInterval(() => {
        sessionUptimeSecs++;
        let d = Math.floor(sessionUptimeSecs / 86400);
        let upH = Math.floor((sessionUptimeSecs % 86400) / 3600);
        let upM = Math.floor((sessionUptimeSecs % 3600) / 60);
        document.getElementById('uptime-text').innerText = d > 0 ? `Up: ${d}d ${upH}h ${upM}m` : `Up: ${upH}h ${upM}m`;
        
        if(isSysTimeOffline) {
            document.getElementById('live-clock').innerText = "Offline";
            document.getElementById('live-clock').style.color = "var(--color-rose)";
        } else {
            sysTimeObj.setSeconds(sysTimeObj.getSeconds() + 1);
            let hh = sysTimeObj.getHours(), mm = String(sysTimeObj.getMinutes()).padStart(2, '0');
            let ampm = hh >= 12 ? 'PM' : 'AM';
            hh = hh % 12 || 12;
            document.getElementById('live-clock').innerText = `${hh}:${mm} ${ampm}`;
            document.getElementById('live-clock').style.color = "var(--text-main)";
            
            let todayStr = getLocalDateString(sysTimeObj);
            if (selectedDateFilter !== todayStr && document.getElementById('active-date-label').innerText === "(Today)") {
                selectedDateFilter = todayStr;
                document.getElementById('date-btn-text').innerText = todayStr;
                processDataTables();
            }
        }
    }, 1000);
    
    document.getElementById('tg-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        let fd = new FormData(e.target);
        try {
            let res = await fetch('/save-telegram', { method: 'POST', body: fd });
            if(res.ok) { showToast("✅ Telegram config saved!", "success"); toggleModal('settings-modal'); }
            else showToast("⚠️ Failed to save", "error");
        } catch(err) { showToast("⚠️ Network Error", "error"); }
    });

    document.getElementById('timer-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        let fd = new FormData(e.target);
        try {
            let res = await fetch('/save-timer', { method: 'POST', body: fd });
            if(res.ok) { showToast("✅ Smart Timer saved!", "success"); toggleModal('settings-modal'); }
            else showToast("⚠️ Failed to save", "error");
        } catch(err) { showToast("⚠️ Network Error", "error"); }
    });

    document.getElementById('ota-form').addEventListener('submit', async (e) => {
        e.preventDefault();
        let file = document.getElementById('ota-file').files[0];
        if(!file) return;
        let fd = new FormData(); fd.append("update", file);
        document.getElementById('ota-btn').innerText = "Flashing... Keep Powered!";
        try {
            let res = await fetch('/update', { method: 'POST', body: fd });
            if(res.ok) { showToast("✅ Firmware Flashed! Rebooting...", "success"); setTimeout(() => location.reload(), 6000); }
            else showToast("❌ Update Failed", "error");
        } catch(err) { showToast("❌ Network Error", "error"); }
        document.getElementById('ota-btn').innerText = "Flash .bin Firmware";
    });

    async function syncTime() {
        showToast("🔄 Syncing time...", "info");
        try {
            let res = await fetch('/api/sync_time', {method: 'POST'});
            let ans = await res.json();
            if(ans.success) showToast("✅ Sync Request Accepted!", "success");
            else showToast("⚠️ Sync Failed! (No Internet)", "error");
        } catch(e) { showToast("⚠️ Request Failed!", "error"); }
    }
    
    async function restartDevice() {
        if(confirm("Restart device?")) {
            try { await fetch('/api/restart', {method: 'POST'}); showToast("🔄 Restarting Device...", "warning"); setTimeout(()=>window.location.reload(), 3000); } catch(e){}
        }
    }

    function toggleModal(id) { 
        let el = document.getElementById(id);
        el.classList.toggle('show'); 
        if (id === 'settings-modal') isSettingsOpen = el.classList.contains('show');
    }

    function updateModeUI(mode, relayStatus) {
      const bA = document.getElementById('btn-auto'), bOn = document.getElementById('btn-on'), bOff = document.getElementById('btn-off');
      const slider = document.getElementById('mode-slider');
      const bg = document.getElementById('relay-status-badge');
      
      bA.classList.remove('active'); bOn.classList.remove('active'); bOff.classList.remove('active');
      
      if (mode === 0) { 
          bA.classList.add('active');
          slider.style.transform = 'translateX(0%)';
          slider.style.backgroundColor = '#2563eb';
          slider.style.boxShadow = '0 0 10px rgba(37,99,235,0.4)';
          bg.innerText = (relayStatus === "ON") ? "AUTO (ON)" : "AUTO (OFF)"; 
          bg.style.color = (relayStatus === "ON") ? "var(--color-emerald)" : "var(--text-sub)"; 
          bg.style.borderColor = (relayStatus === "ON") ? "var(--color-emerald)" : "var(--border-color)";
      }
      else if (mode === 1) { 
          bOn.classList.add('active'); 
          slider.style.transform = 'translateX(100%)';
          slider.style.backgroundColor = '#059669';
          slider.style.boxShadow = '0 0 10px rgba(5,150,105,0.4)';
          bg.innerText = "FORCE ON"; bg.style.color = "var(--color-emerald)"; bg.style.borderColor = "var(--color-emerald)"; 
      }
      else { 
          bOff.classList.add('active'); 
          slider.style.transform = 'translateX(200%)';
          slider.style.backgroundColor = '#dc2626';
          slider.style.boxShadow = '0 0 10px rgba(220,38,38,0.4)';
          bg.innerText = "FORCE OFF"; bg.style.color = "var(--color-rose)"; bg.style.borderColor = "var(--color-rose)"; 
      }
    }

    async function setChargerMode(mode) { try { await fetch('/api/mode?mode=' + mode, { method: 'POST' }); } catch(e) {} }
    
    async function toggleBuzzer() {
        isBuzzerMuted = !isBuzzerMuted;
        try { await fetch('/api/buzzer?mute=' + (isBuzzerMuted ? '1' : '0'), { method: 'POST' }); } catch(e) {}
        updateBuzzerIcon();
    }
    
    function updateBuzzerIcon() {
        const icon = document.getElementById('icon-buzzer');
        if(isBuzzerMuted) {
            icon.innerHTML = '<polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"></polygon><line x1="23" y1="9" x2="17" y2="15"></line><line x1="17" y1="9" x2="23" y2="15"></line>';
            icon.classList.replace('text-blue-400', 'text-rose-400');
        } else {
            icon.innerHTML = '<polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"></polygon><path d="M19.07 4.93a10 10 0 0 1 0 14.14M15.54 8.46a5 5 0 0 1 0 7.07"></path>';
            icon.classList.replace('text-rose-400', 'text-blue-400');
        }
    }

    async function saveLimits() {
      const l = document.getElementById('set-low').value, r = document.getElementById('set-res').value, h = document.getElementById('set-high').value, c = document.getElementById('calib-v').value;
      if(!l || !r || !h) return;
      try { 
          await fetch(`/api/limits?low=${l}&res=${r}&high=${h}`, { method: 'POST' }); 
          if(c) await fetch('/api/calibrate?v=' + c, { method: 'POST' });
          toggleModal('settings-modal'); showToast("✅ Core Limits Saved!", "success");
      } catch(e) {}
    }
    
    async function disableTimer() {
        try {
            await fetch('/api/disable_timer', { method: 'POST' });
            showToast("✅ Timer Disabled!", "success"); 
            toggleModal('settings-modal');
        } catch(e){}
    }

    async function clearLogs() {
      if(confirm("Clear all analytics and logs?")) {
        try { const res = await fetch('/api/clear-logs', { method: 'POST' }); if (res.ok) { showToast("✅ Cleared!", "success"); setTimeout(()=>window.location.reload(), 1500); } } catch(e) {}
      }
    }

    function drawDoughnut(canvasId, up, down) {
      const canvas = document.getElementById(canvasId); const ctx = canvas.getContext('2d');
      const total = up + down; const upAngle = total === 0 ? 0 : (up / total) * 2 * Math.PI;
      const c = canvas.width / 2; const r = c - 8;
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      ctx.beginPath(); ctx.arc(c, c, r, 0, 2 * Math.PI); ctx.strokeStyle = '#ef4444'; ctx.lineWidth = 10; ctx.stroke();
      if (up > 0) { ctx.beginPath(); ctx.arc(c, c, r, -Math.PI/2, upAngle - Math.PI/2); ctx.strokeStyle = '#10b981'; ctx.lineWidth = 10; ctx.lineCap = 'round'; ctx.stroke(); }
    }

    function openCalendar() { calRenderDate = new Date(selectedDateFilter + "T00:00:00"); renderCalendar(); toggleModal('date-modal'); }
    function changeMonth(dir) { calRenderDate.setMonth(calRenderDate.getMonth() + dir); renderCalendar(); }
    
    function renderCalendar() {
      const y = calRenderDate.getFullYear(), m = calRenderDate.getMonth();
      const mNames = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
      document.getElementById('cal-month-title').innerText = `${mNames[m]} ${y}`;
      const firstDay = new Date(y, m, 1).getDay(), days = new Date(y, m + 1, 0).getDate();
      const grid = document.getElementById('cal-days-grid'); grid.innerHTML = '';
      for (let i = 0; i < firstDay; i++) grid.innerHTML += `<span></span>`;
      for (let d = 1; d <= days; d++) {
        let dStr = `${y}-${String(m + 1).padStart(2, '0')}-${String(d).padStart(2, '0')}`;
        let sel = dStr === selectedDateFilter;
        let c = sel ? "background:#2563eb;color:white;border-radius:4px;font-weight:bold; box-shadow: 0 4px 10px rgba(37,99,235,0.4);" : "cursor:pointer;color:var(--text-main);background:var(--inner-bg);border-radius:4px;";
        grid.innerHTML += `<div onclick="selectDate('${dStr}')" class="py-1" style="${c}">${d}</div>`;
      }
    }

    function selectDate(dStr) {
      selectedDateFilter = dStr;
      let todayStr = isSysTimeOffline ? getLocalDateString(new Date()) : getLocalDateString(sysTimeObj);
      document.getElementById('active-date-label').innerText = (dStr === todayStr) ? "(Today)" : `(${dStr})`;
      document.getElementById('date-btn-text').innerText = dStr;
      toggleModal('date-modal'); processDataTables();
    }

    function formatDuration(s) {
      let h = Math.floor(s / 3600), m = Math.floor((s % 3600) / 60), sec = s % 60;
      return h > 0 ? `${h}h ${m.toString().padStart(2, '0')}m` : `${m.toString().padStart(2, '0')}m ${sec.toString().padStart(2, '0')}s`;
    }

    function renderHeatmapMatrix(todayObj) {
      const hm = document.getElementById('grid-heatmap');
      if(!hm) return;
      hm.innerHTML = '';
      
      let dayCutsMap = {};
      if(allOutages && Array.isArray(allOutages)) {
        allOutages.forEach(o => {
          if(!o.start || o.start.startsWith("Offline")) return;
          let dt = o.start.substring(0, 10);
          let hr = parseInt(o.start.substring(11, 13)) || 0;
          let blockIdx = Math.floor(hr / 4);
          if(!dayCutsMap[dt]) dayCutsMap[dt] = {};
          if(!dayCutsMap[dt][blockIdx]) dayCutsMap[dt][blockIdx] = 0;
          dayCutsMap[dt][blockIdx]++;
        });
      }

      for(let i = 6; i >= 0; i--) {
        let d = new Date(todayObj);
        d.setDate(d.getDate() - i);
        let dStr = getLocalDateString(d);
        let dayShort = d.toLocaleDateString('en-US', { weekday: 'narrow' });

        let colHtml = `<div class="heat-col"><span class="heat-head">${dayShort}</span>`;
        for(let b = 0; b < 6; b++) {
          let blockTime = new Date(d);
          blockTime.setHours(b * 4, 0, 0, 0);
          let blockEndTime = new Date(blockTime.getTime() + 4 * 3600 * 1000);
          let blockEndEpoch = Math.floor(blockEndTime.getTime() / 1000);
          
          if (blockTime > todayObj) {
              colHtml += `<div class="heat-block" style="background:transparent; border: 1px solid rgba(255,255,255,0.05);"></div>`;
          } 
          else if (systemBirthEpochGlobal > 0 && blockEndEpoch < systemBirthEpochGlobal) {
              colHtml += `<div class="heat-block" style="background:transparent; border: 1px solid rgba(255,255,255,0.05);"></div>`;
          }
          else {
              let cuts = (dayCutsMap[dStr] && dayCutsMap[dStr][b]) ? dayCutsMap[dStr][b] : 0;
              if (cuts === 0) colHtml += `<div class="heat-block heat-green"></div>`;
              else if (cuts <= 2) colHtml += `<div class="heat-block heat-amber"></div>`;
              else colHtml += `<div class="heat-block heat-red"></div>`;
          }
        }
        colHtml += `</div>`;
        hm.innerHTML += colHtml;
      }
    }

    function analyzeAI() {
      let todayObj = isSysTimeOffline ? new Date() : sysTimeObj; 
      let todayStr = getLocalDateString(todayObj);
      let dayCounts = {}; let hourCounts = {}; 
      
      let selectedDayCuts = (gridState === "OFFLINE" && selectedDateFilter === todayStr) ? 1 : 0;
      let totalCuts30d = 0;

      if(allOutages && Array.isArray(allOutages)) {
          allOutages.forEach(o => {
            if(o.start && o.start.startsWith("Offline")) return; 
            let dStr = o.start ? o.start.substring(0, 10) : ""; let hStr = parseInt(o.start ? o.start.substring(11, 13) : "0");
            
            if (dStr === selectedDateFilter) selectedDayCuts++;
            if(!dayCounts[dStr]) dayCounts[dStr] = 0; dayCounts[dStr] += o.dur_s;
            
            let diffDays = (todayObj - new Date(dStr)) / (1000 * 3600 * 24);
            if (diffDays <= 7 && !isNaN(hStr)) { if(!hourCounts[hStr]) hourCounts[hStr] = 0; hourCounts[hStr]++; }
            if (diffDays <= 30) totalCuts30d++;
          });
      }

      if (gridState === "OFFLINE") totalCuts30d++;

      let tdCuts = document.getElementById('today-cuts-text');
      tdCuts.innerText = `${selectedDayCuts} Cut${selectedDayCuts!==1?'s':''}`;
      if (selectedDayCuts > 0) {
          tdCuts.style.color = "var(--color-amber)";
          tdCuts.style.borderColor = "var(--color-amber)";
          tdCuts.style.boxShadow = "0 0 8px rgba(251,191,36,0.2)";
      } else {
          tdCuts.style.color = "var(--text-sub)";
          tdCuts.style.borderColor = "var(--border-color)";
          tdCuts.style.boxShadow = "none";
      }

      document.getElementById('total-cuts-text').innerText = totalCuts30d;

      let w7 = { d: '--', v: 0 }, w30 = { d: '--', v: 0 };
      for (let [date, dur] of Object.entries(dayCounts)) {
        let diffDays = (todayObj - new Date(date)) / (1000 * 3600 * 24);
        if (diffDays <= 7 && dur > w7.v) w7 = { d: date, v: dur };
        if (diffDays <= 30 && dur > w30.v) w30 = { d: date, v: dur };
      }
      document.getElementById('worst-7-text').innerText = w7.v > 0 ? `${(w7.d.length>5?w7.d.substring(5,10):'--')} (${formatDuration(w7.v)})` : 'None';
      document.getElementById('worst-30-text').innerText = w30.v > 0 ? `${(w30.d.length>5?w30.d.substring(5,10):'--')} (${formatDuration(w30.v)})` : 'None';

      let maxH = -1, maxC = 0;
      for (let [hr, c] of Object.entries(hourCounts)) { if (c > maxC) { maxC = c; maxH = parseInt(hr); } }
      if (maxH !== -1) {
        let ampm = maxH >= 12 ? 'PM' : 'AM'; let hr12 = maxH % 12 || 12;
        document.getElementById('trend-text').innerText = `Around ${hr12} ${ampm}`;
        document.getElementById('trend-text').style.color = "var(--color-amber)";
      } else { document.getElementById('trend-text').innerText = "Clear Pattern"; document.getElementById('trend-text').style.color = "var(--color-emerald)"; }
      
      renderHeatmapMatrix(todayObj);
    }

    function processDataTables() {
      const tb = document.getElementById('outage-log-body'); tb.innerHTML = '';
      let dayDownSecs = 0;
      let isToday = (selectedDateFilter === (isSysTimeOffline ? getLocalDateString(new Date()) : getLocalDateString(sysTimeObj)));
      
      if(isToday && gridState === "OFFLINE" && liveOutageStart !== "") {
          dayDownSecs += liveOutageDuration;
          tb.innerHTML += `<tr style="background: rgba(251,113,133,0.15);"><td style="color:var(--color-rose); font-weight:bold;">${format12Hour(liveOutageStart)}</td><td style="color:var(--text-sub);">-- Live Pending --</td><td style="text-align:right;color:var(--color-rose);font-weight:bold;">${formatDuration(liveOutageDuration)} <span class="animate-pulse">●</span></td></tr>`;
      }
      
      if(allOutages && Array.isArray(allOutages)) {
          let filteredLogs = allOutages.filter(o => o.start && (o.start.startsWith(selectedDateFilter) || (isToday && o.start.startsWith("Offline_"))));

          if (filteredLogs.length === 0 && tb.innerHTML === '') {
            tb.innerHTML = `<tr><td colspan="3" class="text-center text-slate-400">No power cuts logged.</td></tr>`;
          } else {
            [...filteredLogs].reverse().forEach(o => {
              dayDownSecs += o.dur_s;
              tb.innerHTML += `<tr><td class="text-slate-300 font-semibold">${format12Hour(o.start)}</td><td class="text-slate-300">${format12Hour(o.end)}</td><td class="text-right text-rose-400 font-bold">${formatDuration(o.dur_s)}</td></tr>`;
            });
          }
      }

      let dayTotalSecs = 86400; 
      if (isToday) { 
          let tObj = isSysTimeOffline ? new Date() : sysTimeObj;
          dayTotalSecs = (tObj.getHours() * 3600) + (tObj.getMinutes() * 60) + tObj.getSeconds(); 
      }
      let dayUpSecs = Math.max(0, dayTotalSecs - dayDownSecs);
      
      drawDoughnut('dailyCanvas', dayUpSecs, dayDownSecs);
      let dayPct = dayTotalSecs > 0 ? Math.round((dayUpSecs/dayTotalSecs)*100) : 0;
      document.getElementById('daily-center-text').innerText = `${dayPct}%`;
      document.getElementById('daily-text-stats').innerHTML = `<span class="text-emerald-400">${Math.floor(dayUpSecs/3600)}h ${Math.floor((dayUpSecs%3600)/60)}m</span> <span class="text-slate-600">/</span> <span class="text-rose-400">${Math.floor(dayDownSecs/3600)}h ${Math.floor((dayDownSecs%3600)/60)}m</span>`;

      let safeTotalUp = totalUpSecs > 0 ? totalUpSecs : 1;
      drawDoughnut('monthlyCanvas', safeTotalUp, totalDownSecs);
      let totalPct = ((safeTotalUp / (safeTotalUp + totalDownSecs)) * 100);
      document.getElementById('monthly-center-text').innerText = `${Math.round(totalPct)}%`;
      document.getElementById('monthly-text-stats').innerHTML = `<span class="text-emerald-400">${Math.floor(totalUpSecs/3600)}h ${Math.floor((totalUpSecs%3600)/60)}m</span> <span class="text-slate-600">/</span> <span class="text-rose-400">${Math.floor(totalDownSecs/3600)}h ${Math.floor((totalDownSecs%3600)/60)}m</span>`;
      
      let grade = "C (Poor)", gCol = "var(--color-rose)";
      if(totalPct >= 95) { grade = "A (Excellent)"; gCol = "var(--color-emerald)"; }
      else if (totalPct >= 80) { grade = "B (Moderate)"; gCol = "var(--color-amber)"; }
      let gb = document.getElementById('health-grade-text');
      gb.innerText = grade; gb.style.color = gCol; gb.style.borderColor = gCol; gb.style.boxShadow = `0 0 8px ${gCol}40`;
      
      document.getElementById('grid-health-quick').innerText = `${Math.round(totalPct)}% Up`;
      document.getElementById('total-uptime-text').innerText = `${Math.floor(totalUpSecs/3600)}h ${Math.floor((totalUpSecs%3600)/60)}m`;
      document.getElementById('total-downtime-text').innerText = `${Math.floor(totalDownSecs/3600)}h ${Math.floor((totalDownSecs%3600)/60)}m`;
      
      analyzeAI();
    }

    async function fetchLiveData() {
      try {
        const res = await fetch('/api/data'); if (!res.ok) return;
        const d = await res.json();
        
        sessionUptimeSecs = d.session_uptime || 0;
        totalUpSecs = d.total_uptime_s || 0;
        systemBirthEpochGlobal = d.birth_epoch || 0;
        
        let isSynced = d.time_synced;
        let incEpoch = parseInt(d.sys_epoch) || 0;
        
        if (!isSynced && incEpoch === 0) {
            isSysTimeOffline = true;
        } else {
            if(isSysTimeOffline && isSynced) {
                showToast("✅ System Time Synced!", "success");
                isSysTimeOffline = false;
            } else if (!isSynced && incEpoch > 0) {
                isSysTimeOffline = false;
            }
            let currInternalEpoch = Math.floor(sysTimeObj.getTime() / 1000);
            if (Math.abs(currInternalEpoch - incEpoch) > 2 && incEpoch > 0) {
                sysTimeObj = new Date(incEpoch * 1000);
            }
        }

        if (d.last_full_epoch && d.last_full_epoch > 0) {
            let lfcDate = new Date(d.last_full_epoch * 1000);
            document.getElementById('last-full-text').innerText = "Last full: " + format12Hour(lfcDate);
        } else {
            document.getElementById('last-full-text').innerText = "Last full: Not recorded";
        }

        if (d.grid && prevGridState !== "" && prevGridState !== d.grid) {
            if (d.grid === "OFFLINE") showToast("🔴 Grid Offline! Running on Battery", "error");
            else showToast("🟢 Grid Online! Power Restored", "success");
        }
        if (d.relay && prevRelayState !== "" && prevRelayState !== d.relay) {
            if (d.relay === "ON") showToast("⚡ Charging Started!", "info");
            else showToast("🛑 Charging Stopped!", "warning");
        }
        
        const v = parseFloat(d.voltage || 0), vL = parseFloat(d.l_low || 21.0), vH = parseFloat(d.l_high || 29.0);
        let soc = Math.max(0, Math.min(100, Math.round(((v - vL) / (vH - vL)) * 100))) || 0;
        
        if (soc <= 30 && prevSoc > 30) {
            showToast("⚠️ Battery Low (30%)", "error");
        }
        
        prevGridState = d.grid || prevGridState;
        prevRelayState = d.relay || prevRelayState;
        prevSoc = soc;

        let bCol = "var(--color-rose)"; 
        if(v > 25.0) bCol = "var(--color-emerald)"; else if (v > 22.0) bCol = "var(--color-amber)";
        
        document.getElementById('voltage-text').innerHTML = `<span style="color:${bCol}">${v.toFixed(1)}</span><span class="text-2xl text-slate-400">V</span>`;
        document.getElementById('soc-text').innerHTML = `${soc}<span class="text-xl">%</span>`;
        document.getElementById('soc-ring').style.strokeDasharray = `${soc}, 100`;
        document.getElementById('soc-ring').style.stroke = bCol;

        const socStatus = document.getElementById('soc-status');
        const isCharging = (d.relay === "ON");
        const cometRing = document.getElementById('soc-comet');

        if (isCharging) {
            socStatus.innerText = "CHARGING";
            socStatus.style.color = "var(--color-amber)";
            document.documentElement.style.setProperty('--comet-end', `-${soc}`);
            cometRing.style.display = 'block';
            cometRing.classList.add('comet-anim');
        } else {
            cometRing.style.display = 'none';
            cometRing.classList.remove('comet-anim');
            if (v >= vH) {
                socStatus.innerText = "FULL";
                socStatus.style.color = "var(--color-emerald)";
            } else if (soc <= 20) {
                socStatus.innerText = "CUT-OFF";
                socStatus.style.color = "var(--color-rose)";
            } else {
                socStatus.innerText = "DISCHARGING";
                socStatus.style.color = bCol;
            }
        }
        
        gridState = d.grid || "ONLINE";
        liveOutageStart = d.current_outage_start || "";
        liveOutageDuration = d.current_outage_s || 0;

        const gBg = document.getElementById('grid-status-badge');
        const liveCutInd = document.getElementById('live-cut-indicator');
        if (gridState === "ONLINE") { 
          gBg.innerHTML = 'ONLINE'; gBg.style.color = "var(--color-emerald)"; gBg.style.borderColor = "var(--color-emerald)"; gBg.style.boxShadow = "0 0 8px rgba(52,211,153,0.2)";
          liveCutInd.innerText = "Normal"; liveCutInd.style.color = "var(--color-emerald)";
        } else { 
          gBg.innerHTML = '<span class="animate-pulse">●</span> OFFLINE'; gBg.style.color = "var(--color-rose)"; gBg.style.borderColor = "var(--color-rose)"; gBg.style.boxShadow = "0 0 8px rgba(251,113,133,0.2)";
          liveCutInd.innerText = "Power Cut"; liveCutInd.style.color = "var(--color-rose)";
        }
        
        updateModeUI(d.mode || 0, d.relay);

        let eta = document.getElementById('eta-text');
        if (isCharging && v < vH) { eta.innerHTML = `~${((vH - v)*1.5).toFixed(1)}h Left`; eta.style.color="var(--color-amber)"; }
        else if (!isCharging && gridState==="OFFLINE" && v > vL) { eta.innerHTML = `~${((v - vL)*3.0).toFixed(1)}h Left`; eta.style.color="var(--color-rose)"; }
        else if (v >= vH) { eta.innerHTML = "Fully Charged"; eta.style.color="var(--color-emerald)"; }
        else { eta.innerHTML = "Standby Monitoring"; eta.style.color="var(--text-sub)"; }

        document.getElementById('lim-l').innerText = d.l_low || "--"; 
        document.getElementById('lim-r').innerText = d.l_res || "--"; 
        document.getElementById('lim-h').innerText = d.l_high || "--";
        
        document.getElementById('current-wifi').innerText = (d.connected_ssid && d.connected_ssid !== "Offline AP Mode") ? `${d.connected_ssid} 🟢` : "Offline AP Mode 🔴";

        if (!isSettingsOpen) {
            document.getElementById('tg-token').value = d.tg_token || "";
            document.getElementById('tg-chat').value = d.tg_chat || "";
            document.getElementById('tg-enable').checked = d.tg_enable;
            
            document.getElementById('sch-start').value = d.sch_start || "";
            document.getElementById('sch-end').value = d.sch_end || "";
            
            document.getElementById('set-low').value = d.l_low || "";
            document.getElementById('set-res').value = d.l_res || "";
            document.getElementById('set-high').value = d.l_high || "";
        }
        
        let schText = document.getElementById('schedule-indicator');
        if(d.sch_enable && d.sch_start && d.sch_end) { 
            schText.innerText = `Timer Active`; 
            schText.style.color = "var(--color-emerald)"; 
        } else { 
            schText.innerText = "Smart Timer Disabled"; 
            schText.style.color = "var(--text-sub)"; 
        }

        isBuzzerMuted = d.mute_buzzer || false;
        updateBuzzerIcon();

        totalDownSecs = d.downtime_s || 0;
        if (d.outages && JSON.stringify(allOutages) !== JSON.stringify(d.outages)) { allOutages = d.outages; }
        
        processDataTables();

      } catch(e) { console.log(e); }
    }

    document.getElementById('date-btn-text').innerText = selectedDateFilter;
    setInterval(fetchLiveData, 1500); fetchLiveData();
  </script>
</body>
</html>
)rawliteral";

#endif