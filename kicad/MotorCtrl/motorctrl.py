"""
MotorCtrl — STM32WB55 dual-BLDC carrier board.

SKiDL description of the interface board for the Phase 2 firmware.
Pin assignments mirror Core/Inc/main.h.

Run:
    pip install skidl
    python motorctrl.py

Outputs:
    motorctrl.net  — KiCad netlist (import into pcbnew via File > Import > Netlist)
"""

import sys
import os
import glob as _glob

sys.path.insert(0, r'E:\projects\claude-hw-lib')

# Ensure SKiDL can find KiCad symbol libraries regardless of shell env vars.
_kicad_sym_dir = r'C:\Program Files\KiCad\10.0\share\kicad\symbols'
os.environ.setdefault('KICAD_SYMBOL_DIR',  _kicad_sym_dir)
os.environ.setdefault('KICAD9_SYMBOL_DIR', _kicad_sym_dir)
os.environ.setdefault('KICAD8_SYMBOL_DIR', _kicad_sym_dir)
os.environ.setdefault('KICAD7_SYMBOL_DIR', _kicad_sym_dir)

from skidl import Part, Net, generate_netlist, generate_schematic, ERC, set_default_tool, KICAD

from parts.weact_stm32wb55      import make_wb55
from blocks.debounced_button    import debounced_button
from blocks.motor_connector_4wire import motor_connector_4wire
from blocks.linear_3v3_supply   import linear_3v3_supply
from blocks.reverse_polarity_schottky import reverse_polarity_schottky
from utils.ref_allocator        import RefAllocator

set_default_tool(KICAD)


# ---------------------------------------------------------------------------
# Nets
# ---------------------------------------------------------------------------
v12_raw = Net('+12V_RAW')
v12     = Net('+12V')
v3v3    = Net('+3V3')
gnd     = Net('GND')

m1_pwm  = Net('M1_PWM')
m2_pwm  = Net('M2_PWM')
m1_tach = Net('M1_TACH')
m2_tach = Net('M2_TACH')

btn_start    = Net('BTN_START')
btn_stop     = Net('BTN_STOP')
btn_continue = Net('BTN_CONTINUE')
btn_start_sw    = Net('BTN_START_SW')
btn_stop_sw     = Net('BTN_STOP_SW')
btn_continue_sw = Net('BTN_CONTINUE_SW')

led_drive = Net('LED_DRIVE')
led_a     = Net('LED_A')

swdio = Net('SWDIO')
swclk = Net('SWCLK')
nrst  = Net('NRST')

v12.drive = 5
gnd.drive = 5


# ---------------------------------------------------------------------------
# Power input
# ---------------------------------------------------------------------------
J1 = Part('Connector', 'Barrel_Jack', ref='J1', value='12V_DC_Jack',
          footprint='Connector_BarrelJack:BarrelJack_Horizontal')
J1[1] += v12_raw
J1[2] += gnd

C_bulk = Part('Device', 'C_Polarized', value='470uF/25V',
              footprint='Capacitor_THT:CP_Radial_D8.0mm_P3.50mm')
C_bulk[1] += v12
C_bulk[2] += gnd

D1 = reverse_polarity_schottky(v12_raw, v12, d_ref='D1')
U1, C_ldo_in, C_ldo_out, C_ldo_dec = linear_3v3_supply(v12, v3v3, gnd, u_ref='U1')


# ---------------------------------------------------------------------------
# WB55 module
# ---------------------------------------------------------------------------
U2 = make_wb55(ref='U2')
U2['3V3']               += v3v3
U2['GND']               += gnd
U2['NRST']              += nrst
U2['PA0_M1_PWM']        += m1_pwm
U2['PA1_M2_PWM']        += m2_pwm
U2['PA2_BTN_STOP']      += btn_stop
U2['PA3_BTN_START']     += btn_start
U2['PA4_BTN_CONTINUE']  += btn_continue
U2['PA8_M1_TACH']       += m1_tach
U2['PA9_M2_TACH']       += m2_tach
U2['PE4_LED']           += led_drive
U2['PA13_SWDIO']        += swdio
U2['PA14_SWCLK']        += swclk


# ---------------------------------------------------------------------------
# Motor connectors
# ---------------------------------------------------------------------------
J3, C_m1_dec, R_m1_pu = motor_connector_4wire(
    m1_pwm, m1_tach, v12, v3v3, gnd, j_ref='J3', j_value='Motor_M1')

J4, C_m2_dec, R_m2_pu = motor_connector_4wire(
    m2_pwm, m2_tach, v12, v3v3, gnd, j_ref='J4', j_value='Motor_M2')


# ---------------------------------------------------------------------------
# Buttons
# ---------------------------------------------------------------------------
SW1, R_start_pu, R_start_se, C_start_db = debounced_button(
    btn_start, btn_start_sw, v3v3, gnd, sw_ref='SW1', sw_value='START')

SW2, R_stop_pu, R_stop_se, C_stop_db = debounced_button(
    btn_stop, btn_stop_sw, v3v3, gnd, sw_ref='SW2', sw_value='STOP')

SW3, R_cont_pu, R_cont_se, C_cont_db = debounced_button(
    btn_continue, btn_continue_sw, v3v3, gnd, sw_ref='SW3', sw_value='CONTINUE')


# ---------------------------------------------------------------------------
# Status LED on PE4
# ---------------------------------------------------------------------------
R_led = Part('Device', 'R', value='1k',
             footprint='Resistor_SMD:R_0805_2012Metric')
D2 = Part('Device', 'LED', ref='D2', value='LED_BLUE',
          footprint='LED_SMD:LED_0805_2012Metric')
R_led[1] += led_drive
R_led[2] += led_a
D2[2]    += led_a
D2[1]    += gnd


# ---------------------------------------------------------------------------
# SWD header  (1=3V3, 2=SWDIO, 3=SWCLK, 4=NRST, 5=GND)
# ---------------------------------------------------------------------------
J5 = Part('Connector_Generic', 'Conn_01x05', ref='J5', value='SWD',
          footprint='Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical')
J5[1] += v3v3
J5[2] += swdio
J5[3] += swclk
J5[4] += nrst
J5[5] += gnd


# ---------------------------------------------------------------------------
# Annotate all anonymous parts (those still carrying '?' in their ref)
# ---------------------------------------------------------------------------
refs = RefAllocator()
refs.annotate(
    C_bulk,
    C_ldo_in, C_ldo_out, C_ldo_dec,
    C_m1_dec, R_m1_pu,
    C_m2_dec, R_m2_pu,
    R_start_pu, R_start_se, C_start_db,
    R_stop_pu,  R_stop_se,  C_stop_db,
    R_cont_pu,  R_cont_se,  C_cont_db,
    R_led,
)


# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------
ERC()
generate_netlist(file_='motorctrl.net')
generate_schematic(
    file_='motorctrl_auto.kicad_sch',
    retries=5,
    auto_stub=True,
    auto_stub_fallback='labels',
)

# ---------------------------------------------------------------------------
# Post-process: SKiDL writes the full Windows path as the lib_id for any
# part loaded from a local .kicad_sym file (e.g. "E:\...\WeAct_STM32WB55:...").
# KiCad 10 rejects this because the drive letter makes the nickname invalid.
# Replace every such path-based lib_id with the short nickname defined in
# the project's sym-lib-table.
# ---------------------------------------------------------------------------
_SCH_GLOB = _glob.glob('*.kicad_sch')
_LIB_RENAMES = {
    r'E:\projects\claude-hw-lib\sym\WeAct_STM32WB55:WeAct_STM32WB55':
        'WeAct_Modules:WeAct_STM32WB55',
}
for _sch_path in _SCH_GLOB:
    _txt = open(_sch_path, encoding='utf-8').read()
    _changed = False
    for _old, _new in _LIB_RENAMES.items():
        if _old in _txt:
            _txt = _txt.replace(_old, _new)
            _changed = True
    if _changed:
        open(_sch_path, 'w', encoding='utf-8').write(_txt)
        print(f'INFO: Fixed lib_ids in {_sch_path}')
