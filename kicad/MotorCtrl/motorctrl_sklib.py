from collections import defaultdict
from skidl import Pin, Part, Alias, SchLib, SKIDL, TEMPLATE

from skidl.pin import pin_types

SKIDL_lib_version = '0.0.1'

motorctrl = SchLib(tool=SKIDL).add_parts(*[
        Part(**{ 'name':'Barrel_Jack', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'Barrel_Jack'}), 'ref_prefix':'J', 'fplist':[''], 'footprint':'Connector_BarrelJack:BarrelJack_Horizontal', 'keywords':'DC power barrel jack connector', 'description':'DC Barrel Jack', 'datasheet':'', 'pins':[
            Pin(num='1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'C_Polarized', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C_Polarized'}), 'ref_prefix':'C', 'fplist':[''], 'footprint':'Capacitor_THT:CP_Radial_D8.0mm_P3.50mm', 'keywords':'cap capacitor', 'description':'Polarized capacitor', 'datasheet':'', 'pins':[
            Pin(num='1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'D_Schottky', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'D_Schottky'}), 'ref_prefix':'D', 'fplist':[''], 'footprint':'Diode_SMD:D_SMA', 'keywords':'diode Schottky', 'description':'Schottky diode', 'datasheet':'', 'pins':[
            Pin(num='1',name='K',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='A',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'AMS1117-3.3', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'AMS1117-3.3'}), 'ref_prefix':'U', 'fplist':['Package_TO_SOT_SMD:SOT-223-3_TabPin2', 'Package_TO_SOT_SMD:SOT-223-3_TabPin2'], 'footprint':'Package_TO_SOT_SMD:SOT-223-3_TabPin2', 'keywords':'linear regulator ldo fixed positive', 'description':'1A Low Dropout regulator, positive, 3.3V fixed output, SOT-223', 'datasheet':'http://www.advanced-monolithic.com/pdf/ds1117.pdf', 'pins':[
            Pin(num='1',name='GND',func=pin_types.PWRIN,unit=1),
            Pin(num='2',name='VO',func=pin_types.PWROUT,unit=1),
            Pin(num='3',name='VI',func=pin_types.PWRIN,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'C', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'C'}), 'ref_prefix':'C', 'fplist':[''], 'footprint':'Capacitor_SMD:C_0805_2012Metric', 'keywords':'cap capacitor', 'description':'Unpolarized capacitor', 'datasheet':'', 'pins':[
            Pin(num='1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'WeAct_STM32WB55', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'WeAct_STM32WB55'}), 'ref_prefix':'U', 'fplist':['Connector_PinHeader_2.54mm:PinHeader_2x12_P2.54mm_Vertical'], 'footprint':'Connector_PinHeader_2.54mm:PinHeader_2x12_P2.54mm_Vertical', 'keywords':'', 'description':'', 'datasheet':'', 'pins':[
            Pin(num='1',name='3V3',func=pin_types.PWRIN,unit=1),
            Pin(num='2',name='GND',func=pin_types.PWRIN,unit=1),
            Pin(num='3',name='NRST',func=pin_types.INPUT,unit=1),
            Pin(num='4',name='PA0_M1_PWM',func=pin_types.OUTPUT,unit=1),
            Pin(num='5',name='PA1_M2_PWM',func=pin_types.OUTPUT,unit=1),
            Pin(num='6',name='PA2_BTN_STOP',func=pin_types.INPUT,unit=1),
            Pin(num='7',name='PA3_BTN_START',func=pin_types.INPUT,unit=1),
            Pin(num='8',name='PA4_BTN_CONTINUE',func=pin_types.INPUT,unit=1),
            Pin(num='9',name='PA8_M1_TACH',func=pin_types.INPUT,unit=1),
            Pin(num='10',name='PA9_M2_TACH',func=pin_types.INPUT,unit=1),
            Pin(num='11',name='PE4_LED',func=pin_types.OUTPUT,unit=1),
            Pin(num='12',name='PA13_SWDIO',func=pin_types.BIDIR,unit=1),
            Pin(num='13',name='PA14_SWCLK',func=pin_types.INPUT,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'Conn_01x04', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'Conn_01x04'}), 'ref_prefix':'J', 'fplist':[''], 'footprint':'Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical', 'keywords':'connector', 'description':'Generic connector, single row, 01x04, script generated (kicad-library-utils/schlib/autogen/connector/)', 'datasheet':'', 'pins':[
            Pin(num='1',name='Pin_1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='Pin_2',func=pin_types.PASSIVE,unit=1),
            Pin(num='3',name='Pin_3',func=pin_types.PASSIVE,unit=1),
            Pin(num='4',name='Pin_4',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'R', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'R'}), 'ref_prefix':'R', 'fplist':[''], 'footprint':'Resistor_SMD:R_0805_2012Metric', 'keywords':'R res resistor', 'description':'Resistor', 'datasheet':'', 'pins':[
            Pin(num='1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'SW_Push', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'SW_Push'}), 'ref_prefix':'SW', 'fplist':[''], 'footprint':'Button_Switch_THT:SW_PUSH_6mm', 'keywords':'switch normally-open pushbutton push-button', 'description':'Push button switch, generic, two pins', 'datasheet':'', 'pins':[
            Pin(num='1',name='1',func=pin_types.PASSIVE),
            Pin(num='2',name='2',func=pin_types.PASSIVE)], 'unit_defs':[] }),
        Part(**{ 'name':'LED', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'LED'}), 'ref_prefix':'D', 'fplist':[''], 'footprint':'LED_SMD:LED_0805_2012Metric', 'keywords':'LED diode', 'description':'Light emitting diode', 'datasheet':'', 'pins':[
            Pin(num='1',name='K',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='A',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] }),
        Part(**{ 'name':'Conn_01x05', 'dest':TEMPLATE, 'tool':SKIDL, 'aliases':Alias({'Conn_01x05'}), 'ref_prefix':'J', 'fplist':[''], 'footprint':'Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical', 'keywords':'connector', 'description':'Generic connector, single row, 01x05, script generated (kicad-library-utils/schlib/autogen/connector/)', 'datasheet':'', 'pins':[
            Pin(num='1',name='Pin_1',func=pin_types.PASSIVE,unit=1),
            Pin(num='2',name='Pin_2',func=pin_types.PASSIVE,unit=1),
            Pin(num='3',name='Pin_3',func=pin_types.PASSIVE,unit=1),
            Pin(num='4',name='Pin_4',func=pin_types.PASSIVE,unit=1),
            Pin(num='5',name='Pin_5',func=pin_types.PASSIVE,unit=1)], 'unit_defs':[] })])