# MotorCtrl BLE — STM32WB55 Dual BLDC Motor Controller

Phase 2 of a dual BLDC motor controller project. Adds BLE 5.0 wireless control and a Web Bluetooth browser dashboard to the Phase 1 firmware.

## Hardware

| Item | Detail |
|---|---|
| MCU | WeAct STM32WB55 Core Board (Cortex-M4 + BLE 5.0) |
| Motors | 2× BLDC 12V 4-wire |
| PWM output | TIM2 CH1 (M1), TIM2 CH2 (M2) — 20 kHz |
| Tachometer | TIM1 CH1 (M1), TIM1 CH2 (M2) — input capture |
| PID tick | TIM16 — 100 ms |
| Buttons | PA2 STOP, PA3 START, PA4 CONTINUE (EXTI, pull-down) |
| LED | PE4 Blue |

## BLE GATT Services

### Control Service `0x0001`
| Characteristic | UUID | Properties | Size |
|---|---|---|---|
| TargetSpeedM1 | 0x0002 | Read / Write | 2 bytes |
| TargetSpeedM2 | 0x0003 | Read / Write | 2 bytes |
| CycleTime | 0x0004 | Read / Write | 2 bytes |
| Command | 0x0005 | Write | 1 byte |

**Command values:** `0x01` = Start (timed), `0x02` = Stop, `0x03` = Start (continuous)

### Telemetry Service `0x0010`
| Characteristic | UUID | Properties | Size |
|---|---|---|---|
| ActualSpeedM1 | 0x0011 | Read / Notify | 2 bytes |
| ActualSpeedM2 | 0x0012 | Read / Notify | 2 bytes |
| RotationsM1 | 0x0013 | Read / Notify | 4 bytes |
| RotationsM2 | 0x0014 | Read / Notify | 4 bytes |
| ElapsedTime | 0x0015 | Read / Notify | 2 bytes |

## Motor Operation Modes

| Mode | Trigger | Behaviour |
|---|---|---|
| **Stopped** | Power-on / STOP button / BLE Stop command | Motors off |
| **Timed run** | START button / BLE Start command | Run for configured cycle time, then stop and report totals |
| **Continuous** | CONTINUE button / BLE Start-continuous command | Run while active, stop on STOP command |

- **6 pulses per revolution** (tachometer)
- Default cycle time: **35 seconds**
- Cumulative rotations and elapsed time reset at the start of each cycle

## Web Bluetooth Dashboard

A browser-hosted HTML/JS page that connects directly to the STM32WB55 over BLE — no app or installation required.

- **Supported browsers:** Chrome (desktop/Android), Bluefy (iOS)
- **Displays:** actual speed M1 & M2, target speed, cycle time, rotations M1 & M2, elapsed time
- **Controls:** set target speed M1/M2, set cycle time, Start (timed), Stop, Start (continuous)
- **Expandable:** designed to connect up to 4 boards simultaneously in a single browser tab

## Project Structure

```
├── Core/
│   ├── Inc/                  # Application headers (main.h, app_conf.h, …)
│   └── Src/                  # Application source (main.c, app_entry.c, …)
├── STM32_WPAN/
│   ├── App/
│   │   ├── custom_stm.c/h    # GATT service definitions
│   │   ├── custom_app.c/h    # Application callbacks & motor logic
│   │   └── app_ble.c/h       # BLE stack init & advertising
│   └── Target/
│       └── hw_ipcc.c         # Inter-processor communication
├── Drivers/                  # STM32WB HAL + CMSIS
├── Middlewares/               # STM32_WPAN BLE stack
├── Utilities/                # Low-power manager, sequencer
├── EWARM/                    # IAR Embedded Workbench project files
├── docs/                     # Schematics, specs, board datasheet
└── Phase2.ioc                # STM32CubeMX configuration
```

## Build

1. Open `EWARM/Project.eww` in **IAR Embedded Workbench for ARM**
2. Select the `Debug` or `Release` configuration
3. Build → Download and Debug

> The BLE wireless firmware (CPU2) must be flashed separately onto the STM32WB55 using STM32CubeProgrammer before running this application. Use the appropriate `stm32wb5x_BLE_Stack_full_extended_fw.bin` from STMicroelectronics.

## Phase 1 Reference

Phase 1 (STM32F103RB, no BLE) is the original firmware this project extends. Key parameters carried forward:

- M1 target: 1900 RPM, M2 target: 800 RPM
- 6 pulses/revolution, 35-second timed cycle
- PID speed control at 100 ms tick

## License

Proprietary — all rights reserved. Client project for Toon.
