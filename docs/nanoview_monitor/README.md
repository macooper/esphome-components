# Nanoview Docs
TODO

# Sample Config

```yaml
uart:
  tx_pin: 1
  rx_pin: 3
  baud_rate: 2400
  #baud_rate: 115200
  id: uart_bus



sensor:
  - platform: nanoview_monitor
    #test_mode: 'true'
    name: Electricity Monitor
    used_slots: 12 # From 1-16 for the number of sensors that are used.
    update_interval: 3 # how often the value is sent to the frontend. Minimum is 1, max is 86,400 (24 hours)
    voltage:
      name: "Mains Voltage" # Add a name for the entity displaying the mains voltage.
      unit_of_measurement: "V"
    power: # Add a name for the power entity of each used slot.
      - name: "Total Power"
      - name: "Cooker Power"
      - name: "Water Heater Power"
      - name: "Pool Pump Power"
      - name: "Garden Fountain Power"
      - name: "Bedroom Lights Power"
      - name: "Bedroom Sockets Power"
      - name: "Kitchen Sockets Power"
      - name: "Kitchen Lights Power"
      - name: "Kitchen Fridges Power"
      - name: "Cottage Power"
      - name: "Lounge Sockets Power"
      # - name: "Unused"
      # - name: "Unused"
      # - name: "Unused"
      # - name: "Unused"
    energy: # Add a name for the energy entoty of each used slot.
      - name: "Total Energy"
      - name: "Cooker Energy"
      - name: "Water Heater Energy"
      - name: "Pool Pump Energy"
      - name: "Garden Fountain Energy"
      - name: "Bedroom Lights Energy"
      - name: "Bedroom Sockets Energy"
      - name: "Kitchen Sockets Energy"
      - name: "Kitchen Lights Energy"
      - name: "Kitchen Fridges Energy"
      - name: "Cottage Energy"
      - name: "Lounge Sockets Energy"
      # - name: "Unused"
      # - name: "Unused"
      # - name: "Unused"
      # - name: "Unused"
```