// BLE adapter for MotorCtrl (STM32WB55) — one Board instance per device.
// Custom 16-bit UUIDs are promoted to the Bluetooth Base UUID form.

export const VERSION = '1.2.1';  // bump with every deploy, same value in all 4 files

const CTRL_SERVICE = '00000001-0000-1000-8000-00805f9b34fb';
const TELE_SERVICE = '00000010-0000-1000-8000-00805f9b34fb';

const CHAR = {
  tsm1: '00000002-0000-1000-8000-00805f9b34fb',
  tsm2: '00000003-0000-1000-8000-00805f9b34fb',
  ct:   '00000004-0000-1000-8000-00805f9b34fb',
  cmd:  '00000005-0000-1000-8000-00805f9b34fb',
  asm1: '00000011-0000-1000-8000-00805f9b34fb',
  asm2: '00000012-0000-1000-8000-00805f9b34fb',
  rm1:  '00000013-0000-1000-8000-00805f9b34fb',
  rm2:  '00000014-0000-1000-8000-00805f9b34fb',
  et:   '00000015-0000-1000-8000-00805f9b34fb',
  status: '00000016-0000-1000-8000-00805f9b34fb',
};

export const CMD = { START_TIMED: 0x01, STOP: 0x02, START_CONT: 0x03 };
export const STATE = { IDLE: 'idle', TIMED: 'timed', CONT: 'cont' };

// Firmware Status byte → UI state. Matches main.h: 0=Stopped, 1=Running, 2=Continue.
const STATUS_TO_STATE = { 0: STATE.IDLE, 1: STATE.TIMED, 2: STATE.CONT };

export class Board extends EventTarget {
  constructor() {
    super();
    this.device = null;
    this.server = null;
    this.ctrlSvc = null;
    this.teleSvc = null;
    this.chars = {};
    this.state = STATE.IDLE;
    this.values = {
      tsm1: 0, tsm2: 0, ct: 0,
      asm1: 0, asm2: 0, rm1: 0, rm2: 0, et: 0,
    };
  }

  get connected() {
    return !!(this.device && this.device.gatt && this.device.gatt.connected);
  }

  async connect() {
    // requestDevice needs a user gesture and must run exactly once — keep it
    // outside the retry loop.
    this._emit('log', 'Requesting MotorCtrl device…');
    this.device = await navigator.bluetooth.requestDevice({
      filters: [{ name: 'MotorCtrl' }],
      optionalServices: [CTRL_SERVICE, TELE_SERVICE],
    });
    this.device.addEventListener('gattserverdisconnected', () => this._onDisconnect());

    // Intermittent "GATT Server is disconnected. Cannot retrieve services" is a
    // known Web Bluetooth + STM32WB race: the link can drop before discovery
    // settles. Retry the GATT connect + discovery a few times with backoff.
    const attempts = 4;
    this._connecting = true;
    try {
      for (let i = 1; i <= attempts; i++) {
        try {
          await this._open();
          return; // success
        } catch (e) {
          this._emit('log', `Connect attempt ${i}/${attempts} failed: ${e.message || e}`);
          try { this.device.gatt.disconnect(); } catch (_) { /* already down */ }
          if (i === attempts) throw e;
          await delay(300 * i); // 300, 600, 900 ms backoff
        }
      }
    } finally {
      this._connecting = false;
    }
  }

  async _open() {
    this._emit('log', 'Connecting GATT…');
    this.server = await this.device.gatt.connect();
    // Give the link a moment to settle before service discovery — this is what
    // actually fixes the intermittent "cannot retrieve services" drop.
    await delay(250);

    this.ctrlSvc = await this.server.getPrimaryService(CTRL_SERVICE);
    this.teleSvc = await this.server.getPrimaryService(TELE_SERVICE);

    this.chars.tsm1 = await this.ctrlSvc.getCharacteristic(CHAR.tsm1);
    this.chars.tsm2 = await this.ctrlSvc.getCharacteristic(CHAR.tsm2);
    this.chars.ct   = await this.ctrlSvc.getCharacteristic(CHAR.ct);
    this.chars.cmd  = await this.ctrlSvc.getCharacteristic(CHAR.cmd);
    this.chars.asm1 = await this.teleSvc.getCharacteristic(CHAR.asm1);
    this.chars.asm2 = await this.teleSvc.getCharacteristic(CHAR.asm2);
    this.chars.rm1  = await this.teleSvc.getCharacteristic(CHAR.rm1);
    this.chars.rm2  = await this.teleSvc.getCharacteristic(CHAR.rm2);
    this.chars.et   = await this.teleSvc.getCharacteristic(CHAR.et);
    this.chars.status = await this.teleSvc.getCharacteristic(CHAR.status);

    // Hydrate from current firmware state — settings + telemetry + run state.
    // Each read triggers the firmware's read-permit hook, which refreshes the
    // value from the live source, so these are the true current values.
    this._emit('log', 'Reading saved values…');
    this.values.tsm1 = readU16(await this.chars.tsm1.readValue());
    this.values.tsm2 = readU16(await this.chars.tsm2.readValue());
    this.values.ct   = readU16(await this.chars.ct.readValue());
    this.values.asm1 = readU16(await this.chars.asm1.readValue());
    this.values.asm2 = readU16(await this.chars.asm2.readValue());
    this.values.rm1  = readU32(await this.chars.rm1.readValue());
    this.values.rm2  = readU32(await this.chars.rm2.readValue());
    this.values.et   = readU16(await this.chars.et.readValue());
    this.state = statusToState(await this.chars.status.readValue());
    this._emit('hydrated', { ...this.values });
    this._emit('state', this.state);

    // Subscribe to telemetry notifications.
    await this._sub('asm1', readU16);
    await this._sub('asm2', readU16);
    await this._sub('rm1',  readU32);
    await this._sub('rm2',  readU32);
    await this._sub('et',   readU16);

    // Subscribe to run-state notifications — firmware is the source of truth,
    // so this corrects the UI on timed auto-stop and physical button presses.
    this.chars.status.addEventListener('characteristicvaluechanged', (ev) => {
      // The heartbeat's readValue() also fires this listener, so log only real
      // state CHANGES — otherwise every 3 s poll spams "state → idle". The
      // 'state' event still fires every time to keep the UI reconciled.
      const st = statusToState(ev.target.value);
      const changed = st !== this.state;
      this.state = st;
      this._emit('state', st);
      if (changed) this._emit('log', `Firmware state → ${st}`);
    });
    await this.chars.status.startNotifications();
    this._startHeartbeat();

    this._emit('log', 'Connected.');
    this._emit('connected', this.device.name || 'MotorCtrl');
  }

  async _sub(key, decode) {
    const ch = this.chars[key];
    ch.addEventListener('characteristicvaluechanged', (ev) => {
      const v = decode(ev.target.value);
      this.values[key] = v;
      this.dispatchEvent(new CustomEvent('telemetry', { detail: { key, value: v } }));
    });
    await ch.startNotifications();
  }

  async setTargetSpeed(motor, rpm) {
    const key = motor === 1 ? 'tsm1' : 'tsm2';
    await this.chars[key].writeValueWithResponse(u16LE(rpm));
    this.values[key] = rpm;
    this._emit('log', `Set M${motor} target = ${rpm} RPM`);
  }

  async setCycleTime(seconds) {
    await this.chars.ct.writeValueWithResponse(u16LE(seconds));
    this.values.ct = seconds;
    this._emit('log', `Set cycle time = ${seconds} s`);
  }

  async sendCommand(cmd) {
    // Fire-and-confirm: the firmware is the source of truth for run state.
    // We do NOT set this.state here; the Status notification updates the UI
    // once the firmware actually changes state (so a rejected command can't
    // leave the UI showing a state the board isn't in).
    await this.chars.cmd.writeValueWithResponse(new Uint8Array([cmd]));
    this._emit('log', `CMD 0x${cmd.toString(16).padStart(2, '0')} sent`);
    // Self-heal: the firmware only notifies Status on a state CHANGE and never
    // resends a dropped notification, so re-read it shortly after the command
    // to reconcile the UI if that one notification was lost over the air.
    setTimeout(() => this._refreshState().catch(() => {}), 400);
  }

  // Re-read the Status characteristic and re-sync the UI run-state. Used by the
  // post-command reread and the periodic heartbeat to recover from lost Status
  // notifications (common over flaky links / iOS Web Bluetooth bridges).
  async _refreshState() {
    if (!this.connected || !this.chars.status) return;
    const st = statusToState(await this.chars.status.readValue());
    const changed = st !== this.state;
    this.state = st;
    this._emit('state', st);
    if (changed) this._emit('log', `Firmware state ⟳ ${st}`);
  }

  _startHeartbeat() {
    this._stopHeartbeat();
    // Poll run-state while connected so a dropped Status notification can't leave
    // the UI permanently out of sync with the board.
    this._hb = setInterval(() => this._refreshState().catch(() => {}), 3000);
  }

  _stopHeartbeat() {
    if (this._hb) { clearInterval(this._hb); this._hb = null; }
  }

  disconnect() {
    if (this.device && this.device.gatt && this.device.gatt.connected) {
      this.device.gatt.disconnect();
    }
  }

  _onDisconnect() {
    // A drop during the connect retry loop is expected — don't reset the UI;
    // _open() will retry and emit 'connected' on success.
    if (this._connecting) {
      this._emit('log', 'Link dropped during connect — retrying…');
      return;
    }
    this._stopHeartbeat();
    this.state = STATE.IDLE;
    this._emit('disconnected', null);
    this._emit('log', 'Disconnected.');
  }

  _emit(type, detail) {
    this.dispatchEvent(new CustomEvent(type, { detail }));
  }
}

function delay(ms) { return new Promise((resolve) => setTimeout(resolve, ms)); }
function readU16(dv) { return dv.getUint16(0, true); }
function readU32(dv) { return dv.getUint32(0, true); }
function statusToState(dv) { return STATUS_TO_STATE[dv.getUint8(0)] ?? STATE.IDLE; }
function u16LE(v) {
  const b = new Uint8Array(2);
  new DataView(b.buffer).setUint16(0, v & 0xFFFF, true);
  return b;
}
