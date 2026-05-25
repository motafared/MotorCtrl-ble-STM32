# MotorCtrl — Connections to the STM32 Board

Wiring of the **STM32WB55** carrier board (Phase 2), one connection group per
diagram. Pins mirror the firmware map in `Core/Inc/main.h`; source of truth is
the SKiDL design [motorctrl.py](motorctrl.py).

Board MCU module: **WeAct STM32WB55CGU6** (`U2`). All `3V3` logic; motors run on
the `+12V` rail.

---

## 1. STM32 ↔ Motors

Two 4-wire BLDC motors on connectors `J3` (M1) and `J4` (M2). PWM drives the
motor; the tacho returns one pulse train per revolution.

```
   Motor M1  (J3, 4-wire)            STM32WB55 board (U2)
   ┌─────────────────────┐          ┌──────────────────────────┐
   │ +12V   (J3.2)       ├── +12V ───┤ +12V rail                │
   │ GND    (J3.1)       ├── GND  ───┤ GND                      │
   │ PWM    (J3.3)       ├◄──────────┤ PA0   M1_PWM             │ TIM2_CH1
   │ Tacho  (J3.4)       ├──────────►┤ PA8   M1_TACH            │ TIM1_CH1
   └─────────────────────┘          └──────────────────────────┘

   Motor M2  (J4, 4-wire)            STM32WB55 board (U2)
   ┌─────────────────────┐          ┌──────────────────────────┐
   │ +12V   (J4.2)       ├── +12V ───┤ +12V rail                │
   │ GND    (J4.1)       ├── GND  ───┤ GND                      │
   │ PWM    (J4.3)       ├◄──────────┤ PA1   M2_PWM             │ TIM2_CH2
   │ Tacho  (J4.4)       ├──────────►┤ PA9   M2_TACH            │ TIM1_CH2
   └─────────────────────┘          └──────────────────────────┘
```

- Each tacho line has a pull-up to `+3V3` on the board: `R1` (M1), `R2` (M2).
- PWM is 20 kHz (TIM2); tacho is timer input-capture (TIM1).

---

## 2. STM32 ↔ Buttons

Three momentary push-buttons. Each is debounced on the board: pull-up to `+3V3`,
series resistor to the switch, and a cap to `GND`. The switch pulls the MCU pin
low when pressed.

```
   Push-buttons                      STM32WB55 board (U2)
   ┌─────────────────────┐          ┌──────────────────────────┐
   │ START   SW1 ─[R6]───├──────────►┤ PA3   BTN_START          │ EXTI3
   │ STOP    SW2 ─[R7]───├──────────►┤ PA2   BTN_STOP           │ EXTI2
   │ CONT    SW3 ─[R8]───├──────────►┤ PA4   BTN_CONTINUE       │ EXTI4
   │ (cold side → GND)   │          │                          │
   └─────────────────────┘          └──────────────────────────┘
```

- Per pin on the board: pull-up `R3/R4/R5` → `+3V3`, debounce cap `C7/C8/C9` → `GND`.
- Idle = high (3V3); pressed = low. All three fire EXTI interrupts.

---

## 3. STM32 ↔ Status LED

```
   STM32WB55 board (U2)              Status LED
   ┌──────────────────────────┐     ┌──────────────┐
   │ PE4   LED_DRIVE ─[R9 1k]──├────►┤ D2 anode     │
   │ GND ──────────────────────├─────┤ D2 cathode   │
   └──────────────────────────┘     └──────────────┘
```

- `PE4` high → LED on (motors running / heartbeat). `R9` = 1 kΩ series.

---

## 4. STM32 ↔ SWD programmer

5-pin header `J5` for flashing/debug (ST-LINK).

```
   SWD header (J5, 1×5)              STM32WB55 board (U2)
   ┌─────────────────────┐          ┌──────────────────────────┐
   │ +3V3   (J5.1)       ├── 3V3 ────┤ +3V3                     │
   │ SWDIO  (J5.2)       ├──────────►┤ PA13  SWDIO              │
   │ SWCLK  (J5.3)       ├──────────►┤ PA14  SWCLK              │
   │ NRST   (J5.4)       ├──────────►┤ NRST                     │
   │ GND    (J5.5)       ├── GND ────┤ GND                      │
   └─────────────────────┘          └──────────────────────────┘
```

---

## 5. Power input

12 V DC barrel jack → reverse-polarity Schottky → on-board regulator.

```
   12 V DC jack (J1)        Board power stage                  Rails
   ┌─────────────────┐      ┌──────────────────────────┐      ┌──────────┐
   │ +12V   (J1.1)   ├─ D1 ─┤ Schottky → +12V rail       ├────►│ +12V     │
   │                 │      │ AMS1117 (U1): 12V → 3.3V   ├────►│ +3V3     │
   │ GND    (J1.2)   ├──────┤ common ground              ├────►│ GND      │
   └─────────────────┘      └──────────────────────────┘      └──────────┘
```

- `+12V` rail → motors (`J3.2`/`J4.2`) + decoupling. `+3V3` → `U2`, pull-ups, SWD.

---

For the pin-by-pin schematic-drawing checklist see [WIRING.md](WIRING.md);
for the rendered version open [connections.html](connections.html).
