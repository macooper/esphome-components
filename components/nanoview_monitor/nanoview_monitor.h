#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

#include "nanoview_slot.h"
#include "nanoview_emulator.h"

namespace esphome {
  namespace nanoview_monitor {

    class NanoviewMonitor : public Component, public uart::UARTDevice {
      public:
        // See https://esphome.io/components/sensor/custom.html
        float get_setup_priority() const override { return setup_priority::LATE; }
        void setup() override;
        void loop() override;
        void dump_config() override;
        void logMessageAsHex(NanoViewFullMessage *energyDetails);
        void updateEntites(NanoViewFullMessage *energyDetails);
        NanoViewMessageType readMessageStart();
        ReadBufferState readPacket(void *dataBuffer, int length, bool verify);
        bool publishData(NanoViewFullMessage *nanoViewMessage);

        void set_name(std::string name);
        void set_used_slots(int used_slots);
        void set_update_interval(int period);
       
        /// Set force update mode.
        void set_force_update(bool force_update);

        const std::string &get_name();
        int get_used_slots();
        int get_update_interval();

         /**
         * Get whether force update mode is enabled.
         *
         * If the sensor is in force_update mode, the frontend is required to save all
         * state changes to the database when they are published, even if the state is the
         * same as before.
         */
        bool get_force_update() const;

        const std::string &get_slot_name(int slot_no);


        void set_voltage_unit(std::string unit);
        
        /*
          If called at startup, no UART will be used and emuated results will be returned.
          Experimental work in progreaa.
        */
        void set_emulate_uart();

        void set_live_power_sensor(sensor::Sensor *s, int offset) { this->slots[offset-1].set_live_power_sensor(s); }
        void set_accumulated_energy_sensor(sensor::Sensor *s, int offset) { this->slots[offset-1].set_accumulated_energy_sensor(s); }
        void set_voltage_sensor(sensor::Sensor *s) { this->voltage_ = s; }

        NanoviewMonitor();
        
      protected:
        
        bool emulate_uart();

        sensor::Sensor *voltage_;
        float voltage_value_;
        int previousBytes;
        NanoviewSlotSensor    *slots = new NanoviewSlotSensor[NANOVIEW_CHANNEL_COUNT];

        NanoViewMessageType messageType = NANOVIEW_UNSET;
        NanoViewFullMessage nanoViewMessage;
        int updateInterval = 1;
        int updateCount = 0;

        std::string name_;
        int usedSlots_ = NANOVIEW_CHANNEL_COUNT;
        std::string livePowerUnit_;
        std::string accumulatedEnergyUnit_;

        bool force_update_{false};                            ///< Force update mode
        bool emulate_uart_{false};

        int available_();
        bool read_array_(uint8_t *data, size_t len);
        int read_();

        NanoviewEmulator *nanoview_emulator_{nullptr};
    };
  } // namespace nanoview_monitor
} // namespace esphome