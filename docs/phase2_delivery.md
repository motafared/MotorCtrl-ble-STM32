# MotorCtrl — Phase 2 Delivery (BLE Wireless Control)

**Client:** Toon
**Scope:** STM32 firmware + Web Bluetooth dashboard that add wireless control and
live telemetry to the Phase 1 dual-BLDC motor controller.
**Hardware:** WeAct STM32WB55 Core Board (STM32WB55CGU6), two 12 V 4-wire BLDC
motors with 1-pulse-per-revolution tachometers, three push-buttons, status LED.

---

## 1. What this is

Phase 1 was a dual-BLDC speed controller on an STM32F103: two motors, PID speed
control, three run modes (timed / continuous / stopped), driven by on-board
buttons. **Phase 2 moves it to the STM32WB55** (same STM32 ecosystem, plus
built-in Bluetooth LE 5.0) and adds:

- **BLE control** — set target speeds and cycle time, and start/stop the motors,
  from a phone or laptop with no app install (a web page over Web Bluetooth).
- **BLE telemetry** — live actual speed, rotation counts, elapsed time, and run
  state pushed to the dashboard as they change.
- The original **push-button control still works** in parallel with BLE, and the
  dashboard tracks button presses live.

**In scope for Phase 2:**
- STM32WB55 firmware: motor PID port, BLE GATT services (control + telemetry),
  button + timed-cycle logic, status LED.
- Web Bluetooth dashboard (single-page, no install).
- This documentation.

**Out of scope / deferred (see §10):**
- Custom PCB build (a KiCad design exists under `kicad/`, not yet fabricated).
- Multi-board control (up to 4 boards in one browser tab) — architecture is
  ready, UI is single-board.
- Persisting settings across power cycles (flash storage).

---

## 2. Hardware bill of materials

| Item | Notes |
|---|---|
| WeAct STM32WB55 Core Board | STM32WB55CGU6, 64 MHz, BLE 5.0, supplied by client |
| 2× 12 V 4-wire BLDC motors | PWM + tachometer (1 pulse/rev sensed; see §4 for PPR) |
| 12 V DC supply | Motor rail; logic is 3.3 V from the board regulator |
| 3× momentary push-buttons | START / STOP / CONTINUE — see §3.2 wiring (active-HIGH) |
| 1× LED + ~1 kΩ resistor | Status indicator on PE4 |
| ST-LINK (or on-board) | Flashing / debug over SWD |
| Phone or PC with Chrome/Edge | To run the Web Bluetooth dashboard (§7) |

A detailed pin-by-pin wiring sheet and a styled connection diagram live in
`kicad/MotorCtrl/WIRING.md` and `kicad/MotorCtrl/connections.html`.

---

## 3. Hardware connections

All control logic is 3.3 V. The motors run on +12 V; share a common ground
between the 12 V supply and the board.

### 3.1 Pin map (STM32WB55, 64 MHz)

| Signal | STM32 Pin | Peripheral | Notes |
|---|---|---|---|
| Motor 1 PWM | PA0 | TIM2_CH1 | 20 kHz PWM |
| Motor 2 PWM | PA1 | TIM2_CH2 | 20 kHz PWM |
| Motor 1 tacho | PA8 | TIM1_CH1 | input capture |
| Motor 2 tacho | PA9 | TIM1_CH2 | input capture |
| Button STOP | PA2 | EXTI2 | active-HIGH, internal pull-down |
| Button START | PA3 | EXTI3 | active-HIGH, internal pull-down |
| Button CONTINUE | PA4 | EXTI4 | active-HIGH, internal pull-down |
| Status LED (blue) | PE4 | GPIO out | ON running, 5 Hz heartbeat |
| SWD debug | PA13 / PA14 | SWDIO / SWCLK | flashing |

Timer setup: TIM2 prescaler 0 / period 3199 → 20 kHz PWM at 64 MHz · TIM1
prescaler 999 → tacho input capture · TIM16 prescaler 6399 / period 999 →
100 ms PID + telemetry tick.

### 3.2 Push-buttons (active-HIGH, internal pull-down)

The buttons are wired **active-HIGH**: pressing a button connects the MCU pin to
**+3V3**. The MCU's **internal pull-down** holds the pin LOW when released, so
**no external pull resistor is required**.

```
   Push-buttons                      STM32WB55 board
   ┌─────────────────────┐          ┌──────────────────────────┐
   │ START   SW1 ────────├─────────►┤ PA3   BTN_START          │ EXTI3  rising (press)
   │ STOP    SW2 ────────├─────────►┤ PA2   BTN_STOP           │ EXTI2  rising (press)
   │ CONT    SW3 ────────├─────────►┤ PA4   BTN_CONTINUE       │ EXTI4  both edges
   │ (common → +3V3)     │          │   (internal pull-down ON) │
   └─────────────────────┘          └──────────────────────────┘
```

- **Idle = LOW**, **pressed = HIGH**.
- START / STOP act on the **rising edge** (the press).
- CONTINUE uses **both edges**: rising = start (runs while held), falling =
  release = stop.
- Firmware debounces 50 ms in software; an optional series resistor + small cap
  to GND is fine for ESD but not electrically required.

> ⚠ This differs from the older active-low note in some early hardware drafts.
> The firmware is the source of truth (`Core/Src/main.c` → `MX_GPIO_Init`,
> `GPIO_PULLDOWN`). The KiCad/SKiDL source (`kicad/MotorCtrl/motorctrl.py`) must
> match: button common to **+3V3**, external pull-ups removed.

### 3.3 Motors

```
   Motor M1 (4-wire)                 STM32WB55 board
   +12V ───────────── +12 V rail
   GND  ───────────── GND (common with board GND)
   PWM  ◄──────────── PA0  (TIM2_CH1)
   Tacho ──────────►  PA8  (TIM1_CH1)

   Motor M2 (4-wire)
   PWM  ◄──────────── PA1  (TIM2_CH2)
   Tacho ──────────►  PA9  (TIM1_CH2)
```

Each tacho line typically needs a pull-up to +3V3. The PWM output idles at the
inactive level until a motor is started (see §9 troubleshooting on bench
behaviour with no motor connected).

---

## 4. Build modes (Khaled vs Toon)

A single switch in `Core/Inc/main.h` selects between the bench-test setup and the
client hardware, because they differ in tachometer pulses-per-revolution and PWM
polarity:

```c
#define MOTOR_MODE   MODE_KHALED   /* <<< switch here: MODE_KHALED or MODE_TOON */
```

| Mode | `MOTOR_PPR` | PWM polarity | Use |
|---|---|---|---|
| `MODE_KHALED` | 2 | `TIM_OCPOLARITY_HIGH` (not inverted) | bench testing |
| `MODE_TOON` | 6 | `TIM_OCPOLARITY_LOW` (inverted) | client hardware |

`M1_PPR` / `M2_PPR` follow `MOTOR_PPR`. PWM polarity is re-applied in the
`USER CODE BEGIN TIM2_Init 2` block of `Core/Src/main.c` so it survives CubeMX
regeneration. **Flip the one `MOTOR_MODE` line and rebuild** to switch targets —
nothing else changes. Default ships as `MODE_KHALED`.

Other defaults (`main.h`): target speed 1200 RPM per motor, cycle time 35 s
(timed mode), PID `Kp = 0.012`, `Ki = 0`, `Kd = 0`, 100 ms control tick.

---

## 5. Firmware install

The STM32WB55 has **two cores**: the application runs on the CM4; the BLE stack
runs on a CM0+ co-processor and is flashed **separately**. You must have the BLE
stack in place once before the application's Bluetooth will work.

### 5.1 Flash the BLE co-processor stack (one-time, STM32CubeProgrammer)

Binaries come from the **STM32CubeWB** package:
`STM32Cube_FW_WB_Vx.x.x\Projects\STM32WB_Copro_Wireless_Binaries\STM32WB5x\`.

1. Connect via ST-LINK; open **STM32CubeProgrammer**.
2. Flash **FUS** `stm32wb5x_FUS_fw.bin` at **`0x080EC000`** — skip if a recent
   FUS (≥ 1.2.0) is already present.
3. Flash the **BLE Full stack** `stm32wb5x_BLE_Stack_full_fw.bin` at
   **`0x080C0000`** (this project uses `BLE_STACK=FULL`).
4. Verify in the **Wireless** tab — it shows the installed FUS and BLE versions.

(If the stack is already installed and verified, skip to 5.2.)

### 5.2 Build and flash the application (STM32CubeIDE)

This project is **STM32CubeIDE only** (ignore the `EWARM/` folder).

1. Open **STM32CubeIDE** → **File → Open Projects from File System** → select the
   `Phase2/` project folder.
2. Confirm `MOTOR_MODE` in `main.h` (§4) is set for the board you're flashing.
3. **Project → Build All** — should complete with **0 errors**.
4. **Run → Run As → STM32 C/C++ Application** to flash the CM4 application at
   `0x08000000` and start it.

### 5.3 Verify it's alive

- The **blue LED (PE4)** is OFF when stopped. Press **START** → motors run and the
  LED blinks ~5 Hz (the PID-tick heartbeat). Press **STOP** → LED off.
- From a phone/PC, the board advertises as **`MotorCtrl`** and is visible to a BLE
  scanner (nRF Connect) or the dashboard (§7).

---

## 6. BLE GATT reference

- **Advertised name:** `MotorCtrl`
- **All multi-byte values are little-endian.**
- Two services: **Control** (write settings + commands) and **Telemetry**
  (read + notify live values).

### 6.1 Control service — UUID `0x0001`

| Characteristic | UUID | Type | Properties | Meaning |
|---|---|---|---|---|
| TSM1 | `0x0002` | uint16 | Read / Write | Target speed motor 1 (RPM) |
| TSM2 | `0x0003` | uint16 | Read / Write | Target speed motor 2 (RPM) |
| CT   | `0x0004` | uint16 | Read / Write | Cycle time (seconds, timed mode) |
| CMD  | `0x0005` | uint8  | Write | Command: `0x01` start timed · `0x02` stop · `0x03` start continuous |

### 6.2 Telemetry service — UUID `0x0010`

| Characteristic | UUID | Type | Properties | Meaning |
|---|---|---|---|---|
| ASM1 | `0x0011` | uint16 | Read / Notify | Actual speed motor 1 (RPM) |
| ASM2 | `0x0012` | uint16 | Read / Notify | Actual speed motor 2 (RPM) |
| RM1  | `0x0013` | uint32 | Read / Notify | Cumulative rotations motor 1 |
| RM2  | `0x0014` | uint32 | Read / Notify | Cumulative rotations motor 2 |
| ET   | `0x0015` | uint16 | Read / Notify | Elapsed time (seconds) |
| STATUS | `0x0016` | uint8 | Read / Notify | Run state: `0` idle · `1` timed · `2` continuous |

### 6.3 Behaviour notes

- **Reads return live values.** A read of any characteristic refreshes it from the
  firmware's current state before answering (not a stale cache), so a client that
  connects mid-run sees the true values immediately.
- **STATUS is the source of truth for run state.** The firmware notifies STATUS on
  **every** transition — BLE command, timed auto-stop, *and* physical button
  presses — so a dashboard always reflects what the board is actually doing.
- Telemetry notifications are emitted every 100 ms while running, from a safe BLE
  task context (not from an interrupt).

---

## 7. Web Bluetooth dashboard

A single-page dashboard (in the `WebApp/` folder) that connects to the board over
BLE — **no install**.

### 7.1 Browser support

- **Desktop / Android:** Google Chrome, Edge, or Brave (any Chromium with Web
  Bluetooth).
- **iPhone / iPad:** the **Bluefy** browser (free, App Store) — Safari does not
  support Web Bluetooth.
- Web Bluetooth requires a **secure context**: serve over `https://` (e.g. GitHub
  Pages) or `http://localhost`. Opening the file directly (`file://`) will not
  work.

### 7.2 Run it locally

```bash
cd WebApp
python -m http.server 8000
# open http://localhost:8000 in Chrome
```

### 7.3 Using it

1. Click **Connect** and pick **MotorCtrl** from the chooser.
2. The dashboard hydrates from the board: target speeds, cycle time, live RPM,
   rotation counts, elapsed time, and run state.
3. Set **Target** speed per motor and the **Cycle** time, then **Start Timed**,
   **Start Continuous**, or **Stop**.
4. The **Live Speed** chart shows actual vs target RPM over a 60 s rolling window.
5. Settings are **locked while running** — stop first to change them.
6. The state chip and lock track the board live, including physical-button
   presses and the automatic stop at the end of a timed run.

> **After flashing new firmware, hard-reload the page** (Ctrl+Shift+R). Browsers
> cache the dashboard's scripts; a stale cache can keep telemetry working while
> new features silently don't.

---

## 8. Operation summary

| Action | Button | BLE command | Result |
|---|---|---|---|
| Start timed run | START (PA3) | CMD `0x01` | Both motors run to target speed; auto-stop after the cycle time (default 35 s). |
| Stop | STOP (PA2) | CMD `0x02` | Motors stop immediately, any mode. |
| Start continuous | CONTINUE (PA4), held | CMD `0x03` | Motors run; **button mode stops on release**. |

- **Speed control** is closed-loop PID using the tachometer; target defaults to
  1200 RPM and is adjustable live over BLE.
- **Rotations** accumulate while running and are reported live and at cycle end.
- The **blue LED** is ON (blinking 5 Hz) while running, OFF when stopped.

---

## 9. Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Dashboard can't find the board | BLE stack not flashed, or board not advertising | Verify CPU2 stack in CubeProgrammer **Wireless** tab (§5.1); confirm `MotorCtrl` shows in a BLE scanner |
| "GATT Server is disconnected. Cannot retrieve services" (intermittent) | Web Bluetooth + WB connection race | The dashboard retries automatically; if persistent, forget the device in OS Bluetooth and reconnect |
| Connected, telemetry works, but run-state/buttons don't update the UI | Browser serving a cached old dashboard | **Hard-reload** the page (Ctrl+Shift+R), or DevTools → Network → "Disable cache" |
| No PWM on the analyser with no motor connected | Open-loop PID winds duty to 100% (no feedback) → pin sits constant HIGH | Expected on the bench; connect a motor for closed-loop, or temporarily zero the PID gains / force a fixed duty |
| Motor doesn't reach/track speed | Wrong `MOTOR_MODE` PPR, or tacho not wired | Confirm `MODE_KHALED` (PPR 2) vs `MODE_TOON` (PPR 6) matches the motor; check tacho pull-up and wiring |
| Button does nothing / triggers inverted | Button wired active-low instead of active-HIGH | Wire button common to **+3V3** (§3.2); firmware uses internal pull-down |
| Settings won't change in the dashboard | Motors are running (settings locked) | Press **Stop** first, then edit |
| Build fails after editing BLE chars in CubeMX | CubeMX caps STM32WB services at 5 characteristics | The 6th char (STATUS) is added in code, not the `.ioc`; see §10 |

---

## 10. Production notes & deferred items

- **STM32WB CubeMX 5-characteristic limit.** The CubeMX GATT designer caps each
  STM32WB service at 5 characteristics. The 6th telemetry characteristic
  (**STATUS**) is therefore added **manually in code** (ST's recommended
  workaround: `aci_gatt_add_char()` + increased `max_attr_record`). Because it
  lives partly outside CubeMX user-code regions, **re-running CubeMX "Generate
  Code" will wipe parts of it** — re-apply from the firmware history if you
  regenerate, or freeze the `.ioc`.
- **Multi-board dashboard.** The web app's BLE layer is one-instance-per-device
  and event-driven, ready for the "up to 4 boards in one tab" requirement; only
  the UI is currently single-board.
- **Persistent settings.** Target speeds / cycle time reset to defaults on power
  cycle. A future revision could store them in flash.
- **Custom PCB.** A KiCad design exists under `kicad/MotorCtrl/` (SKiDL source
  `motorctrl.py`). It must be brought in line with the firmware before
  fabrication — notably the **active-HIGH button wiring** (§3.2).
- **Watchdog / fault handling.** No IWDG is enabled; consider one for unattended
  operation.

---

*Document version: Phase 2 — 2026-05-25*
