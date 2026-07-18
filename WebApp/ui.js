// DOM wiring + chart for a single Board.

export const VERSION = '1.6.0';  // bump with every deploy, same value in all 4 files

const CHART_WINDOW_MS = 60_000;

const BOARD_NAME_KEY = 'mc_boardName';
const DEFAULT_BOARD_NAME = 'Board One';

function getBoardName() {
  return (localStorage.getItem(BOARD_NAME_KEY) || '').trim() || DEFAULT_BOARD_NAME;
}

export function bindUI(board) {
  const $ = (id) => document.getElementById(id);

  const els = {
    status: $('status'),
    btnConnect: $('btn-connect'),
    stateChip: $('state-chip'),
    asm1: $('asm1'), asm2: $('asm2'),
    rm1:  $('rm1'),  rm2:  $('rm2'),
    et:   $('et'),
    tsm1In: $('tsm1-in'), tsm1Set: $('tsm1-set'),
    tsm2In: $('tsm2-in'), tsm2Set: $('tsm2-set'),
    ctIn:   $('ct-in'),   ctSet:   $('ct-set'),
    btnStartTimed: $('btn-start-timed'),
    btnStartCont:  $('btn-start-cont'),
    btnStop:       $('btn-stop'),
    btnExportCsv:  $('btn-export-csv'),
    csvFormat:     $('csv-format'),
    autoExportWrap:   $('auto-export-wrap'),
    autoExportToggle: $('auto-export-toggle'),
    chartPause: $('chart-pause'),
    chartClear: $('chart-clear'),
    log: $('log'),
    logExport: $('log-export'),
    logClear: $('log-clear'),
  };

  const chart = makeChart($('chart'));
  let paused = false;

  // ---- Per-cycle recorder (CSV export task 2/5) ----
  // Buffers one row per telemetry tick (~100 ms, driven by the asm1
  // notification) from cycle start to end. Only the most recently completed
  // cycle is kept, per Toon's spec ("available until the next cycle
  // completes") — no export UI yet, this task just proves recording works.
  let recState = 'idle';   // tracked locally so the frequent 'state' event
                            // (fires every heartbeat, not just on change)
                            // only triggers start/stop on a REAL transition
  const recorder = { active: false, t0: null, startDate: null, settings: null, samples: [] };
  let lastCycle = null;

  function startRecording() {
    recorder.active = true;
    recorder.t0 = Date.now();
    recorder.startDate = new Date(recorder.t0);
    recorder.settings = { tsm1: board.values.tsm1, tsm2: board.values.tsm2, ct: board.values.ct };
    recorder.samples = [];
  }

  function stopRecording() {
    if (!recorder.active) return;
    recorder.active = false;
    lastCycle = {
      boardName: boardNameEl.value,
      startDate: recorder.startDate,
      settings: recorder.settings,
      samples: recorder.samples,
    };
    const seconds = (recorder.samples.length / 10).toFixed(1);
    logLine(els.log, `Cycle recorded: ${recorder.samples.length} samples (${seconds} s) — ready to export`);
    els.btnExportCsv.disabled = false;
    els.btnExportCsv.title = '';
    if (autoExportDir) autoExportCsv(lastCycle, resolveCsvFormat(els.csvFormat.value));
  }

  function handleCycleTransition(newState) {
    const wasRunning = recState !== 'idle';
    const isRunning = newState !== 'idle';
    if (!wasRunning && isRunning) startRecording();
    else if (wasRunning && !isRunning) stopRecording();
    recState = newState;
  }

  // ---- Board name (Option A: dashboard-stored; labels CSV exports) ----
  const boardNameEl = $('board-name');
  boardNameEl.value = getBoardName();
  boardNameEl.addEventListener('change', () => {
    const name = boardNameEl.value.trim() || DEFAULT_BOARD_NAME;
    boardNameEl.value = name;
    localStorage.setItem(BOARD_NAME_KEY, name);
    logLine(els.log, `Board name → ${name}`);
    if (board.connected) setStatus(els.status, 'connected', name);
  });

  // ---- Connection button ----
  els.btnConnect.addEventListener('click', async () => {
    if (board.connected) {
      board.disconnect();
      return;
    }
    try {
      els.btnConnect.disabled = true;
      await board.connect();
    } catch (e) {
      logLine(els.log, `ERROR: ${e.message || e}`);
      setStatus(els.status, 'error', e.message || String(e));
    } finally {
      els.btnConnect.disabled = false;
    }
  });

  // ---- Settings writes (flash the button on success/failure; the board layer
  //      already logs the write itself, e.g. "Set TSM1 = 500"). ----
  els.tsm1Set.addEventListener('click', () =>
    doSet(els.tsm1Set, () => board.setTargetSpeed(1, intOr0(els.tsm1In.value))));
  els.tsm2Set.addEventListener('click', () =>
    doSet(els.tsm2Set, () => board.setTargetSpeed(2, intOr0(els.tsm2In.value))));
  els.ctSet.addEventListener('click', () =>
    doSet(els.ctSet, () => board.setCycleTime(intOr0(els.ctIn.value))));

  async function doSet(btn, action) {
    try {
      await action();
      flashBtn(btn, true);
    } catch (e) {
      flashBtn(btn, false);
      err(e);
    }
  }

  // ---- Commands ----
  els.btnStartTimed.addEventListener('click', () => board.sendCommand(0x01).catch(err));
  els.btnStartCont.addEventListener('click',  () => board.sendCommand(0x03).catch(err));
  els.btnStop.addEventListener('click',       () => board.sendCommand(0x02).catch(err));
  els.btnExportCsv.addEventListener('click', () => {
    if (lastCycle) exportCsv(lastCycle, resolveCsvFormat(els.csvFormat.value));
  });

  // ---- CSV decimal/delimiter format (Toon: comma-decimal NL default, but
  // automate it from the browser's language, with a manual override) ----
  els.csvFormat.value = localStorage.getItem(CSV_FORMAT_KEY) || 'auto';
  els.csvFormat.addEventListener('change', () => {
    localStorage.setItem(CSV_FORMAT_KEY, els.csvFormat.value);
  });

  // ---- Auto-export (desktop Chrome/Edge only — File System Access API).
  // Android/iOS and other browsers lack showDirectoryPicker, so the control
  // stays hidden there and only the manual Export CSV button is available.
  // Not persisted across reloads: the folder handle/permission doesn't
  // survive a reload anyway, so the toggle always starts unchecked. ----
  let autoExportDir = null;
  if ('showDirectoryPicker' in window) {
    els.autoExportWrap.classList.remove('hidden');
    els.autoExportWrap.classList.add('inline-flex');
  }
  els.autoExportToggle.addEventListener('change', async () => {
    if (els.autoExportToggle.checked) {
      try {
        autoExportDir = await window.showDirectoryPicker({ mode: 'readwrite' });
        logLine(els.log, `Auto-export folder selected: ${autoExportDir.name}`);
      } catch (e) {
        els.autoExportToggle.checked = false;
        autoExportDir = null;
        if (e.name !== 'AbortError') err(e);   // AbortError = user cancelled the picker
      }
    } else {
      autoExportDir = null;
      logLine(els.log, 'Auto-export disabled.');
    }
  });

  async function autoExportCsv(cycle, format) {
    if (!autoExportDir) return;
    const name = `${sanitizeFilenamePart(cycle.boardName)}_${fmtDateYMD(cycle.startDate)}_${fmtTimeHMSCompact(cycle.startDate)}.csv`;
    try {
      const fileHandle = await autoExportDir.getFileHandle(name, { create: true });
      const writable = await fileHandle.createWritable();
      await writable.write(buildCsv(cycle, format));
      await writable.close();
      logLine(els.log, `Auto-exported: ${name}`);
    } catch (e) {
      // Folder permission can be revoked externally (moved/deleted, OS prompt
      // declined) — disable auto-export rather than fail silently on every
      // future cycle; the manual Export CSV button is unaffected.
      logLine(els.log, `Auto-export failed, disabling: ${e.message || e}`);
      autoExportDir = null;
      els.autoExportToggle.checked = false;
    }
  }

  // ---- Chart controls ----
  els.chartPause.addEventListener('click', () => {
    paused = !paused;
    els.chartPause.textContent = paused ? 'Resume' : 'Pause';
  });
  els.chartClear.addEventListener('click', () => {
    for (const ds of chart.data.datasets) ds.data = [];
    chart._t0 = null;   // restart the count-up axis at 0s
    chart.options.scales.x.min = 0;
    chart.options.scales.x.max = CHART_WINDOW_MS / 1000;
    chart.update('none');
  });

  // Mobile browsers background the tab on app-switch and can leave the chart
  // canvas sized to a stale/zero dimension, so it redraws tiny on return. The
  // container's CSS size is unchanged, so no resize fires to fix it — force one
  // (deferred a frame so layout has settled) whenever the page becomes visible.
  const refreshChart = () => requestAnimationFrame(() => { chart.resize(); chart.update('none'); });
  document.addEventListener('visibilitychange', () => { if (!document.hidden) refreshChart(); });
  window.addEventListener('pageshow', refreshChart);

  // ---- Log export / clear (buttons live inside <summary>; stop the click from
  //      toggling the <details> open/closed). Export saves the FULL history,
  //      not just the ~50 lines visible on screen. ----
  els.logExport.addEventListener('click', (e) => { e.preventDefault(); e.stopPropagation(); exportLog(); });
  els.logClear.addEventListener('click', (e) => {
    e.preventDefault(); e.stopPropagation();
    logHistory.length = 0;
    els.log.textContent = '';
  });

  function err(e) { logLine(els.log, `ERROR: ${e.message || e}`); }

  // ---- Board events ----
  board.addEventListener('connected', (ev) => {
    // Status pill shows the dashboard's custom board name (consistent with the
    // card title), not the raw BLE device name — but the real BLE name is still
    // useful for debugging, so keep it in the log.
    setStatus(els.status, 'connected', boardNameEl.value);
    logLine(els.log, `BLE device name: ${ev.detail}`);
    els.btnConnect.textContent = 'Disconnect';
    applyState(els, board.state, true);
  });
  board.addEventListener('disconnected', () => {
    setStatus(els.status, 'disconnected');
    els.btnConnect.textContent = 'Connect';
    applyState(els, 'idle', false);
    if (recorder.active) {
      recorder.active = false;
      logLine(els.log, 'Cycle recording discarded: disconnected mid-cycle');
    }
    recState = 'idle';
  });
  board.addEventListener('hydrated', (ev) => {
    const v = ev.detail;
    els.tsm1In.value = v.tsm1;
    els.tsm2In.value = v.tsm2;
    els.ctIn.value   = v.ct;
    els.asm1.textContent = v.asm1;
    els.asm2.textContent = v.asm2;
    els.rm1.textContent  = v.rm1;
    els.rm2.textContent  = v.rm2;
    els.et.textContent   = fmtTime(v.et);
    setTargetLines(chart, v.tsm1, v.tsm2);
  });
  board.addEventListener('state', (ev) => {
    applyState(els, ev.detail, board.connected);
    handleCycleTransition(ev.detail);
  });
  board.addEventListener('log', (ev) => logLine(els.log, ev.detail));

  board.addEventListener('telemetry', (ev) => {
    const { key, value } = ev.detail;
    const now = Date.now();
    switch (key) {
      case 'asm1':
        els.asm1.textContent = value;
        if (!paused) pushPoint(chart, 0, now, value);
        // asm1 arrives once per telemetry tick (~100 ms), so use it as the
        // per-row tick for the cycle recorder too — board.values already
        // holds the latest asm2/rm1/rm2 from their own notifications.
        if (recorder.active) {
          recorder.samples.push({
            elapsedMs: now - recorder.t0,
            asm1: value, asm2: board.values.asm2,
            rm1: board.values.rm1, rm2: board.values.rm2,
          });
        }
        break;
      case 'asm2':
        els.asm2.textContent = value;
        if (!paused) pushPoint(chart, 2, now, value);
        break;
      case 'rm1': els.rm1.textContent = value; break;
      case 'rm2': els.rm2.textContent = value; break;
      case 'et':  els.et.textContent = fmtTime(value); break;
    }
    if (!paused) {
      // Push target lines as flat trace so they extend with time.
      pushPoint(chart, 1, now, board.values.tsm1);
      pushPoint(chart, 3, now, board.values.tsm2);
      // Constant-width window in elapsed seconds: fixed 0–60s until the first
      // minute fills (steady scroll speed from the very first sample), then it
      // slides with newest data at the right.
      const winS = CHART_WINDOW_MS / 1000;
      const elapsed = (now - chart._t0) / 1000;
      trimChart(chart, elapsed - winS);
      chart.options.scales.x.min = Math.max(0, elapsed - winS);
      chart.options.scales.x.max = Math.max(winS, elapsed);
      chart.update('none');
    }
  });

  applyState(els, 'idle', false);
  setStatus(els.status, 'disconnected');
}

function applyState(els, state, connected) {
  const running = state !== 'idle';
  const label = state === 'idle' ? 'Idle'
              : state === 'timed' ? 'Running (Timed)'
              : 'Running (Continuous)';
  els.stateChip.textContent = label;
  els.stateChip.className = 'px-2.5 py-0.5 text-xs rounded-full ' +
    (running ? 'bg-emerald-600/30 text-emerald-300' : 'bg-slate-700 text-slate-300');

  const lockTip = running ? 'Stop the motor to change settings' : '';
  for (const el of [els.tsm1In, els.tsm1Set, els.tsm2In, els.tsm2Set,
                    els.ctIn, els.ctSet,
                    els.btnStartTimed, els.btnStartCont]) {
    el.disabled = running;
    el.title = lockTip;
  }
  // Stop must be available whenever the board is connected — never gate it on the
  // reported run-state. A dropped Status notification could otherwise leave the
  // motor running with Stop greyed out (the firmware is the only other way to stop).
  els.btnStop.disabled = !connected;
}

function setStatus(el, kind, detail) {
  if (kind === 'connected') {
    el.textContent = `Connected — ${detail}`;
    el.className = 'px-3 py-1 text-xs rounded-full bg-emerald-600/30 text-emerald-300';
  } else if (kind === 'error') {
    el.textContent = `Error: ${detail}`;
    el.className = 'px-3 py-1 text-xs rounded-full bg-rose-600/30 text-rose-300';
  } else {
    el.textContent = 'Disconnected';
    el.className = 'px-3 py-1 text-xs rounded-full bg-slate-700 text-slate-300';
  }
}

// Full log history (chronological, oldest first) — survives the on-screen
// 50-line cap so Export can save everything captured during a debug session.
const logHistory = [];
const LOG_HISTORY_MAX = 20000;

function logLine(el, msg) {
  const d = new Date();
  const t = `${d.toLocaleTimeString()}.${String(d.getMilliseconds()).padStart(3, '0')}`;
  const line = `[${t}] ${msg}`;
  logHistory.push(line);
  if (logHistory.length > LOG_HISTORY_MAX) logHistory.shift();
  // On-screen: newest on top, capped so the panel stays light.
  el.textContent = `${line}\n` + el.textContent;
  const lines = el.textContent.split('\n');
  if (lines.length > 50) el.textContent = lines.slice(0, 50).join('\n');
}

function exportLog() {
  const text = logHistory.join('\n') + '\n';
  const blob = new Blob([text], { type: 'text/plain' });
  const url = URL.createObjectURL(blob);
  const stamp = new Date().toISOString().replace(/[:.]/g, '-');
  const a = document.createElement('a');
  a.href = url;
  a.download = `motorctrl-log-${stamp}.txt`;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

// ---- CSV export (Toon spec: rows-only template, metadata repeated in every
// row). Decimal/delimiter is either detected from the browser's language or
// forced by the dashboard's format select (task 4/5). ----
const CSV_HEADER = [
  'Board Name', 'Date', 'Time', 'Elapsed Time [s]',
  'Target Speed M1 [rpm]', 'Actual Speed M1 [rpm]', 'Rotations M1',
  'Target Speed M2 [rpm]', 'Actual Speed M2 [rpm]', 'Rotations M2',
];
const CSV_FORMAT_KEY = 'mc_csvFormat';   // 'auto' | 'comma' | 'dot'

// Dutch/European locales use a decimal comma, so the delimiter must move to
// semicolon to avoid ambiguity; English-style locales use a decimal dot with
// a comma delimiter (ordinary CSV). Detected via Intl, same mechanism Excel
// itself uses to guess a file's format.
function detectDecimalStyle() {
  const dec = new Intl.NumberFormat(navigator.language).formatToParts(1.1)
    .find((p) => p.type === 'decimal');
  return dec && dec.value === ',' ? 'comma' : 'dot';
}

function resolveCsvFormat(pref) {
  const style = (pref === 'comma' || pref === 'dot') ? pref : detectDecimalStyle();
  return style === 'comma' ? { decimal: ',', delimiter: '; ' } : { decimal: '.', delimiter: ', ' };
}

function pad2(n) { return String(n).padStart(2, '0'); }
function fmtDateYMD(d) { return `${d.getFullYear()}-${pad2(d.getMonth() + 1)}-${pad2(d.getDate())}`; }
function fmtTimeHMS(d) { return `${pad2(d.getHours())}:${pad2(d.getMinutes())}:${pad2(d.getSeconds())}`; }
function fmtTimeHMSCompact(d) { return `${pad2(d.getHours())}${pad2(d.getMinutes())}${pad2(d.getSeconds())}`; }
function sanitizeFilenamePart(s) { return s.replace(/[\\/:*?"<>|]/g, '-'); }

function buildCsv(cycle, format) {
  const dateStr = fmtDateYMD(cycle.startDate);
  const timeStr = fmtTimeHMS(cycle.startDate);
  const rows = cycle.samples.map((s) => [
    cycle.boardName, dateStr, timeStr,
    (s.elapsedMs / 1000).toFixed(1).replace('.', format.decimal),
    cycle.settings.tsm1, s.asm1, s.rm1,
    cycle.settings.tsm2, s.asm2, s.rm2,
  ].join(format.delimiter));
  return [CSV_HEADER.join(format.delimiter), ...rows].join('\r\n') + '\r\n';   // CRLF for Excel
}

function exportCsv(cycle, format) {
  const blob = new Blob([buildCsv(cycle, format)], { type: 'text/csv' });
  const url = URL.createObjectURL(blob);
  const name = `${sanitizeFilenamePart(cycle.boardName)}_${fmtDateYMD(cycle.startDate)}_${fmtTimeHMSCompact(cycle.startDate)}.csv`;
  const a = document.createElement('a');
  a.href = url;
  a.download = name;
  document.body.appendChild(a);
  a.click();
  a.remove();
  URL.revokeObjectURL(url);
}

function fmtTime(sec) {
  const m = String(Math.floor(sec / 60)).padStart(2, '0');
  const s = String(sec % 60).padStart(2, '0');
  return `${m}:${s}`;
}

function intOr0(v) {
  const n = parseInt(v, 10);
  return Number.isFinite(n) && n >= 0 ? n : 0;
}

// Brief outline flash to acknowledge a Set press: green on success, red on
// failure. Uses a ring (outline) so it never shifts layout. Re-pressing while
// still flashing resets the timer cleanly.
function flashBtn(btn, ok) {
  const ring = ok ? 'ring-emerald-400' : 'ring-rose-400';
  if (btn._flashTimer) clearTimeout(btn._flashTimer);
  btn.classList.remove('ring-emerald-400', 'ring-rose-400');
  btn.classList.add('ring-2', ring);
  btn._flashTimer = setTimeout(() => {
    btn.classList.remove('ring-2', ring);
    btn._flashTimer = null;
  }, 700);
}

function makeChart(canvas) {
  return new Chart(canvas, {
    type: 'line',
    data: {
      datasets: [
        { label: 'Actual M1', yAxisID: 'y1', data: [], borderColor: '#38bdf8', borderWidth: 2, pointRadius: 0, tension: 0.2 },
        { label: 'Target M1', yAxisID: 'y1', data: [], borderColor: '#38bdf8', borderWidth: 1, borderDash: [4, 4], pointRadius: 0 },
        { label: 'Actual M2', yAxisID: 'y2', data: [], borderColor: '#f59e0b', borderWidth: 2, pointRadius: 0, tension: 0.2 },
        { label: 'Target M2', yAxisID: 'y2', data: [], borderColor: '#f59e0b', borderWidth: 1, borderDash: [4, 4], pointRadius: 0 },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      scales: {
        x: {
          // X values are elapsed seconds since the first sample (anchored in
          // chart._t0 by pushPoint), NOT epoch ms — so Chart.js picks round
          // ticks (0s, 10s, 20s…). With epoch values it chose positions that
          // were "nice" in absolute time but arbitrary as elapsed (8s, 18s…).
          type: 'linear',
          min: 0,
          max: CHART_WINDOW_MS / 1000,
          ticks: {
            color: '#94a3b8',
            includeBounds: false,   // no stray tick at the scrolling window edge
            callback: (v) => `${Math.round(v)}s`,
          },
          grid: { color: '#334155' },
        },
        // Dual Y-axis, one per motor, each in its motor's colour. They scale
        // independently — to lock them to a shared scale, give both the same
        // `max` (e.g. max: 3000).
        y1: {
          type: 'linear', position: 'left', beginAtZero: true,
          ticks: { color: '#38bdf8' },
          grid: { color: '#334155' },
          title: { display: true, text: 'M1 RPM', color: '#38bdf8' },
        },
        y2: {
          type: 'linear', position: 'right', beginAtZero: true,
          ticks: { color: '#f59e0b' },
          grid: { drawOnChartArea: false }, // avoid double gridlines over y1
          title: { display: true, text: 'M2 RPM', color: '#f59e0b' },
        },
      },
      plugins: { legend: { labels: { color: '#cbd5e1' } } },
    },
  });
}

function pushPoint(chart, idx, ts, val) {
  if (chart._t0 == null) chart._t0 = ts;   // anchor the count-up axis at first sample
  chart.data.datasets[idx].data.push({ x: (ts - chart._t0) / 1000, y: val });
}

function trimChart(chart, minX) {
  for (const ds of chart.data.datasets) {
    while (ds.data.length && ds.data[0].x < minX) ds.data.shift();
  }
}

function setTargetLines(chart, t1, t2) {
  const x = chart._t0 == null ? 0 : (Date.now() - chart._t0) / 1000;
  chart.data.datasets[1].data = [{ x, y: t1 }];
  chart.data.datasets[3].data = [{ x, y: t2 }];
  chart.update('none');
}
