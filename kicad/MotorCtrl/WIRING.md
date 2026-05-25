# MotorCtrl Schematic — Wiring Checklist

Work through each net below and draw wires (or place labels) in KiCad's schematic editor. Tick off each net as you complete it. ERC clean = done.

**Conventions:**
- Pin numbers come from the schematic symbol pin numbers (not MCU-package pins).
- For each net, the simplest approach in KiCad is to **place a global label** with the net name on each listed pin — KiCad connects them by name. No long wires needed.
- For power nets (`+12V`, `+3V3`, `GND`) use the matching **power symbol**, not a label.

---

## Power rails

### Net: `+12V`  *(use `+12V` power symbol on each pin)*
- [ ] **J1.1** — barrel jack (center +)  → goes to **D1.2 (A)** via short wire (this is the only wire-connection in the +12V chain — series reverse-polarity diode)
- [ ] **D1.1 (K)** — Schottky cathode = protected +12V (place +12V power symbol here)
- [ ] **C1.1** — 470 µF bulk cap, + side
- [ ] **C4.1** — 10 µF LDO input cap, + side
- [ ] **U1.3 (VI)** — AMS1117 input
- [ ] **C2.1** — 100 nF M1 decoupling
- [ ] **C3.1** — 100 nF M2 decoupling
- [ ] **J3.2** — motor M1 connector +12V pin
- [ ] **J4.2** — motor M2 connector +12V pin
- [ ] Place one **PWR_FLAG** symbol on this rail (so ERC sees it as driven)

### Net: `+3V3`  *(use `+3V3` power symbol on each pin)*
- [ ] **U1.2 (VO)** — AMS1117 output
- [ ] **C5.1** — 10 µF output cap, + side
- [ ] **C6.1** — 100 nF output decoupling
- [ ] **U2.1 (3V3)** — WB55 module 3V3
- [ ] **R1.1** — M1 tacho pull-up
- [ ] **R2.1** — M2 tacho pull-up
- [ ] **R3.1** — BTN_START pull-up
- [ ] **R4.1** — BTN_STOP pull-up
- [ ] **R5.1** — BTN_CONTINUE pull-up
- [ ] **J5.1** — SWD header 3V3
- [ ] Place one **PWR_FLAG** symbol on this rail

### Net: `GND`  *(use `GND` power symbol on each pin)*
- [ ] **J1.2** — barrel jack ring (−)
- [ ] **J1.3** — switch contact (tie to GND, or leave NC + add no-connect flag)
- [ ] **C1.2** — 470 µF, − side
- [ ] **U1.1 (GND)** — AMS1117 ground
- [ ] **C4.2** — 10 µF input cap, − side
- [ ] **C5.2** — 10 µF output cap, − side
- [ ] **C6.2** — 100 nF output decoupling
- [ ] **C2.2** — M1 decoupling
- [ ] **C3.2** — M2 decoupling
- [ ] **U2.2 (GND)** — WB55 module ground
- [ ] **J3.1** — motor M1 GND
- [ ] **J4.1** — motor M2 GND
- [ ] **SW1.1** — START button cold side
- [ ] **SW2.1** — STOP button cold side
- [ ] **SW3.1** — CONTINUE button cold side
- [ ] **C7.2** — START debounce cap
- [ ] **C8.2** — STOP debounce cap
- [ ] **C9.2** — CONTINUE debounce cap
- [ ] **D2.1 (K)** — LED cathode
- [ ] **J5.5** — SWD GND
- [ ] Place one **PWR_FLAG** symbol on this rail

---

## Motor signals

### Net: `M1_PWM`  *(label both pins `M1_PWM`)*
- [ ] **U2.4 (PA0/M1_PWM)**
- [ ] **J3.3** — motor M1 connector PWM pin

### Net: `M2_PWM`  *(label both pins `M2_PWM`)*
- [ ] **U2.5 (PA1/M2_PWM)**
- [ ] **J4.3** — motor M2 connector PWM pin

### Net: `M1_TACH`  *(label all three `M1_TACH`)*
- [ ] **U2.9 (PA8/M1_TACH)**
- [ ] **R1.2** — pull-up bottom side
- [ ] **J3.4** — motor M1 tacho pin

### Net: `M2_TACH`  *(label all three `M2_TACH`)*
- [ ] **U2.10 (PA9/M2_TACH)**
- [ ] **R2.2** — pull-up bottom side
- [ ] **J4.4** — motor M2 tacho pin

---

## Buttons (each has 2 nets)

Topology per button: pull-up R from MCU pin to +3V3, cap from MCU pin to GND, series R between switch and MCU pin, switch to GND.

### START button (PA3)
**Net: `BTN_START`** *(label these pins `BTN_START`)*
- [ ] **U2.7 (PA3/BTN_START)**
- [ ] **R3.2** — pull-up bottom
- [ ] **C7.1** — debounce cap top
- [ ] **R6.1** — series resistor, MCU side

**Net: `BTN_START_SW`** *(internal node between series R and switch)*
- [ ] **R6.2** — series resistor, switch side
- [ ] **SW1.2** — switch hot side

### STOP button (PA2)
**Net: `BTN_STOP`**
- [ ] **U2.6 (PA2/BTN_STOP)**
- [ ] **R4.2**
- [ ] **C8.1**
- [ ] **R7.1**

**Net: `BTN_STOP_SW`**
- [ ] **R7.2**
- [ ] **SW2.2**

### CONTINUE button (PA4)
**Net: `BTN_CONTINUE`**
- [ ] **U2.8 (PA4/BTN_CONTINUE)**
- [ ] **R5.2**
- [ ] **C9.1**
- [ ] **R8.1**

**Net: `BTN_CONTINUE_SW`**
- [ ] **R8.2**
- [ ] **SW3.2**

---

## LED (PE4)

### Net: `LED_DRIVE`  *(label both `LED_DRIVE`)*
- [ ] **U2.11 (PE4/LED)**
- [ ] **R9.1** — series resistor MCU side

### Net: `LED_A`  *(label both `LED_A`)*
- [ ] **R9.2** — series resistor LED side
- [ ] **D2.2 (A)** — LED anode

(D2.1 cathode → GND, already in GND list above.)

---

## SWD programming/debug

### Net: `SWDIO`
- [ ] **U2.12 (PA13/SWDIO)**
- [ ] **J5.2**

### Net: `SWCLK`
- [ ] **U2.13 (PA14/SWCLK)**
- [ ] **J5.3**

### Net: `NRST`
- [ ] **U2.3 (NRST)**
- [ ] **J5.4**

(J5.1 = +3V3, J5.5 = GND — already covered.)

---

## After wiring — ERC checklist

1. **Inspect → Electrical Rules Checker → Run ERC**.
2. Expected to pass clean. If not, common causes:
   - **"Pin not connected"** → missed a pin in the lists above.
   - **"Power pin not driven"** → a `+12V` / `+3V3` / `GND` rail is missing its `PWR_FLAG`.
   - **"Label dangling"** → a label was placed near but not on a pin endpoint. Drag it onto the pin tip.
   - **"Lib symbol mismatch"** → can be ignored (KiCad noticed our embedded symbols differ slightly from its installed library copies).
3. Cross-check pin assignments visually against `Core/Inc/main.h:65-83`.

## Net summary count

13 nets total: `+12V`, `+3V3`, `GND`, `M1_PWM`, `M2_PWM`, `M1_TACH`, `M2_TACH`, `BTN_START`, `BTN_START_SW`, `BTN_STOP`, `BTN_STOP_SW`, `BTN_CONTINUE`, `BTN_CONTINUE_SW`, `LED_DRIVE`, `LED_A`, `SWDIO`, `SWCLK`, `NRST` → **18 nets** (3 power + 15 signal).
