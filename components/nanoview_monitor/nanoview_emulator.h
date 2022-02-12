#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "nanoview_util.h"
#ifdef USE_HOMEASSISTANT_TIME
  #include "esphome/components/homeassistant/time/homeassistant_time.h"
#endif

#define CONF_NANOVIEW_FIRMWARE_MESSAGE_COUNT 4

/*
  NB. This is not a UART emulator, it is intended only for debugging the nanoview code.
*/
namespace esphome {
  namespace nanoview_monitor {

    class NanoviewEmulator {
      public:

        NanoviewEmulator();

        void setup();

        void loop();

        int available();
        bool read_array(uint8_t *data, size_t len);
        void set_update_interval(int period);
  
        int read(); // Read a single byte, but return it as an int
        bool read_byte(uint8_t *data);
      protected:
        void loop_();
        void update_millis_(bool updatePrevious);
#ifdef USE_HOMEASSISTANT_TIME
        time::ESPTime currentTime;
#endif
        unsigned long currentTimeMillis_;
        unsigned long currentTimeSeconds_;
        unsigned long previousTimeMillis_;
        unsigned long previousTimeSeconds_;

        int updateInterval_ = 1;

        std::list<uint8_t> serial_buffer_;
        int part_packet_length_ = 0;
        int firmware_count_{};
    };
  } // namespace nanoview_monitor
} // namespace esphome
