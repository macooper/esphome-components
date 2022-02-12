/*
  The nanoview has 16 slots, each with a live and accumulated energy reading.
  This class represents a single slot, containing two sensors, one for each
  reading.
*/

#pragma once
#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
  namespace nanoview_monitor {
    static const char *SLOTTAG = "NanoviewSlotSensor";
    class NanoviewSlotSensor {
      public:
        NanoviewSlotSensor() {
        }

        ~NanoviewSlotSensor() {
          //delete this->live_power_;
          //delete this->accumulated_energy_;
        }
        void set_live_power(uint16_t power) { this->live_power_value_ = power; this->live_power_->publish_state(power); }
        void set_accumulated_energy(uint32_t energy) {
          this->accumulated_energy_value_ = energy;
          float energyKwH = 0;
          if (energy > 0) energyKwH = energy/1000.0;
          
          ESP_LOGD(SLOTTAG, "Converting Accumulated Energy From %d Wh to %f KwH for publishing", energy, energyKwH);
          this->accumulated_energy_->publish_state(energyKwH);
        }

        void set_live_power_sensor(sensor::Sensor *s) { this->live_power_ = s; }
        void set_accumulated_energy_sensor(sensor::Sensor *s) { this->accumulated_energy_ = s; }

      protected:
        sensor::Sensor *live_power_; 
        float live_power_value_;
        sensor::Sensor *accumulated_energy_;
        float accumulated_energy_value_;
    };
  } // namespace nanoview_monitor
} // namespace esphome