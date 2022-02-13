# Nanoview Docs
TODO

# Sample Config

```yaml
#debug:

# Enable logging
logger:
  #level: DEBUG
  level: INFO
  baud_rate: 0 # Disable logging which uses serial port so it can be used for the nanoview. See hardware docs, there are some gotchas with this.
  hardware_uart: UART1 # Probably not needed

esphome:
  name: wemostest
  platform: ESP8266
  board: d1_mini

# Used during testing
#external_components:
#  - source:
#      type: local
#      path: esphome-components/components

# see https://esphome.io/components/external_components.html for details
external_components:
  - source: github://macooper/esphome-components

uart:
  tx_pin: 1
  rx_pin: 3
  baud_rate: 2400
  id: uart_bus

sensor:
  - platform: nanoview_monitor
    #test_mode: 'true' # If uncommented, the built in emulater will be used which generates random data for all values.
    name: Electricity Monitor
    used_slots: 13 # From 1-16 for the sensors that are used.
    update_interval: 3 # how often the value is sent to the frontend. Minimum is 1, max is 86,400 (24 hours)
    voltage:
      name: "Mains Voltage"
    power: # Add a name for each slot.  If missing, slot_<number> will be used.
      - name: "Total Power"
      - name: "Cooker Power"
      - name: "Cottage Power"
      - name: "Unknown Power"
      - name: "Dining / Living Area Plugs Power"
      - name: "Bedroom Plugs Power"
      - name: "Kitchen Plugs Power"
      - name: "Not Sure Power??"
      - name: "Office, Garage Sockets Power"
      - name: "Living Area Lights Power"
      - name: "Bedroom Lights Power"
      - name: "Pool Pump Power"
      - name: "Water Heater Energy"
      # - name: "Security Systems Power"
      # - name: "Dining Area Sockets Power"
      # - name: "Garden Hut Power"
      # - name: "Office Sockets Power"
    energy: # Add a name for each slot.  If missing, slot_<number> will be used.
      - name: "Total Energy"
      - name: "Cooker Energy"
      - name: "Cottage Energy"
      - name: "Unknown Energy"
      - name: "Dining / Living Area Energy"
      - name: "Bedroom Sockets Energy"
      - name: "Kitchen Sockets Energy"
      - name: "Not Sure Energy??"
      - name: "Office, Garage Sockets Energy"
      - name: "Living Area Lights Energy"
      - name: "Bedroom Lights Energy"
      - name: "Pool Pump Energy"
      - name: "Water Heater Energy"
      # - name: "Security Systems Power"
      # - name: "Dining Area Sockets Power"
      # - name: "Garden Hut Power"
      # - name: "Office Sockets Power"

# Enable Home Assistant API
api:

ota:
  password: !secret wemostest_ota

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

  # Enable fallback hotspot (captive portal) in case wifi connection fails
  ap:
    ssid: "Wemostest Fallback Hotspot"
    password: !secret fallback_hotspot_password

captive_portal:

```

In addition to the above, you'll also need passwords in the secrets file like this, obviously, you'll need to use your own passwords.
```
wifi_ssid: "myssid"
wifi_password: "wifipass"
ota_update: "0344fdt6rgr55rtr"
fallback_hotspot_password: "gefdvgernhmj"
mqtt_username: "mosquito"
mqtt_password: "gefdvgernhmj"
wemostest_ota: "0344fdt6rgr55rtr"
```