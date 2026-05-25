# MotorCtrl BLE — STM32WB55 Dual BLDC Motor Controller (Phase 2)

Wireless control and live telemetry for a **dual 12 V BLDC motor controller**, built on the
**WeAct STM32WB55** (Cortex-M4 + Bluetooth LE 5.0). Phase 2 adds BLE control plus a
**Web Bluetooth dashboard** — drive the motors and watch live data from a phone or laptop, with
no app install. The original push-button control still works in parallel.

> 📄 **Full delivery document:** [`docs/phase2_delivery.md`](docs/phase2_delivery.md)
> ([styled HTML](docs/phase2_delivery.html)) — scope, BOM, wiring, flashing, GATT reference,
> dashboard usage, troubleshooting.

## Repository layout (monorepo)

```
Phase2-Project/
├── firmware/   STM32WB55 application (STM32CubeIDE) + BLE GATT services
├── WebApp/     Web Bluetooth dashboard (single-page, no install)
├── kicad/      Hardware design (KiCad, generated from SKiDL)
└── docs/       Delivery doc + connection diagrams
```

## At a glance

- **Two BLDC motors**, closed-loop PID speed control (100 ms tick), tachometer feedback.
- **Three run modes:** Timed run (default 35 s) · Continuous (run while held) · Stopped.
- **Live telemetry:** actual speed, cumulative rotations, elapsed time, and run state.
- **Buttons + BLE in parallel** — the dashboard tracks physical button presses live.

## Hardware

| Item | Detail |
|---|---|
| MCU | WeAct STM32WB55 Core Board (STM32WB55CGU6, 64 MHz, BLE 5.0) |
| Motors | 2× BLDC 12 V 4-wire |
| PWM | PA0 (M1) / PA1 (M2) — TIM2, 20 kHz |
| Tachometer | PA8 (M1) / PA9 (M2) — TIM1 input capture |
| PID tick | TIM16 — 100 ms |
| Buttons | PA2 STOP, PA3 START, PA4 CONTINUE — EXTI, **active-HIGH, internal pull-down** |
| Status LED | PE4 (blue) — ON running, 5 Hz heartbeat |

## BLE GATT

Advertised name **`MotorCtrl`**. All multi-byte values little-endian.

**Control service `0x0001`**

| Char | UUID | Type | Access | Meaning |
|---|---|---|---|---|
| TSM1 | 0x0002 | u16 | R/W | Target speed M1 (RPM) |
| TSM2 | 0x0003 | u16 | R/W | Target speed M2 (RPM) |
| CT | 0x0004 | u16 | R/W | Cycle time (s) |
| CMD | 0x0005 | u8 | W | `0x01` start timed · `0x02` stop · `0x03` start continuous |

**Telemetry service `0x0010`** (Read / Notify)

| Char | UUID | Type | Meaning |
|---|---|---|---|
| ASM1 / ASM2 | 0x0011 / 0x0012 | u16 | Actual speed M1 / M2 (RPM) |
| RM1 / RM2 | 0x0013 / 0x0014 | u32 | Cumulative rotations M1 / M2 |
| ET | 0x0015 | u16 | Elapsed time (s) |
| STATUS | 0x0016 | u8 | Run state: `0` idle · `1` timed · `2` continuous |

Reads return live values (refreshed from firmware state on each read). STATUS is notified on
every transition — BLE command, timed auto-stop, **and** physical button presses.

## Build & flash (firmware)

This project is **STM32CubeIDE only**.

1. **One-time:** flash the CPU2 BLE stack with STM32CubeProgrammer — `stm32wb5x_FUS_fw.bin` at
   `0x080EC000` (skip if FUS ≥ 1.2.0), then `stm32wb5x_BLE_Stack_full_fw.bin` at `0x080C0000`.
2. Open `firmware/` in STM32CubeIDE → **Build All** → **Run** to flash the application.

**Build modes** — one switch in `firmware/Core/Inc/main.h`:

| `MOTOR_MODE` | Tacho PPR | PWM polarity | Use |
|---|---|---|---|
| `MODE_KHALED` | 2 | non-inverted | bench testing |
| `MODE_TOON` | 6 | inverted | client hardware |

Defaults: target 1200 RPM/motor, 35 s cycle, PID `Kp = 0.012`.

## Run the dashboard

```bash
cd WebApp
python -m http.server 8000      # then open http://localhost:8000 in Chrome
```

- **Browsers:** Chrome / Edge / Brave (desktop, Android); **Bluefy** on iOS.
- Web Bluetooth needs a secure context (`https://` or `http://localhost`); `file://` won't work.
- After flashing new firmware, **hard-reload** the page (Ctrl+Shift+R) to avoid a cached script.

## License

Proprietary — all rights reserved. Client project for Toon.
