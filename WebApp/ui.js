// DOM wiring + chart for a single Board.

const CHART_WINDOW_MS = 60_000;

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
    chartPause: $('chart-pause'),
    chartClear: $('chart-clear'),
    log: $('log'),
    logExport: $('log-export'),
    logClear: $('log-clear'),
  };

  const chart = makeChart($('chart'));
  let paused = false;

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

  // ---- Settings writes ----
  els.tsm1Set.addEventListener('click', () => board.setTargetSpeed(1, intOr0(els.tsm1In.value)).catch(err));
  els.tsm2Set.addEventListener('click', () => board.setTargetSpeed(2, intOr0(els.tsm2In.value)).catch(err));
  els.ctSet.addEventListener('click',   () => board.setCycleTime(intOr0(els.ctIn.value)).catch(err));

  // ---- Commands ----
  els.btnStartTimed.addEventListener('click', () => board.sendCommand(0x01).catch(err));
  els.btnStartCont.addEventListener('click',  () => board.sendCommand(0x03).catch(err));
  els.btnStop.addEventListener('click',       () => board.sendCommand(0x02).catch(err));

  // ---- Chart controls ----
  els.chartPause.addEventListener('click', () => {
    paused = !paused;
    els.chartPause.textContent = paused ? 'Resume' : 'Pause';
  });
  els.chartClear.addEventListener('click', () => {
    for (const ds of chart.data.datasets) ds.data = [];
    chart.update('none');
  });

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
    setStatus(els.status, 'connected', ev.detail);
    els.btnConnect.textContent = 'Disconnect';
    applyState(els, board.state);
  });
  board.addEventListener('disconnected', () => {
    setStatus(els.status, 'disconnected');
    els.btnConnect.textContent = 'Connect';
    applyState(els, 'idle');
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
  board.addEventListener('state', (ev) => applyState(els, ev.detail));
  board.addEventListener('log', (ev) => logLine(els.log, ev.detail));

  board.addEventListener('telemetry', (ev) => {
    const { key, value } = ev.detail;
    const now = Date.now();
    switch (key) {
      case 'asm1':
        els.asm1.textContent = value;
        if (!paused) pushPoint(chart, 0, now, value);
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
      trimChart(chart, now - CHART_WINDOW_MS);
      chart.update('none');
    }
  });

  applyState(els, 'idle');
  setStatus(els.status, 'disconnected');
}

function applyState(els, state) {
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
  els.btnStop.disabled = !running;
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

function fmtTime(sec) {
  const m = String(Math.floor(sec / 60)).padStart(2, '0');
  const s = String(sec % 60).padStart(2, '0');
  return `${m}:${s}`;
}

function intOr0(v) {
  const n = parseInt(v, 10);
  return Number.isFinite(n) && n >= 0 ? n : 0;
}

function makeChart(canvas) {
  return new Chart(canvas, {
    type: 'line',
    data: {
      datasets: [
        { label: 'Actual M1', data: [], borderColor: '#38bdf8', borderWidth: 2, pointRadius: 0, tension: 0.2 },
        { label: 'Target M1', data: [], borderColor: '#38bdf8', borderWidth: 1, borderDash: [4, 4], pointRadius: 0 },
        { label: 'Actual M2', data: [], borderColor: '#f59e0b', borderWidth: 2, pointRadius: 0, tension: 0.2 },
        { label: 'Target M2', data: [], borderColor: '#f59e0b', borderWidth: 1, borderDash: [4, 4], pointRadius: 0 },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: false,
      scales: {
        x: { type: 'linear', ticks: { color: '#94a3b8', callback: (v) => relSec(v) }, grid: { color: '#334155' } },
        y: { beginAtZero: true, ticks: { color: '#94a3b8' }, grid: { color: '#334155' }, title: { display: true, text: 'RPM', color: '#94a3b8' } },
      },
      plugins: { legend: { labels: { color: '#cbd5e1' } } },
    },
  });
}

function relSec(tsMs) {
  const d = (tsMs - Date.now()) / 1000;
  return `${d.toFixed(0)}s`;
}

function pushPoint(chart, idx, ts, val) {
  chart.data.datasets[idx].data.push({ x: ts, y: val });
}

function trimChart(chart, minTs) {
  for (const ds of chart.data.datasets) {
    while (ds.data.length && ds.data[0].x < minTs) ds.data.shift();
  }
}

function setTargetLines(chart, t1, t2) {
  const now = Date.now();
  chart.data.datasets[1].data = [{ x: now, y: t1 }];
  chart.data.datasets[3].data = [{ x: now, y: t2 }];
  chart.update('none');
}
