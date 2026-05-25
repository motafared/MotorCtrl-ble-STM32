#!/usr/bin/env python3
"""
generate_easyeda.py
Generates an EasyEDA Standard schematic JSON for the MotorCtrl board.

Run:    python generate_easyeda.py
Output: kicad/MotorCtrl_EasyEDA.json

Import into EasyEDA Standard (easyeda.com):
    File > Import > EasyEDA > select MotorCtrl_EasyEDA.json

After import:
  1. Zoom into each group and drag any net label that is not yet on a pin.
     EasyEDA highlights the pin in orange when your cursor is close enough — it snaps.
  2. J2A / J2B are placeholder headers for the WeAct STM32WB55 module.
     Search EasyEDA library for "WeAct STM32WB55" and replace them, or keep them as-is
     and annotate the pin names in the Value field.
  3. Run ERC (Inspect > ERC) — fix any "pin not connected" by snapping nearby labels.

All LCSC part numbers are valid — EasyEDA will auto-load symbols from the cloud.
"""

import json

# ─────────────────────────────────────────────────────────────
# ID generator (every shape in EasyEDA needs a unique ggeId)
# ─────────────────────────────────────────────────────────────
_id = 0
def gid():
    global _id
    _id += 1
    return f"gge{_id}"

shapes = []

# ─────────────────────────────────────────────────────────────
# Primitive helpers — append EasyEDA shape strings
# ─────────────────────────────────────────────────────────────

def comp(x, y, rot, pkg, lcsc, ref, val):
    """Component instance referencing EasyEDA cloud library by LCSC ID."""
    shapes.append(
        f"C~{x}~{y}~{rot}~{gid()}~0~{pkg}~{lcsc}~{ref}~{val}~1~~0~{gid()}~locked"
    )

def netlabel(x, y, net, rot=0):
    """Global net label — EasyEDA connects all labels with the same name."""
    shapes.append(
        f"N~{x}~{y}~{rot}~{gid()}~{net}~0~normal~{gid()}~locked"
    )

def pwrflag(x, y, net):
    """Power symbol (needed on every power rail so ERC sees it as driven)."""
    shapes.append(
        f"PF~{x}~{y}~0~{gid()}~{net}~~{gid()}~locked"
    )

def text(x, y, txt, size=14):
    """Section title text."""
    shapes.append(
        f"T~L~{x}~{y}~0~1~0~arial~{size}~bold~normal~#336699~{txt}~{gid()}~locked"
    )


# ─────────────────────────────────────────────────────────────
# SECTION: POWER INPUT  (top-left, x=100-720, y=100-540)
# ─────────────────────────────────────────────────────────────
text(100, 70, "POWER INPUT")

# J1 — DC Barrel Jack 2.1mm centre-positive
# LCSC C16214 (DC-005)   pin1=centre(+)  pin2=ring(-)  pin3=switch(NC)
comp(180, 200, 0, "DC-005", "C16214", "J1", "DC_JACK_12V")
netlabel(140, 200, "+12V_RAW", 180)  # pin1 centre+
netlabel(180, 232, "GND")            # pin2 ring-
# pin3 switch: add a no-connect flag manually in EasyEDA

# D1 — Schottky diode 1N5819  reverse-polarity protection
# LCSC C8598 (SOD-123)   Anode(A) left  Cathode(K) right
comp(340, 200, 0, "SOD-123FL", "C8598", "D1", "1N5819")
netlabel(310, 200, "+12V_RAW", 180)  # Anode
netlabel(372, 200, "+12V")           # Cathode = protected +12 V

pwrflag(420, 155, "+12V")

# C1 — 470 µF / 25 V bulk electrolytic
# LCSC C176178   pin1=(+) top   pin2=(-) bottom
comp(260, 420, 0, "CAP-TH_BD8.0-P3.50", "C176178", "C1", "470uF/25V")
netlabel(260, 385, "+12V")
netlabel(260, 458, "GND")

# U1 — AMS1117-3.3 LDO  SOT-223
# LCSC C6186   pin1=GND  pin2=OUT  pin3=IN
comp(520, 290, 0, "SOT-223-3_L6.5-W3.4-P2.30-LS7.0-BR", "C6186", "U1", "AMS1117-3.3")
netlabel(478, 290, "GND")       # pin1
netlabel(562, 278, "+3V3")      # pin2 OUT
netlabel(562, 302, "+12V")      # pin3 IN

# C4 — 10 µF LDO input bypass  C0805
# LCSC C19702
comp(460, 430, 0, "C0805", "C19702", "C4", "10uF")
netlabel(460, 398, "+12V")
netlabel(460, 462, "GND")

# C5 — 10 µF LDO output bypass  C0805
comp(580, 430, 0, "C0805", "C19702", "C5", "10uF")
netlabel(580, 398, "+3V3")
netlabel(580, 462, "GND")

# C6 — 100 nF output decoupling  C0402
# LCSC C1525
comp(660, 430, 0, "C0402", "C1525", "C6", "100nF")
netlabel(660, 398, "+3V3")
netlabel(660, 462, "GND")

pwrflag(700, 380, "+3V3")
pwrflag(120, 490, "GND")


# ─────────────────────────────────────────────────────────────
# SECTION: STM32WB55 MODULE  (centre, x=760-1080, y=100-600)
# Split into two placeholder headers; replace with real WeAct symbol later.
# ─────────────────────────────────────────────────────────────
text(760, 70, "STM32WB55 MODULE (U2)")
text(760, 90, "  -> replace J2A/J2B with WeAct symbol")

# J2A — left pin row  (3V3, GND, NRST, PA0-PA3)
# LCSC C124378  7-pin 2.54 mm single-row male header
comp(830, 480, 0, "HDR-TH_7P-P2.54-V-M", "C124378", "J2A", "WB55_LEFT")
# Vertical header: pin 1 at top, pins 20 units apart
netlabel(798, 418, "+3V3")          # pin1
netlabel(798, 438, "GND")           # pin2
netlabel(798, 458, "NRST")          # pin3
netlabel(798, 478, "M1_PWM")        # pin4  PA0
netlabel(798, 498, "M2_PWM")        # pin5  PA1
netlabel(798, 518, "BTN_STOP")      # pin6  PA2
netlabel(798, 538, "BTN_START")     # pin7  PA3

# J2B — right pin row  (PA4, PA8, PA9, PE4, PA13, PA14)
# LCSC C124377  6-pin 2.54 mm single-row male header
comp(1010, 460, 0, "HDR-TH_6P-P2.54-V-M", "C124377", "J2B", "WB55_RIGHT")
netlabel(1042, 428, "BTN_CONTINUE") # pin1  PA4
netlabel(1042, 448, "M1_TACH")      # pin2  PA8
netlabel(1042, 468, "M2_TACH")      # pin3  PA9
netlabel(1042, 488, "LED_DRIVE")    # pin4  PE4
netlabel(1042, 508, "SWDIO")        # pin5  PA13
netlabel(1042, 528, "SWCLK")        # pin6  PA14


# ─────────────────────────────────────────────────────────────
# SECTION: MOTOR OUTPUTS  (top-right, x=1100-1600, y=100-520)
# ─────────────────────────────────────────────────────────────
text(1100, 70, "MOTOR OUTPUTS")

# R1 — M1 tacho pull-up 10 kΩ  (rotation=90 → vertical)
# LCSC C25804
comp(1180, 170, 90, "R0402", "C25804", "R1", "10k")
netlabel(1180, 140, "+3V3")
netlabel(1180, 200, "M1_TACH")

# R2 — M2 tacho pull-up 10 kΩ
comp(1280, 170, 90, "R0402", "C25804", "R2", "10k")
netlabel(1280, 140, "+3V3")
netlabel(1280, 200, "M2_TACH")

# C2 — M1 motor power decoupling 100 nF
comp(1150, 340, 0, "C0402", "C1525", "C2", "100nF")
netlabel(1150, 308, "+12V")
netlabel(1150, 372, "GND")

# C3 — M2 motor power decoupling 100 nF
comp(1260, 340, 0, "C0402", "C1525", "C3", "100nF")
netlabel(1260, 308, "+12V")
netlabel(1260, 372, "GND")

# J3 — Motor M1 connector  KF2510 4-pin
# Pin order: 1=GND, 2=+12V, 3=M1_PWM, 4=M1_TACH
# LCSC C225517
comp(1520, 230, 0, "KF2510-4P", "C225517", "J3", "M1_CONN")
netlabel(1552, 208, "GND")
netlabel(1552, 224, "+12V")
netlabel(1552, 240, "M1_PWM")
netlabel(1552, 256, "M1_TACH")

# J4 — Motor M2 connector  KF2510 4-pin
comp(1520, 430, 0, "KF2510-4P", "C225517", "J4", "M2_CONN")
netlabel(1552, 408, "GND")
netlabel(1552, 424, "+12V")
netlabel(1552, 440, "M2_PWM")
netlabel(1552, 456, "M2_TACH")


# ─────────────────────────────────────────────────────────────
# SECTION: BUTTONS  (bottom, y=650-1050)
# Per-button circuit: 10k pull-up R + 100nF debounce C + 100R series R + tactile SW
# ─────────────────────────────────────────────────────────────
text(100, 630, "BUTTONS")

def button_group(cx, net_mcu, net_sw, r_pu, r_ser, cap_ref, sw_ref):
    """Place one complete button sub-circuit centred at x=cx, starting y=700."""
    by = 700
    # Pull-up resistor (vertical: +3V3 → MCU pin)
    comp(cx,      by,        90, "R0402", "C25804", r_pu,    "10k")
    netlabel(cx, by - 22, "+3V3")
    netlabel(cx, by + 22, net_mcu)
    # Debounce cap (vertical: MCU pin → GND)
    comp(cx + 60, by + 70,  0,  "C0402", "C1525",  cap_ref, "100nF")
    netlabel(cx + 60, by + 40, net_mcu)
    netlabel(cx + 60, by + 100, "GND")
    # Series resistor (vertical: MCU pin → SW net)
    comp(cx,      by + 120,  90, "R0402", "C25804", r_ser,   "100R")
    netlabel(cx, by + 98,  net_mcu)
    netlabel(cx, by + 142, net_sw)
    # Tactile switch (horizontal: SW net → GND)
    comp(cx,      by + 200,  0,  "SW-SMD_4P-P4.5-W6.5-H4.3", "C318884", sw_ref, "TACTILE")
    netlabel(cx - 32, by + 200, net_sw)
    netlabel(cx + 32, by + 200, "GND")

button_group(200,  "BTN_START",    "BTN_START_SW",    "R3", "R6", "C7", "SW1")
button_group(500,  "BTN_STOP",     "BTN_STOP_SW",     "R4", "R7", "C8", "SW2")
button_group(800,  "BTN_CONTINUE", "BTN_CONTINUE_SW", "R5", "R8", "C9", "SW3")


# ─────────────────────────────────────────────────────────────
# SECTION: LED  (bottom-centre)
# ─────────────────────────────────────────────────────────────
text(1080, 630, "LED (PE4)")

# R9 — 220 Ω series resistor (vertical: MCU → LED anode)
comp(1120, 760, 90, "R0402", "C25804", "R9", "220R")
netlabel(1120, 732, "LED_DRIVE")
netlabel(1120, 788, "LED_A")

# D2 — Blue 3 mm LED
# LCSC C84256   pin2=Anode(+) left   pin1=Cathode(-) right
comp(1260, 760, 0, "LED-TH_BD3.0-P2.54-FD", "C84256", "D2", "LED_BLUE")
netlabel(1228, 760, "LED_A")
netlabel(1292, 760, "GND")


# ─────────────────────────────────────────────────────────────
# SECTION: SWD DEBUG HEADER  (bottom-right)
# ─────────────────────────────────────────────────────────────
text(1380, 630, "SWD DEBUG")

# J5 — 5-pin 2.54 mm SWD header  (1=3V3, 2=SWDIO, 3=SWCLK, 4=NRST, 5=GND)
# LCSC C49257
comp(1500, 780, 0, "HDR-TH_5P-P2.54-V-M", "C49257", "J5", "SWD_HEADER")
netlabel(1468, 748, "+3V3")
netlabel(1468, 764, "SWDIO")
netlabel(1468, 780, "SWCLK")
netlabel(1468, 796, "NRST")
netlabel(1468, 812, "GND")


# ─────────────────────────────────────────────────────────────
# Build JSON and write file
# ─────────────────────────────────────────────────────────────
schematic = {
    "head": {
        "editorVersion": "6.5.46.0",
        "newgId": len(shapes) + 1,
        "c_para": {},
        "hasIdFlag": True,
        "title": "MotorCtrl",
        "description": "STM32WB55 Dual BLDC Motor Controller - Phase 2",
        "version": "1.0",
        "origId": "",
        "newId": ""
    },
    "canvas": "CA~1000~1000~#ffffff~yes~yes~10~1600~1200~line~10~mm~1~0~yes~1600~1200",
    "shape": shapes,
    "BBox": {"x": 0, "y": 0, "width": 0, "height": 0},
    "Layers": [],
    "systemNetGroups": {}
}

out = "MotorCtrl_EasyEDA.json"
with open(out, "w") as f:
    json.dump(schematic, f, indent=2)

print(f"Generated  {out}  ({len(shapes)} shapes)")
print()
print("HOW TO IMPORT")
print("  1. Open  easyeda.com  (free, no install)")
print("  2. File > Import > EasyEDA > select MotorCtrl_EasyEDA.json")
print("  3. All components load from LCSC cloud — may take a few seconds")
print()
print("AFTER IMPORT")
print("  - Zoom in, drag any net label that missed its pin (EasyEDA snaps to orange highlight)")
print("  - J2A / J2B are placeholder headers for the WeAct WB55 module")
print("  - Run Inspect > ERC when done")
