#include <stdlib.h>     /* srand, rand */
#include <time.h>

#include "esphome/core/log.h"
#include "nanoview_emulator.h"
#include "esphome/core/hal.h"
//#include "esphome/components/time/real_time_clock.h"

namespace {
  static const char *TAG = "nanoview_emulator_local";
  
}

namespace esphome
{
  namespace nanoview_monitor
  {

    static const char *TAG = "NanoviewEmulator";

#ifdef USE_HOMEASSISTANT_TIME
    /*
      Examples of usage of time (time is taken from Home Assistant)

      struct tm c_time = this->currentTime.to_c_tm(); // Gets a C style TM structure
      std::string this->currentTime.strftime("%A %d %B %Y %H%M%S");
      ESP_LOGI(TAG, "Current Time %s", asctime(&c_time));
      ESP_LOGI(TAG, "Current Time %s" , this->currentTime.strftime("%A %d %B %Y %H%M%S").c_str());
      ESP_LOGI(TAG, "Current Timestamp %d" , this->currentTime.timestamp); // Standard timestamp, as in seconds since midnight on Jan 1st 1970
    */
    void set_time_(time::ESPTime *currentTime){
      *currentTime = homeassistant::global_homeassistant_time->now();
    };
#endif

    NanoviewEmulator::NanoviewEmulator()
    {
    }

    void NanoviewEmulator::loop_() {
      unsigned long start = millis();
      generateNanoviewFullMessage(&this->serial_buffer_, this->firmware_count_ <= CONF_NANOVIEW_FIRMWARE_MESSAGE_COUNT);
      unsigned long end = millis();
      ESP_LOGI(TAG, "Generating data with generateNanoviewFullMessage took %d ms" , end - start);
      logBuffer(&this->serial_buffer_);
      this->firmware_count_++;
    };

    void NanoviewEmulator::setup() {
      //std::srand(std::time(NULL)); // This doesn't work in setup. Not sure why
      std::srand(millis() * 52136);
      
      ESP_LOGI(TAG, "List Size %d", this->serial_buffer_.size());

      this->part_packet_length_ = generatePartPacket(&this->serial_buffer_);
    };

    void NanoviewEmulator::loop() {
      this->update_millis_(false);
#ifdef USE_HOMEASSISTANT_TIME
      set_time_(&this->currentTime);
#endif
      
      /*
        The real nanoview sends a full set of sensor readings once a second. The code takes advantage of
        this fact to avoid using timed components
      */
      if (this->updateInterval_ <= (this->currentTimeSeconds_ - this->previousTimeSeconds_)) {
#ifdef USE_HOMEASSISTANT_TIME
        ESP_LOGI(TAG, "Current Time %s" , this->currentTime.strftime("%A %d %B %Y %H%M%S").c_str());
#endif
        ESP_LOGI(TAG, "Current Time seconds %d" , this->currentTimeSeconds_);
        ESP_LOGI(TAG, "Previous Time seconds %d" , this->previousTimeSeconds_);
        ESP_LOGI(TAG, "Update Interval %d" , this->updateInterval_);
        ESP_LOGI(TAG, "Part Packet Length %d" , this->part_packet_length_);
        logHeapSpace();
        loop_();
        this->update_millis_(true);
      }
    };

    void NanoviewEmulator::update_millis_(bool updatePrevious){
      uint32_t updatedMillis = millis();
      // As millis will wrap every 49.71 days, we need to allow for millis being reset.
      // This will force the previousXX settings to be updated when that occurs.
      // Will likely never happen in SA thanks to Eskom resets ;)
      if (this->currentTimeMillis_ > updatedMillis) updatePrevious = true;
      this->currentTimeMillis_ = updatedMillis;
      this->currentTimeSeconds_ = this->currentTimeMillis_/1000;
      if (updatePrevious) {
        this->previousTimeMillis_ = this->currentTimeMillis_;
        this->previousTimeSeconds_ = this->currentTimeSeconds_;
      }
    };

    void NanoviewEmulator::set_update_interval(int period) {
      this->updateInterval_ = period;
    };

    int NanoviewEmulator::available() {
      int result = this->serial_buffer_.size();
      // Uncomment this to disable the nanoview_monitor processing for debugging purposes.
      //result = 0;
      return result;
    };

    bool NanoviewEmulator::read_array(uint8_t *data, size_t len) {
      bool status{false};
      if (this->serial_buffer_.size() >= len) {
        uint8_t *position = data;
        for (int i=0; i<len; i++) {
          this->read_byte(position);
          position++;
        }
      }
      return status;
    };

    bool NanoviewEmulator::read_byte(uint8_t *data) {
      bool status{false};
      if (this->serial_buffer_.size() > 0) {
        *data = this->serial_buffer_.front();
        ESP_LOGV(TAG, "read_byte:: Read value %d\n", *data);
        this->serial_buffer_.pop_front();
        status = true;
      }
      return status;
    };

    // Read a single byte, but return it as an int
    int NanoviewEmulator::read() {
      uint8_t data;
      if (!this->read_byte(&data))
        return -1;
      return data;
    }
  } // namespace nanoview_monitor
} // namespace esphome
