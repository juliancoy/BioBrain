#pragma once

// Complete BioBrain web dashboard — served at GET /
// Single-page application with live visualization, controls, and parameter editing.
static const char* WEB_DASHBOARD_HTML = R"DASHBOARD(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>BioBrain Dashboard</title>
<style>
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }

:root {
    --bg: #0d0d1a;
    --panel: #1a1a2e;
    --panel-border: #2a2a4e;
    --text: #e0e0e0;
    --text-dim: #8888aa;
    --accent: #4af;
    --accent-hot: #f4a;
    --success: #4fa;
    --warning: #fa4;
    --error: #f44;
    --btn-bg: #2a2a4e;
    --btn-hover: #3a3a6e;
    --input-bg: #12122a;
    --scrollbar-bg: #1a1a2e;
    --scrollbar-thumb: #3a3a5e;
}

html, body {
    background: var(--bg);
    color: var(--text);
    font-family: 'SF Mono', 'Fira Code', 'Cascadia Code', 'Consolas', monospace;
    font-size: 13px;
    line-height: 1.5;
    overflow-x: hidden;
    min-height: 100vh;
}

::-webkit-scrollbar { width: 8px; height: 8px; }
::-webkit-scrollbar-track { background: var(--scrollbar-bg); }
::-webkit-scrollbar-thumb { background: var(--scrollbar-thumb); border-radius: 4px; }

/* ── Layout ────────────────────────────────────────────────── */

.app-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 12px 20px;
    background: var(--panel);
    border-bottom: 1px solid var(--panel-border);
    position: sticky;
    top: 0;
    z-index: 100;
}

.app-header h1 {
    font-size: 18px;
    font-weight: 700;
    color: var(--accent-hot);
    letter-spacing: 1px;
}

.app-header .status-badge {
    display: flex;
    align-items: center;
    gap: 8px;
    font-size: 12px;
}

.status-dot {
    width: 10px;
    height: 10px;
    border-radius: 50%;
    background: var(--error);
    animation: pulse 2s infinite;
}

.status-dot.running { background: var(--success); }
.status-dot.paused { background: var(--warning); }
.status-dot.stopped { background: var(--error); }
.status-dot.disconnected { background: #666; }

@keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.5; }
}

.dashboard {
    display: grid;
    grid-template-columns: 1fr 1fr;
    grid-template-rows: auto auto auto auto;
    gap: 12px;
    padding: 12px;
    max-width: 1800px;
    margin: 0 auto;
}

/* ── Panels ────────────────────────────────────────────────── */

.panel {
    background: var(--panel);
    border: 1px solid var(--panel-border);
    border-radius: 8px;
    overflow: hidden;
}

.panel-header {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 10px 14px;
    background: rgba(255,255,255,0.02);
    border-bottom: 1px solid var(--panel-border);
    font-size: 12px;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 1px;
    color: var(--text-dim);
}

.panel-body {
    padding: 14px;
}

.panel.full-width {
    grid-column: 1 / -1;
}

/* ── Tabs ──────────────────────────────────────────────────── */

.tab-bar {
    display: flex;
    gap: 0;
    border-bottom: 1px solid var(--panel-border);
    background: rgba(0,0,0,0.2);
}

.tab-btn {
    padding: 8px 16px;
    background: none;
    border: none;
    color: var(--text-dim);
    font-family: inherit;
    font-size: 12px;
    cursor: pointer;
    border-bottom: 2px solid transparent;
    transition: all 0.2s;
}

.tab-btn:hover { color: var(--text); background: rgba(255,255,255,0.03); }
.tab-btn.active { color: var(--accent); border-bottom-color: var(--accent); }

.tab-content { display: none; }
.tab-content.active { display: block; }

/* ── Buttons ───────────────────────────────────────────────── */

button, .btn {
    background: var(--btn-bg);
    color: var(--text);
    border: 1px solid var(--panel-border);
    padding: 6px 14px;
    border-radius: 4px;
    font-family: inherit;
    font-size: 12px;
    cursor: pointer;
    transition: all 0.15s;
    white-space: nowrap;
}

button:hover, .btn:hover {
    background: var(--btn-hover);
    border-color: var(--accent);
}

button:active { transform: scale(0.97); }

button.primary {
    background: #1a3a5e;
    border-color: var(--accent);
    color: var(--accent);
}

button.success { border-color: var(--success); color: var(--success); }
button.warning { border-color: var(--warning); color: var(--warning); }
button.danger { border-color: var(--error); color: var(--error); }

button:disabled {
    opacity: 0.4;
    cursor: not-allowed;
}

.btn-group {
    display: flex;
    gap: 6px;
    flex-wrap: wrap;
}

/* ── Inputs ────────────────────────────────────────────────── */

select, input[type="number"], input[type="text"] {
    background: var(--input-bg);
    color: var(--text);
    border: 1px solid var(--panel-border);
    padding: 5px 8px;
    border-radius: 4px;
    font-family: inherit;
    font-size: 12px;
    outline: none;
}

select:focus, input:focus {
    border-color: var(--accent);
}

input[type="range"] {
    -webkit-appearance: none;
    appearance: none;
    background: var(--input-bg);
    height: 6px;
    border-radius: 3px;
    outline: none;
}

input[type="range"]::-webkit-slider-thumb {
    -webkit-appearance: none;
    width: 16px;
    height: 16px;
    border-radius: 50%;
    background: var(--accent);
    cursor: pointer;
}

label {
    font-size: 11px;
    color: var(--text-dim);
    display: block;
    margin-bottom: 3px;
}

/* ── Status Grid ───────────────────────────────────────────── */

.stat-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(140px, 1fr));
    gap: 10px;
}

.stat-item {
    background: rgba(0,0,0,0.25);
    border-radius: 6px;
    padding: 10px 12px;
    border-left: 3px solid var(--accent);
}

.stat-label {
    font-size: 10px;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.5px;
    margin-bottom: 2px;
}

.stat-value {
    font-size: 20px;
    font-weight: 700;
    color: var(--text);
}

.stat-value.small { font-size: 14px; }

/* ── Activity Bars ─────────────────────────────────────────── */

.activity-bar-container {
    margin: 6px 0;
}

.activity-bar-label {
    display: flex;
    justify-content: space-between;
    font-size: 11px;
    margin-bottom: 2px;
}

.activity-bar-label .region-name { color: var(--text); font-weight: 600; }
.activity-bar-label .region-rate { color: var(--text-dim); }

.activity-bar-track {
    height: 18px;
    background: rgba(0,0,0,0.3);
    border-radius: 3px;
    overflow: hidden;
    position: relative;
}

.activity-bar-fill {
    height: 100%;
    border-radius: 3px;
    transition: width 0.5s ease;
    min-width: 2px;
}

/* Region colors matching Qt palette */
.region-color-0 { background: #e74c3c; } /* Retina - red */
.region-color-1 { background: #e67e22; } /* LGN - orange */
.region-color-2 { background: #f1c40f; } /* V1 - yellow */
.region-color-3 { background: #2ecc71; } /* V2/V4 - green */
.region-color-4 { background: #3498db; } /* IT - blue */
.region-color-5 { background: #9b59b6; } /* VTA - purple */
.region-color-6 { background: #e91e9a; } /* Striatum - pink */
.region-color-7 { background: #1abc9c; } /* Motor - cyan */
.region-color-8 { background: #d4a053; } /* Wernicke - gold */
.region-color-9 { background: #5dade2; } /* Broca - light blue */

/* ── Region Table ──────────────────────────────────────────── */

.region-table {
    width: 100%;
    border-collapse: collapse;
    font-size: 12px;
}

.region-table th {
    text-align: left;
    padding: 8px 10px;
    font-weight: 600;
    color: var(--text-dim);
    border-bottom: 1px solid var(--panel-border);
    font-size: 10px;
    text-transform: uppercase;
    letter-spacing: 0.5px;
}

.region-table td {
    padding: 6px 10px;
    border-bottom: 1px solid rgba(255,255,255,0.04);
}

.region-table tr:hover td {
    background: rgba(255,255,255,0.03);
}

.region-table .color-dot {
    display: inline-block;
    width: 8px;
    height: 8px;
    border-radius: 50%;
    margin-right: 6px;
    vertical-align: middle;
}

/* ── Spike Log ─────────────────────────────────────────────── */

.spike-log {
    height: 200px;
    overflow-y: auto;
    font-size: 11px;
    background: var(--input-bg);
    border-radius: 4px;
    padding: 8px;
}

.spike-entry {
    display: flex;
    gap: 12px;
    padding: 2px 0;
    border-bottom: 1px solid rgba(255,255,255,0.02);
}

.spike-entry .spike-time { color: var(--text-dim); min-width: 80px; }
.spike-entry .spike-region { min-width: 100px; font-weight: 600; }
.spike-entry .spike-count { color: var(--accent); }

/* ── Control Rows ──────────────────────────────────────────── */

.control-row {
    display: flex;
    align-items: center;
    gap: 10px;
    margin: 8px 0;
    flex-wrap: wrap;
}

.control-row label {
    margin-bottom: 0;
    min-width: 80px;
}

/* ── Parameter Editor ──────────────────────────────────────── */

.param-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
}

.param-card {
    background: rgba(0,0,0,0.2);
    border-radius: 6px;
    padding: 12px;
    border: 1px solid rgba(255,255,255,0.05);
}

.param-card h4 {
    font-size: 12px;
    font-weight: 600;
    margin-bottom: 10px;
    display: flex;
    align-items: center;
    gap: 6px;
}

.param-card .field {
    margin: 6px 0;
}

.param-card select, .param-card input {
    width: 100%;
}

.synapse-toggles {
    display: flex;
    gap: 8px;
    flex-wrap: wrap;
}

.synapse-toggle {
    display: flex;
    align-items: center;
    gap: 4px;
    padding: 4px 10px;
    background: var(--input-bg);
    border: 1px solid var(--panel-border);
    border-radius: 4px;
    cursor: pointer;
    font-size: 11px;
    transition: all 0.15s;
    user-select: none;
}

.synapse-toggle.active {
    border-color: var(--accent);
    background: rgba(68, 170, 255, 0.1);
    color: var(--accent);
}

/* ── Debug Panel ───────────────────────────────────────────── */

.debug-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
}

.debug-section {
    background: rgba(0,0,0,0.2);
    border-radius: 6px;
    padding: 12px;
}

.debug-section h4 {
    font-size: 11px;
    color: var(--text-dim);
    text-transform: uppercase;
    letter-spacing: 0.5px;
    margin-bottom: 8px;
    border-bottom: 1px solid rgba(255,255,255,0.06);
    padding-bottom: 6px;
}

.debug-row {
    display: flex;
    justify-content: space-between;
    padding: 3px 0;
    font-size: 12px;
}

.debug-row .debug-key { color: var(--text-dim); }
.debug-row .debug-val { color: var(--text); font-weight: 600; }

.trace-output {
    background: var(--input-bg);
    border-radius: 4px;
    padding: 8px;
    font-size: 11px;
    max-height: 250px;
    overflow-y: auto;
    white-space: pre-wrap;
    word-break: break-all;
    color: var(--text-dim);
}

/* ── Screenshot ────────────────────────────────────────────── */

.screenshot-container {
    text-align: center;
    padding: 10px;
}

.screenshot-container img {
    max-width: 100%;
    border-radius: 6px;
    border: 1px solid var(--panel-border);
}

.screenshot-placeholder {
    padding: 40px;
    color: var(--text-dim);
    font-size: 12px;
}

/* ── Hardware Panel ────────────────────────────────────────── */

.hw-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(120px, 1fr));
    gap: 8px;
}

.hw-item {
    text-align: center;
    padding: 8px;
    background: rgba(0,0,0,0.2);
    border-radius: 6px;
}

.hw-item .hw-val {
    font-size: 16px;
    font-weight: 700;
    color: var(--accent);
}

.hw-item .hw-label {
    font-size: 10px;
    color: var(--text-dim);
    margin-top: 2px;
}

/* ── Toast ─────────────────────────────────────────────────── */

.toast-container {
    position: fixed;
    bottom: 20px;
    right: 20px;
    z-index: 1000;
    display: flex;
    flex-direction: column;
    gap: 8px;
}

.toast {
    padding: 10px 16px;
    background: var(--panel);
    border: 1px solid var(--panel-border);
    border-radius: 6px;
    font-size: 12px;
    animation: toastIn 0.3s ease;
    max-width: 340px;
}

.toast.error { border-color: var(--error); color: var(--error); }
.toast.success { border-color: var(--success); color: var(--success); }
.toast.info { border-color: var(--accent); color: var(--accent); }

@keyframes toastIn {
    from { opacity: 0; transform: translateY(20px); }
    to { opacity: 1; transform: translateY(0); }
}

/* ── Disconnected Overlay ──────────────────────────────────── */

.disconnect-overlay {
    display: none;
    position: fixed;
    top: 0; left: 0; right: 0; bottom: 0;
    background: rgba(13,13,26,0.85);
    z-index: 200;
    align-items: center;
    justify-content: center;
    flex-direction: column;
    gap: 12px;
}

.disconnect-overlay.visible { display: flex; }

.disconnect-overlay .disc-icon {
    font-size: 48px;
    color: var(--error);
}

.disconnect-overlay .disc-text {
    font-size: 18px;
    font-weight: 700;
    color: var(--error);
}

.disconnect-overlay .disc-sub {
    font-size: 12px;
    color: var(--text-dim);
}

/* ── Responsive ────────────────────────────────────────────── */

@media (max-width: 900px) {
    .dashboard {
        grid-template-columns: 1fr;
    }
    .panel.full-width { grid-column: 1; }
    .param-grid { grid-template-columns: 1fr; }
    .debug-grid { grid-template-columns: 1fr; }
    .stat-grid { grid-template-columns: repeat(2, 1fr); }
}

@media (max-width: 500px) {
    .app-header { padding: 8px 12px; }
    .app-header h1 { font-size: 14px; }
    .dashboard { padding: 8px; gap: 8px; }
    .stat-grid { grid-template-columns: 1fr 1fr; }
    .stat-value { font-size: 16px; }
}
</style>
</head>
<body>

<!-- Header -->
<header class="app-header">
    <h1>BIOBRAIN</h1>
    <div class="status-badge">
        <span id="statusText">Connecting...</span>
        <span class="status-dot disconnected" id="statusDot"></span>
    </div>
</header>

<!-- Disconnected Overlay -->
<div class="disconnect-overlay" id="disconnectOverlay">
    <div class="disc-icon">&#x26A0;</div>
    <div class="disc-text">Disconnected</div>
    <div class="disc-sub">Cannot reach BioBrain server. Retrying...</div>
</div>

<!-- Main Dashboard -->
<div class="dashboard">

    <!-- ═══ Status Panel ═══ -->
    <div class="panel">
        <div class="panel-header">Simulation Status</div>
        <div class="panel-body">
            <div class="stat-grid" id="statusGrid">
                <div class="stat-item" style="border-color: var(--accent-hot)">
                    <div class="stat-label">Sim Time</div>
                    <div class="stat-value small" id="simTime">--</div>
                </div>
                <div class="stat-item" style="border-color: var(--success)">
                    <div class="stat-label">Wall Time</div>
                    <div class="stat-value small" id="wallTime">--</div>
                </div>
                <div class="stat-item" style="border-color: var(--warning)">
                    <div class="stat-label">Speed Ratio</div>
                    <div class="stat-value" id="speedRatio">--</div>
                </div>
                <div class="stat-item" style="border-color: var(--accent)">
                    <div class="stat-label">Spikes/sec</div>
                    <div class="stat-value" id="spikesPerSec">--</div>
                </div>
                <div class="stat-item" style="border-color: #9b59b6">
                    <div class="stat-label">Active Neurons</div>
                    <div class="stat-value" id="activeNeurons">--</div>
                </div>
                <div class="stat-item" style="border-color: #1abc9c">
                    <div class="stat-label">Regions</div>
                    <div class="stat-value" id="regionCount">--</div>
                </div>
            </div>
        </div>
    </div>

    <!-- ═══ Hardware Panel ═══ -->
    <div class="panel">
        <div class="panel-header">Hardware Profile</div>
        <div class="panel-body">
            <div class="hw-grid" id="hwGrid">
                <div class="hw-item"><div class="hw-val" id="hwTier">--</div><div class="hw-label">Tier</div></div>
                <div class="hw-item"><div class="hw-val" id="hwCPU">--</div><div class="hw-label">CPU Cores</div></div>
                <div class="hw-item"><div class="hw-val" id="hwRAM">--</div><div class="hw-label">RAM (GB)</div></div>
                <div class="hw-item"><div class="hw-val" id="hwGPU">--</div><div class="hw-label">GPU</div></div>
                <div class="hw-item"><div class="hw-val" id="hwNeurons">--</div><div class="hw-label">Total Neurons</div></div>
            </div>
        </div>
    </div>

    <!-- ═══ Activity Bars ═══ -->
    <div class="panel full-width">
        <div class="panel-header">Brain Region Activity</div>
        <div class="panel-body" id="activityBars">
            <div style="color: var(--text-dim); font-size: 12px;">Waiting for data...</div>
        </div>
    </div>

    <!-- ═══ Controls Panel ═══ -->
    <div class="panel">
        <div class="panel-header">Controls</div>
        <div class="panel-body">
            <div class="control-row">
                <label>Simulation</label>
                <div class="btn-group">
                    <button class="success" onclick="simAction('start')">Start</button>
                    <button class="warning" onclick="simAction('pause')">Pause</button>
                    <button class="primary" onclick="simAction('resume')">Resume</button>
                    <button class="danger" onclick="simAction('stop')">Stop</button>
                </div>
            </div>
            <div class="control-row">
                <label>Management</label>
                <div class="btn-group">
                    <button onclick="simAction('rebuild')">Rebuild Brain</button>
                    <button onclick="simAction('restart')">Restart Sim</button>
                </div>
            </div>
            <div class="control-row">
                <label>Inject Spikes</label>
                <select id="injectRegion" style="width: 130px;"></select>
                <input type="number" id="injectCount" value="100" min="1" max="10000" style="width: 80px;">
                <button class="primary" onclick="injectSpikes()">Inject</button>
            </div>
            <div class="control-row">
                <label>Camera</label>
                <select id="cameraSelect" style="width: 200px;" onchange="switchCamera()">
                    <option value="">Loading...</option>
                </select>
            </div>
            <div class="control-row">
                <label>Screenshot</label>
                <button onclick="takeScreenshot()">Capture</button>
            </div>
            <div class="screenshot-container" id="screenshotContainer">
                <div class="screenshot-placeholder">No screenshot captured</div>
            </div>
        </div>
    </div>

    <!-- ═══ Tabbed Panel: Table / Spikes / Params / Debug ═══ -->
    <div class="panel">
        <div class="tab-bar">
            <button class="tab-btn active" onclick="switchTab(this, 'tabRegions')">Regions</button>
            <button class="tab-btn" onclick="switchTab(this, 'tabSpikes')">Spikes</button>
            <button class="tab-btn" onclick="switchTab(this, 'tabParams')">Parameters</button>
            <button class="tab-btn" onclick="switchTab(this, 'tabDebug')">Debug</button>
        </div>

        <!-- Region Table Tab -->
        <div class="tab-content active" id="tabRegions">
            <div class="panel-body" style="overflow-x: auto;">
                <table class="region-table" id="regionTable">
                    <thead>
                        <tr>
                            <th>Region</th>
                            <th>Neurons</th>
                            <th>Firing Rate</th>
                            <th>Active</th>
                            <th>Backend</th>
                        </tr>
                    </thead>
                    <tbody id="regionTableBody">
                        <tr><td colspan="5" style="color: var(--text-dim)">Loading...</td></tr>
                    </tbody>
                </table>
            </div>
        </div>

        <!-- Spike Log Tab -->
        <div class="tab-content" id="tabSpikes">
            <div class="panel-body">
                <div class="spike-log" id="spikeLog">
                    <div style="color: var(--text-dim)">Waiting for spike data...</div>
                </div>
            </div>
        </div>

        <!-- Parameters Tab -->
        <div class="tab-content" id="tabParams">
            <div class="panel-body">
                <div class="control-row" style="margin-bottom: 12px;">
                    <label>Region</label>
                    <select id="paramRegionSelect" style="width: 200px;" onchange="loadRegionParams()"></select>
                    <button class="primary" onclick="submitRegionConfig()">Apply Changes</button>
                </div>
                <div class="param-grid">
                    <div class="param-card">
                        <h4>Neuron Model</h4>
                        <div class="field">
                            <select id="paramModel">
                                <option value="Izhikevich">Izhikevich</option>
                                <option value="HH">Hodgkin-Huxley</option>
                                <option value="AdEx">AdEx</option>
                                <option value="LIF">LIF</option>
                            </select>
                        </div>
                    </div>
                    <div class="param-card">
                        <h4>Compute Backend</h4>
                        <div class="field">
                            <select id="paramBackend">
                                <option value="CPU">CPU</option>
                                <option value="Metal">Metal</option>
                                <option value="CUDA">CUDA</option>
                            </select>
                        </div>
                    </div>
                    <div class="param-card">
                        <h4>Plasticity Rule</h4>
                        <div class="field">
                            <select id="paramPlasticity">
                                <option value="STDP">STDP</option>
                                <option value="DopamineSTDP">Dopamine STDP</option>
                                <option value="NeuromodulatorySTDP">Neuromodulatory STDP</option>
                                <option value="None">None</option>
                            </select>
                        </div>
                    </div>
                    <div class="param-card">
                        <h4>Synapse Types</h4>
                        <div class="synapse-toggles" id="synapseToggles">
                            <div class="synapse-toggle active" data-type="AMPA" onclick="toggleSynapse(this)">AMPA</div>
                            <div class="synapse-toggle active" data-type="NMDA" onclick="toggleSynapse(this)">NMDA</div>
                            <div class="synapse-toggle active" data-type="GABA-A" onclick="toggleSynapse(this)">GABA-A</div>
                            <div class="synapse-toggle active" data-type="GABA-B" onclick="toggleSynapse(this)">GABA-B</div>
                        </div>
                    </div>
                </div>
                <div class="control-row" style="margin-top: 14px;">
                    <label>Vocal Volume</label>
                    <input type="range" id="volumeSlider" min="0" max="100" value="75" style="flex: 1;">
                    <span id="volumeValue" style="min-width: 40px; text-align: right;">75%</span>
                </div>
            </div>
        </div>

        <!-- Debug Tab -->
        <div class="tab-content" id="tabDebug">
            <div class="panel-body">
                <div class="debug-grid">
                    <div class="debug-section">
                        <h4>Propagation Counters</h4>
                        <div id="debugCounters">
                            <div class="debug-row"><span class="debug-key">Loading...</span></div>
                        </div>
                    </div>
                    <div class="debug-section">
                        <h4>Memory Usage</h4>
                        <div id="debugMemory">
                            <div class="debug-row"><span class="debug-key">Loading...</span></div>
                        </div>
                    </div>
                    <div class="debug-section">
                        <h4>Profiling Stats</h4>
                        <div id="debugProfile">
                            <div class="debug-row"><span class="debug-key">Loading...</span></div>
                        </div>
                    </div>
                    <div class="debug-section">
                        <h4>Projection Wiring Trace</h4>
                        <div class="trace-output" id="debugTrace">Loading...</div>
                    </div>
                </div>
            </div>
        </div>
    </div>

</div>

<!-- Toast Container -->
<div class="toast-container" id="toastContainer"></div>

<script>
// ═══════════════════════════════════════════════════════════════
//  BioBrain Dashboard — Client-side Application
// ═══════════════════════════════════════════════════════════════

const REGION_COLORS = [
    '#e74c3c', '#e67e22', '#f1c40f', '#2ecc71', '#3498db',
    '#9b59b6', '#e91e9a', '#1abc9c', '#d4a053', '#5dade2'
];

const REGION_NAMES_FALLBACK = [
    'Retina', 'LGN', 'V1', 'V2/V4', 'IT',
    'VTA', 'Striatum', 'Motor', 'Wernicke', 'Broca'
];

let connected = false;
let regions = [];
let simStatus = {};
let refreshTimer = null;
let failCount = 0;

// ── Utility ───────────────────────────────────────────────────

function fmt(n, digits) {
    if (n === undefined || n === null) return '--';
    if (typeof n === 'number') {
        if (digits !== undefined) return n.toFixed(digits);
        if (Math.abs(n) >= 1000) return n.toLocaleString('en-US', { maximumFractionDigits: 1 });
        if (Math.abs(n) < 0.01 && n !== 0) return n.toExponential(2);
        return n.toFixed(2);
    }
    return String(n);
}

function fmtTime(ms) {
    if (ms === undefined || ms === null) return '--';
    var sec = ms / 1000;
    if (sec < 60) return sec.toFixed(1) + 's';
    if (sec < 3600) return (sec / 60).toFixed(1) + 'm';
    return (sec / 3600).toFixed(2) + 'h';
}

function fmtWall(sec) {
    if (sec === undefined || sec === null) return '--';
    if (sec < 60) return sec.toFixed(0) + 's';
    if (sec < 3600) return Math.floor(sec / 60) + 'm ' + (sec % 60).toFixed(0) + 's';
    var h = Math.floor(sec / 3600);
    var m = Math.floor((sec % 3600) / 60);
    return h + 'h ' + m + 'm';
}

function regionColor(idx) {
    return REGION_COLORS[idx % REGION_COLORS.length];
}

function regionName(id) {
    for (var i = 0; i < regions.length; i++) {
        if (regions[i].id === id) return regions[i].name;
    }
    return REGION_NAMES_FALLBACK[id] || 'Region ' + id;
}

// ── Toast Notifications ───────────────────────────────────────

function toast(msg, type) {
    type = type || 'info';
    var el = document.createElement('div');
    el.className = 'toast ' + type;
    el.textContent = msg;
    document.getElementById('toastContainer').appendChild(el);
    setTimeout(function() { el.remove(); }, 4000);
}

// ── Tab Switching ─────────────────────────────────────────────

function switchTab(btn, tabId) {
    var parent = btn.closest('.panel');
    var tabs = parent.querySelectorAll('.tab-btn');
    var contents = parent.querySelectorAll('.tab-content');
    for (var i = 0; i < tabs.length; i++) tabs[i].classList.remove('active');
    for (var i = 0; i < contents.length; i++) contents[i].classList.remove('active');
    btn.classList.add('active');
    document.getElementById(tabId).classList.add('active');
}

// ── Synapse Toggle ────────────────────────────────────────────

function toggleSynapse(el) {
    el.classList.toggle('active');
}

// ── Volume Slider ─────────────────────────────────────────────

document.getElementById('volumeSlider').addEventListener('input', function() {
    document.getElementById('volumeValue').textContent = this.value + '%';
});

// ── API Fetch Helper ──────────────────────────────────────────

function api(url, opts) {
    opts = opts || {};
    return fetch(url, opts).then(function(res) {
        if (!res.ok) throw new Error('HTTP ' + res.status);
        var ct = res.headers.get('content-type') || '';
        if (ct.indexOf('json') >= 0) return res.json();
        return res.text();
    });
}

function apiPost(url, body) {
    var opts = { method: 'POST' };
    if (body) {
        opts.headers = { 'Content-Type': 'application/json' };
        opts.body = JSON.stringify(body);
    }
    return api(url, opts);
}

// ── Connection Status ─────────────────────────────────────────

function setConnected(state) {
    connected = state;
    var overlay = document.getElementById('disconnectOverlay');
    var dot = document.getElementById('statusDot');
    if (!state) {
        overlay.classList.add('visible');
        dot.className = 'status-dot disconnected';
        document.getElementById('statusText').textContent = 'Disconnected';
    } else {
        overlay.classList.remove('visible');
        failCount = 0;
    }
}

function updateStatusDisplay() {
    var dot = document.getElementById('statusDot');
    var txt = document.getElementById('statusText');
    if (!connected) return;
    if (simStatus.running && !simStatus.paused) {
        dot.className = 'status-dot running';
        txt.textContent = 'Running';
    } else if (simStatus.paused) {
        dot.className = 'status-dot paused';
        txt.textContent = 'Paused';
    } else {
        dot.className = 'status-dot stopped';
        txt.textContent = 'Stopped';
    }
}

// ── Simulation Controls ───────────────────────────────────────

function simAction(action) {
    var url = '/api/sim/' + action;
    apiPost(url).then(function(data) {
        toast(action.charAt(0).toUpperCase() + action.slice(1) + ' OK', 'success');
    }).catch(function(err) {
        toast('Failed: ' + err.message, 'error');
    });
}

function injectSpikes() {
    var regionId = document.getElementById('injectRegion').value;
    var count = document.getElementById('injectCount').value || 100;
    var url = '/api/sim/inject/region?id=' + regionId + '&count=' + count;
    apiPost(url).then(function(data) {
        toast('Injected ' + count + ' spikes', 'success');
    }).catch(function(err) {
        toast('Inject failed: ' + err.message, 'error');
    });
}

function switchCamera() {
    var sel = document.getElementById('cameraSelect');
    if (!sel.value) return;
    apiPost('/api/webcam/switch?id=' + encodeURIComponent(sel.value)).then(function() {
        toast('Camera switched', 'success');
    }).catch(function(err) {
        toast('Camera switch failed: ' + err.message, 'error');
    });
}

function takeScreenshot() {
    api('/api/screenshot').then(function(data) {
        var container = document.getElementById('screenshotContainer');
        if (data && data.base64) {
            var ext = (data.format || 'png').toLowerCase();
            container.innerHTML = '<img src="data:image/' + ext + ';base64,' + data.base64 + '" alt="Screenshot">';
        } else {
            container.innerHTML = '<div class="screenshot-placeholder">No screenshot available</div>';
        }
    }).catch(function(err) {
        toast('Screenshot failed: ' + err.message, 'error');
    });
}

// ── Region Config Submission ──────────────────────────────────

function submitRegionConfig() {
    var regionId = document.getElementById('paramRegionSelect').value;
    if (!regionId && regionId !== 0) {
        toast('Select a region first', 'error');
        return;
    }

    var synapses = [];
    var toggles = document.querySelectorAll('#synapseToggles .synapse-toggle.active');
    for (var i = 0; i < toggles.length; i++) {
        synapses.push(toggles[i].getAttribute('data-type'));
    }

    var body = {
        region_id: parseInt(regionId),
        neuron_model: document.getElementById('paramModel').value,
        backend: document.getElementById('paramBackend').value,
        plasticity: document.getElementById('paramPlasticity').value,
        synapse_types: synapses,
        vocal_volume: parseInt(document.getElementById('volumeSlider').value) / 100.0
    };

    apiPost('/api/config/region', body).then(function(data) {
        toast('Config applied to region ' + regionName(parseInt(regionId)), 'success');
    }).catch(function(err) {
        toast('Config failed: ' + err.message, 'error');
    });
}

function loadRegionParams() {
    // Pre-fill from known region data if available
    var regionId = parseInt(document.getElementById('paramRegionSelect').value);
    for (var i = 0; i < regions.length; i++) {
        if (regions[i].id === regionId) {
            var r = regions[i];
            // Attempt to match model name
            var modelSel = document.getElementById('paramModel');
            if (r.model) {
                for (var j = 0; j < modelSel.options.length; j++) {
                    if (r.model.indexOf(modelSel.options[j].value) >= 0 ||
                        modelSel.options[j].text.indexOf(r.model) >= 0) {
                        modelSel.selectedIndex = j;
                        break;
                    }
                }
            }
            // Attempt to match backend
            var backSel = document.getElementById('paramBackend');
            if (r.backend) {
                for (var j = 0; j < backSel.options.length; j++) {
                    if (r.backend.toLowerCase().indexOf(backSel.options[j].value.toLowerCase()) >= 0) {
                        backSel.selectedIndex = j;
                        break;
                    }
                }
            }
            break;
        }
    }
}

// ── Data Refresh Functions ────────────────────────────────────

function refreshStatus() {
    return api('/api/sim/status').then(function(data) {
        simStatus = data;
        setConnected(true);
        updateStatusDisplay();

        document.getElementById('simTime').textContent = fmtTime(data.sim_time_ms);
        document.getElementById('wallTime').textContent = fmtWall(data.uptime_sec);
        document.getElementById('spikesPerSec').textContent = fmt(data.spikes_per_second, 0);
        document.getElementById('activeNeurons').textContent = fmt(data.total_active_neurons);
        document.getElementById('regionCount').textContent = data.region_count || '--';

        // Speed ratio from profile if available
    }).catch(function(err) {
        failCount++;
        if (failCount >= 3) setConnected(false);
    });
}

function refreshRegions() {
    return api('/api/regions').then(function(data) {
        if (!Array.isArray(data)) return;
        regions = data;

        // Update activity bars
        var maxRate = 1;
        for (var i = 0; i < data.length; i++) {
            if (data[i].firing_rate_Hz > maxRate) maxRate = data[i].firing_rate_Hz;
        }
        var barScale = Math.max(maxRate * 1.2, 10);

        var html = '';
        for (var i = 0; i < data.length; i++) {
            var r = data[i];
            var pct = Math.min((r.firing_rate_Hz / barScale) * 100, 100);
            var colorIdx = (r.id !== undefined) ? r.id : i;
            html += '<div class="activity-bar-container">' +
                '<div class="activity-bar-label">' +
                '<span class="region-name" style="color:' + regionColor(colorIdx) + '">' + r.name + '</span>' +
                '<span class="region-rate">' + fmt(r.firing_rate_Hz, 1) + ' Hz</span>' +
                '</div>' +
                '<div class="activity-bar-track">' +
                '<div class="activity-bar-fill region-color-' + (colorIdx % 10) + '" style="width:' + pct + '%"></div>' +
                '</div></div>';
        }
        document.getElementById('activityBars').innerHTML = html || '<div style="color: var(--text-dim)">No regions</div>';

        // Update region table
        var tbody = '';
        for (var i = 0; i < data.length; i++) {
            var r = data[i];
            var colorIdx = (r.id !== undefined) ? r.id : i;
            tbody += '<tr>' +
                '<td><span class="color-dot" style="background:' + regionColor(colorIdx) + '"></span>' + r.name + '</td>' +
                '<td>' + (r.neuron_count || 0).toLocaleString() + '</td>' +
                '<td>' + fmt(r.firing_rate_Hz, 2) + ' Hz</td>' +
                '<td>' + (r.active_neurons || 0).toLocaleString() + '</td>' +
                '<td>' + (r.backend || '--') + '</td>' +
                '</tr>';
        }
        document.getElementById('regionTableBody').innerHTML = tbody || '<tr><td colspan="5">No data</td></tr>';

        // Update inject region dropdown and param region dropdown
        updateRegionDropdowns(data);
    }).catch(function() {});
}

function updateRegionDropdowns(data) {
    var selectors = ['injectRegion', 'paramRegionSelect'];
    for (var s = 0; s < selectors.length; s++) {
        var sel = document.getElementById(selectors[s]);
        var currentVal = sel.value;
        var opts = '';
        for (var i = 0; i < data.length; i++) {
            var selected = (String(data[i].id) === currentVal) ? ' selected' : '';
            opts += '<option value="' + data[i].id + '"' + selected + '>' + data[i].name + '</option>';
        }
        if (opts) sel.innerHTML = opts;
    }
}

function refreshSpikes() {
    return api('/api/spikes?last=50').then(function(data) {
        if (!Array.isArray(data) || data.length === 0) return;
        var html = '';
        // Show most recent first
        for (var i = data.length - 1; i >= 0; i--) {
            var e = data[i];
            var name = regionName(e.region);
            var colorIdx = e.region % REGION_COLORS.length;
            html += '<div class="spike-entry">' +
                '<span class="spike-time">' + fmt(e.time, 1) + ' ms</span>' +
                '<span class="spike-region" style="color:' + REGION_COLORS[colorIdx] + '">' + name + '</span>' +
                '<span class="spike-count">' + e.spikes + ' spikes</span>' +
                '</div>';
        }
        document.getElementById('spikeLog').innerHTML = html;
    }).catch(function() {});
}

function refreshHardware() {
    return api('/api/hardware').then(function(data) {
        document.getElementById('hwTier').textContent = data.tier_name || ('T' + (data.tier || '?'));
        document.getElementById('hwCPU').textContent = data.cpu_cores || '--';
        document.getElementById('hwRAM').textContent = data.ram_gb ? fmt(data.ram_gb, 0) : '--';
        document.getElementById('hwGPU').textContent = data.has_gpu ? (data.gpu_name || 'Yes') : 'None';
        document.getElementById('hwNeurons').textContent = data.total_neurons ? data.total_neurons.toLocaleString() : '--';
    }).catch(function() {});
}

function refreshProfile() {
    return api('/api/profile').then(function(data) {
        // Update speed ratio
        if (data.realtime_ratio !== undefined) {
            document.getElementById('speedRatio').textContent = fmt(data.realtime_ratio, 2) + 'x';
        }

        var html = '';
        var keys = ['samples', 'avg_step_ms', 'realtime_ratio', 'drift_sec', 'min_step_ms', 'max_step_ms'];
        var labels = ['Samples', 'Avg Step (ms)', 'Realtime Ratio', 'Drift (s)', 'Min Step (ms)', 'Max Step (ms)'];
        for (var i = 0; i < keys.length; i++) {
            if (data[keys[i]] !== undefined) {
                html += '<div class="debug-row"><span class="debug-key">' + labels[i] +
                    '</span><span class="debug-val">' + fmt(data[keys[i]]) + '</span></div>';
            }
        }
        // Show any extra keys
        for (var k in data) {
            if (keys.indexOf(k) < 0) {
                html += '<div class="debug-row"><span class="debug-key">' + k +
                    '</span><span class="debug-val">' + fmt(data[k]) + '</span></div>';
            }
        }
        document.getElementById('debugProfile').innerHTML = html || '<div class="debug-row"><span class="debug-key">No data</span></div>';
    }).catch(function() {});
}

function refreshDebug() {
    // Counters
    api('/api/debug/counters').then(function(data) {
        var html = '';
        for (var k in data) {
            html += '<div class="debug-row"><span class="debug-key">' + k.replace(/_/g, ' ') +
                '</span><span class="debug-val">' + fmt(data[k]) + '</span></div>';
        }
        document.getElementById('debugCounters').innerHTML = html || '<div class="debug-row"><span class="debug-key">No data</span></div>';
    }).catch(function() {
        document.getElementById('debugCounters').innerHTML = '<div class="debug-row"><span class="debug-key" style="color:var(--error)">Unavailable</span></div>';
    });

    // Memory
    api('/api/memory').then(function(data) {
        var html = '';
        for (var k in data) {
            var val = data[k];
            if (typeof val === 'number') val = fmt(val);
            html += '<div class="debug-row"><span class="debug-key">' + k.replace(/_/g, ' ') +
                '</span><span class="debug-val">' + val + '</span></div>';
        }
        document.getElementById('debugMemory').innerHTML = html || '<div class="debug-row"><span class="debug-key">No data</span></div>';
    }).catch(function() {
        document.getElementById('debugMemory').innerHTML = '<div class="debug-row"><span class="debug-key" style="color:var(--error)">Unavailable</span></div>';
    });

    // Trace
    api('/api/debug/trace').then(function(data) {
        var el = document.getElementById('debugTrace');
        if (typeof data === 'string') {
            el.textContent = data;
        } else {
            el.textContent = JSON.stringify(data, null, 2);
        }
    }).catch(function() {
        document.getElementById('debugTrace').textContent = 'Unavailable';
    });
}

function refreshCameras() {
    api('/api/webcam/cameras').then(function(data) {
        if (!data || !data.cameras) return;
        var sel = document.getElementById('cameraSelect');
        var html = '';
        for (var i = 0; i < data.cameras.length; i++) {
            var cam = data.cameras[i];
            var selected = cam.active ? ' selected' : '';
            html += '<option value="' + cam.id + '"' + selected + '>' + cam.name + (cam.active ? ' (active)' : '') + '</option>';
        }
        if (html) sel.innerHTML = html;
        else sel.innerHTML = '<option value="">No cameras</option>';
    }).catch(function() {});
}

// ── Main Refresh Loop ─────────────────────────────────────────

var tickCount = 0;

function refresh() {
    tickCount++;

    // Every tick: status + regions + spikes
    refreshStatus();
    refreshRegions();
    refreshSpikes();

    // Every 5 ticks: profile
    if (tickCount % 5 === 0) {
        refreshProfile();
    }

    // Every 10 ticks: hardware, cameras, debug
    if (tickCount % 10 === 0) {
        refreshHardware();
        refreshCameras();
        refreshDebug();
    }
}

// ── Initialize ────────────────────────────────────────────────

function init() {
    // Initial full load
    refreshStatus();
    refreshRegions();
    refreshSpikes();
    refreshHardware();
    refreshProfile();
    refreshCameras();
    refreshDebug();

    // Start auto-refresh at 1 second interval
    refreshTimer = setInterval(refresh, 1000);
}

// Start when DOM is ready
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', init);
} else {
    init();
}
</script>
</body>
</html>
)DASHBOARD";
