import logging
import pprint
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart, mqtt
from esphome.const import  CONF_ID, UNIT_EMPTY, ICON_EMPTY

from esphome.const import (
    CONF_VOLTAGE,
    CONF_NAME,
    UNIT_VOLT,
    UNIT_WATT,
    UNIT_KILOWATT_HOURS,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    CONF_ID,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    #LAST_RESET_TYPE_NEVER, # https://github.com/zuidwijk/dsmr/blob/main/components/dsmr/sensor.py
    # https://community.home-assistant.io/t/energy-not-showing-the-expected-entities-for-consumption/326880/2
)

DEVICE_CLASSES = [
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
]

_LOGGER = logging.getLogger(__name__)

DEPENDENCIES = ['uart']

CONF_NANOVIEW_SLOTS_POWER = "power"
CONF_NANOVIEW_SLOTS_ENERGY = "energy"
CONF_NANOVIEW_USED_SLOTS = "used_slots"
CONF_NANOVIEW_SLOT_MIN = 1
CONF_NANOVIEW_SLOT_MAX = 16
CONF_NANOVOEW_UPDATE_INTERVAL = "update_interval"
CONF_NANOVOEW_LIVE_POWER_UNIT = 'live_power_unit'
CONF_NANOVOEW_ACCUMULATED_ENERGY_UNIT = 'accumulated_energy_unit'
CONF_NANOVIEW_TEST_MODE = 'test_mode'


CONF_NANOVIEW_SLOT_NO = "slot_no"

pp = pprint.PrettyPrinter(indent=2)

nanoview_monitor_ns = cg.esphome_ns.namespace('nanoview_monitor')
sensor_ns = cg.esphome_ns.namespace('sensor')
NanoviewMonitor = nanoview_monitor_ns.class_('NanoviewMonitor', cg.Component, uart.UARTDevice)

_UNDEF = object()

# https://github.com/alecthomas/voluptuous/blob/master/README.md for examples
# CONFIG_SCHEMA = cv.ENTITY_BASE_SCHEMA.extend(cv.MQTT_COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA).extend({
CONFIG_SCHEMA = cv.ENTITY_BASE_SCHEMA.extend(cv.MQTT_COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA).extend({
    cv.GenerateID(): cv.declare_id(NanoviewMonitor),
    cv.Required(CONF_NAME): cv.string,
    cv.Optional(CONF_NANOVOEW_UPDATE_INTERVAL): cv.int_range(min=1, max=86400),
    cv.Required(CONF_NANOVIEW_USED_SLOTS): cv.int_range(min=CONF_NANOVIEW_SLOT_MIN, max=CONF_NANOVIEW_SLOT_MAX),
    cv.Optional(CONF_NANOVOEW_LIVE_POWER_UNIT, default=UNIT_KILOWATT_HOURS): UNIT_KILOWATT_HOURS,
    cv.Optional(CONF_NANOVOEW_ACCUMULATED_ENERGY_UNIT, default=UNIT_WATT): UNIT_WATT,
    cv.Optional(CONF_NANOVIEW_TEST_MODE, default=False): cv.boolean,
    cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_VOLT,
            icon="mdi:home-assistant",
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_VOLTAGE,
            state_class=STATE_CLASS_MEASUREMENT,
            #last_reset=LAST_RESET_TYPE_NEVER,
        ).extend(
            {
                cv.Required(CONF_NAME): cv.string,
            }
        ),
    cv.Optional(CONF_NANOVIEW_SLOTS_POWER): cv.Schema([
        sensor.sensor_schema(
            unit_of_measurement=UNIT_WATT,
            icon="mdi:home-assistant",
            accuracy_decimals=1,
            device_class=DEVICE_CLASS_POWER,
            state_class=STATE_CLASS_MEASUREMENT, # STATE_CLASS_TOTAL_INCREASING, STATE_CLASS_NONE, STATE_CLASS_MEASUREMENT
            #last_reset=LAST_RESET_TYPE_NEVER,
        ).extend(
            {
            }
        )
    ]),
    cv.Optional(CONF_NANOVIEW_SLOTS_ENERGY): cv.Schema([
        sensor.sensor_schema(
            unit_of_measurement=UNIT_KILOWATT_HOURS,
            icon="mdi:home-assistant",
            accuracy_decimals=3,
            device_class=DEVICE_CLASS_ENERGY,
            # Not sure if this should be STATE_CLASS_TOTAL_INCREASING
            state_class=STATE_CLASS_MEASUREMENT, # STATE_CLASS_TOTAL_INCREASING, STATE_CLASS_NONE, STATE_CLASS_MEASUREMENT
            #last_reset=LAST_RESET_TYPE_NEVER,
        ).extend(
            {
            }
        ),
    ])
})

async def to_code(config):
    # if CONF_NANOVIEW_SLOTS_POWER in config:
    #     config[CONF_NANOVIEW_SLOTS_POWER]

                # conf2 = {}
            # for key in conf:
            #     conf2[key] = conf[key]
            # conf2['id'] = str(conf['id'])+"e"

    nanoviewMonitor = cg.new_Pvariable(config[CONF_ID])
    if config[CONF_NANOVIEW_TEST_MODE]:
        _LOGGER.info("  test_mode enabled: "+str(config[CONF_NANOVIEW_TEST_MODE]))
        cg.add(nanoviewMonitor.set_emulate_uart())
    await cg.register_component(nanoviewMonitor, config)
    await uart.register_uart_device(nanoviewMonitor, config)

    _LOGGER.info("Nanoview Sensor code generation:")
    if CONF_NAME in config:
        cg.add(nanoviewMonitor.set_name(config[CONF_NAME]))
        cg.add(nanoviewMonitor.set_used_slots(config[CONF_NANOVIEW_USED_SLOTS]))
        cg.add(nanoviewMonitor.set_update_interval(config[CONF_NANOVOEW_UPDATE_INTERVAL]))
        # CONF_NANOVOEW_LIVE_POWER_UNIT
        # CONF_NANOVOEW_ACCUMULATED_ENERGY_UNIT

        _LOGGER.info("  Friendly Name: "+config[CONF_NAME])
    # Voltage Sensor
    if CONF_VOLTAGE in config:
        conf = config[CONF_VOLTAGE]
        _LOGGER.info("    Voltage Section ::")
        if CONF_NAME in conf:
            sens = await sensor.new_sensor(conf)
            cg.add(nanoviewMonitor.set_voltage_sensor(sens))
            _LOGGER.info("      Friendly Name: "+conf[CONF_NAME])
            _LOGGER.info("      Voltage Unit: "+conf["unit_of_measurement"])
            _LOGGER.info("      Voltage ID: "+str(conf[CONF_ID]))

    if CONF_NANOVIEW_SLOTS_POWER in config:
        _LOGGER.info("        Slots Section Power ::")
        for p, conf in enumerate(config[CONF_NANOVIEW_SLOTS_POWER]):
            pp.pprint(conf)
            slot_no = p+1
            sens = await sensor.new_sensor(conf)
            cg.add(nanoviewMonitor.set_live_power_sensor(sens, slot_no))
            _LOGGER.info("  Friendly Slot Name: "+conf[CONF_NAME])
            _LOGGER.info("  Slot Number: "+str(slot_no))
            _LOGGER.info("  Sensor ID: "+str(conf[CONF_ID]))
            
            # conf2 = {}
            # for key in conf:
            #     conf2[key] = conf[key]
            # conf2['id'] = str(conf['id'])+"e"

            # sens = await sensor.new_sensor(conf2)
            # cg.add(nanoviewMonitor.set_accumulated_energy_sensor(sens, slot_no))
            # _LOGGER.info("  Friendly Slot Name: "+conf2[CONF_NAME])
            # _LOGGER.info("  Slot Number: "+str(slot_no))
            # _LOGGER.info("  Sensor ID: "+str(conf2[CONF_ID]))
    
    if CONF_NANOVIEW_SLOTS_ENERGY in config:
        _LOGGER.info("        Slots Section Energy ::")
        for e, conf in enumerate(config[CONF_NANOVIEW_SLOTS_ENERGY]):
            pp.pprint(conf)
            slot_no = e+1
            sens = await sensor.new_sensor(conf)
            cg.add(nanoviewMonitor.set_accumulated_energy_sensor(sens, slot_no))
            _LOGGER.info("  Friendly Slot Name: "+conf[CONF_NAME])
            _LOGGER.info("  Slot Number: "+str(slot_no))
            _LOGGER.info("  Sensor ID: "+str(conf[CONF_ID]))

    cg.add_define("USE_NANOVIEW_MONITOR")
    cg.add_global(nanoview_monitor_ns.using)
    cg.add_global(sensor_ns.using)