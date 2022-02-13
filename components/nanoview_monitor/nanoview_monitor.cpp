#include "esphome/core/log.h"
#include "nanoview_monitor.h"
#include "esphome.h"


#ifdef USE_LOGGER
 #include "esphome/components/logger/logger.h"
#endif

namespace esphome {
  namespace nanoview_monitor {

    static const char *TAG = "NanoviewMonitor";

    void NanoviewMonitor::setup() {
      ESP_LOGI(TAG, "Entering Nanoview Monitor Setup");
      this->messageType = NANOVIEW_UNSET;
      pinMode(D1, OUTPUT);
      if (this->emulate_uart_) this->nanoview_emulator_->setup();
      if (this->emulate_uart_) {
        ESP_LOGI(TAG, "##################### WARNING #######################");
        ESP_LOGI(TAG, "# Running in UART emulation mode. In this mode, the #");
        ESP_LOGI(TAG, "# OE pin of the TSX0108E will remain low. This mode #");
        ESP_LOGI(TAG, "# is intended for testing when no nanoview is       #");
        ESP_LOGI(TAG, "# available. In this mode, the UART is simulated.   #");
        ESP_LOGI(TAG, "# Fake test data is generated to simulate the UART  #");
        ESP_LOGI(TAG, "# and the hardware UART is available for logging.   #");
        ESP_LOGI(TAG, "# Note that the output connected to the OE pin of   #");
        ESP_LOGI(TAG, "# the TSX0108E will be set LOW.                     #");
        ESP_LOGI(TAG, "##################### WARNING #######################");
        digitalWrite(D1, LOW); // Connected to EO of TSX0108E

        
      }
      else {
        ESP_LOGI(TAG, "Enabling signal to serial port via EO of TSX0108E");
        digitalWrite(D1, HIGH); // Connected to EO of TSX0108E
      }
    }

    void NanoviewMonitor::loop() {
      if (this->emulate_uart_) this->nanoview_emulator_->loop();
      int availableBytes = 0;

      availableBytes = this->available_();
      ReadBufferState messageStatus = BUFFER_EMPTY;
      //delay(300);
      //availableBytes = 0;
      if (availableBytes > 0) {
        switch (this->messageType) {
          case NANOVIEW_LIVE_POWER: {
            std::size_t required_size = sizeof(nanoViewMessage.live_power);
            ESP_LOGD(TAG, "NANOVIEW_LIVE_POWER looking for %d bytes", required_size);
            messageStatus = this->readPacket(&nanoViewMessage.live_power, required_size, true);
            if (BUFFER_VALID == messageStatus) {
              ESP_LOGD(TAG, "Recieved NANOVIEW_LIVE_POWER Packet");
              nanoViewMessage.livePowerValid = true;
              this->messageType = NANOVIEW_UNSET;
            }
          }
          break;
          case NANOVIEW_ACCUMULATED_ENERGY: {
            std::size_t required_size = sizeof(nanoViewMessage.accumulated_energy);
            ESP_LOGD(TAG, "NANOVIEW_ACCUMULATED_ENERGY looking for %d bytes", required_size);
            messageStatus = this->readPacket(&nanoViewMessage.accumulated_energy, required_size, true);
            if (BUFFER_VALID == messageStatus) {
              ESP_LOGD(TAG, "Recieved NANOVIEW_ACCUMULATED_ENERGY Packet");
              nanoViewMessage.AccumulatedEnergyValid = true;
              this->messageType = NANOVIEW_UNSET;
            }
          }
          break;
          case NANOVIEW_FIRMWARE_VERSION: {
            std::size_t required_size = sizeof(nanoViewMessage.firmware_version);
            ESP_LOGD(TAG, "NANOVIEW_FIRMWARE_VERSION looking for %d bytes", required_size);
            messageStatus = this->readPacket(&nanoViewMessage.firmware_version, required_size, true);
            if (BUFFER_VALID == messageStatus) {
              ESP_LOGD(TAG, "Recieved NANOVIEW_FIRMWARE_VERSION Packet");
              this->messageType = NANOVIEW_UNSET;
              nanoViewMessage.firmwareValid = true;
            }
          }
          break;
          case NANOVIEW_MESSAGE_START:
            ESP_LOGD(TAG, "Processing NANOVIEW_MESSAGE_START");
          default:
            ESP_LOGD(TAG, "Processing default");
            this->messageType = this->readMessageStart();
          break;
        } // switch 
        if (BUFFER_CORRUPT == messageStatus) {
          ESP_LOGD(TAG, "Clearing Corrupt Buffer");
          this->messageType = NANOVIEW_UNSET;
        }
          
        if (nanoViewMessage.AccumulatedEnergyValid && nanoViewMessage.livePowerValid) {
          /*
            update_interval from nanoview.yaml.
            Will only send update on update_interval successful reading.
            The nanoview hardware sends a full set of readings once each second.
            This is used to set how many seconds pass between readings.
            Note the data will still be read to remove it from the seril buffers, 
            but will then be overwritten if not sent to the home assistant.
          */
          ESP_LOGD(TAG, "updateCount %d, updateInterval %d", this->updateCount, this->updateInterval);
          if (++(this->updateCount) >= this->updateInterval) {
            this->logMessageAsHex(&nanoViewMessage);
            // We have both important packets for this second, so lets publish them to MQTT
            publishData(&nanoViewMessage); // Initial attempt.  Only oublished the value, not the configuration
            this->updateCount = 0;
            logHeapSpace();
          }
          // Reset ready for next reading.
          nanoViewMessage.livePowerValid = false;
          nanoViewMessage.AccumulatedEnergyValid = false;
          nanoViewMessage.firmwareValid = false;
          //ESP_LOGI(TAG, "NanoviewMonitor::loop %d bytes available", availableBytes);
        }
      }
    }

    void NanoviewMonitor::set_voltage_unit(std::string unit) {
      this->voltage_->set_unit_of_measurement(unit);
    }

    /*
      This code reads from the UART at startup to find the first packet start value
      The same code is used to read the type from each data packet. Ths will cause the
      code to resync with the 0xAA start byte if we get a corrupt packet.
    */
    NanoViewMessageType NanoviewMonitor::readMessageStart() {
      uint8_t currentValue = 0;
      int count = 0;
      NanoViewMessageType result = NANOVIEW_UNSET;
      while (currentValue != NANOVIEW_MESSAGE_START && count < 5) { // '\xAA'
        if (this->available_() > 0) {
          currentValue = (NanoViewMessageType) this->read_();
        }
        count++;
      }

      if (currentValue == NANOVIEW_MESSAGE_START) {
        result = (NanoViewMessageType) this->read_();
      }
      
      return result;
    }

    ReadBufferState NanoviewMonitor::readPacket(void *dataBuffer, int length, bool verify) {
      uint8_t crc1 = 0;
      uint8_t crc2 = 0;
      ReadBufferState result = BUFFER_EMPTY;
      uint8_t *localBuffer = (uint8_t *)dataBuffer;
      int availableBytes = this->available_();
      ESP_LOGD(TAG, "NanoviewMonitor::readPacket Looking For %d Bytes", length);
      if (availableBytes >= length) {
        if (this->read_array_(localBuffer, length)) {
          result = BUFFER_VALID;
          ESP_LOGD(TAG, "NanoviewMonitor::readPacket read %d bytes, %d remaining", length, this->available_());
          if (verify) {
            /*
              Checksum Calculation pseudo code

              crc1 = 0
              crc2 = 0
              for byte in data[]:
                crc1 = crc1 ADD byte
                crc2 = crc2 XOR crc1
            */
            for (int i = 0; i<(length - 2); i++) {
              crc1 += localBuffer[i];
              crc2 = crc2 ^ crc1;
            }
            if (crc1 != localBuffer[length - 2]) {
              ESP_LOGE(TAG, "CRC1 expected %x but got %x\n", crc1, localBuffer[length - 2]);
              result = BUFFER_CORRUPT;
            }
            if (crc2 != localBuffer[length - 1]) {
              ESP_LOGE(TAG, "CRC2 expected %x but got %x\n", crc2, localBuffer[length - 1]);
              result = BUFFER_CORRUPT;
            }
          }
        }
      }
      return result;
    }

    void NanoviewMonitor::logMessageAsHex(NanoViewFullMessage *energyDetails) {
      ESP_LOGI(TAG, "Hex Logging");
      char printBuffer[100];
      if (energyDetails->firmwareValid) {
        ESP_LOGI(TAG, "Firware Version 0x%02X, crc1 0x%02X crc2 0x%02X\n", 
                  energyDetails->firmware_version.version,
                  energyDetails->firmware_version.crc1,
                  energyDetails->firmware_version.crc2);
      }
      if (energyDetails->livePowerValid) {
        ESP_LOGI(TAG, " 0x%04X\n", energyDetails->live_power.voltage);
        ESP_LOGI(TAG, "Live Power 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X, 0x%04X\n", 
          energyDetails->live_power.power[0], energyDetails->live_power.power[1], energyDetails->live_power.power[2],
          energyDetails->live_power.power[3], energyDetails->live_power.power[4], energyDetails->live_power.power[5],
          energyDetails->live_power.power[6], energyDetails->live_power.power[7], energyDetails->live_power.power[8],
          energyDetails->live_power.power[9], energyDetails->live_power.power[10], energyDetails->live_power.power[11],
          energyDetails->live_power.power[12], energyDetails->live_power.power[13], energyDetails->live_power.power[14],
          energyDetails->live_power.power[15]);
          ESP_LOGI(TAG, "CRC For Live Power 0x%02X, 0x%02X\n", energyDetails->live_power.crc1, energyDetails->live_power.crc2);
      }
      if (energyDetails->AccumulatedEnergyValid) {
          ESP_LOGI(TAG, "Accumulated Energy 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X\n", 
          energyDetails->accumulated_energy.power[0], energyDetails ->accumulated_energy.power[1], energyDetails->accumulated_energy.power[2],
          energyDetails->accumulated_energy.power[3], energyDetails->accumulated_energy.power[4], energyDetails->accumulated_energy.power[5],
          energyDetails->accumulated_energy.power[6], energyDetails->accumulated_energy.power[7], energyDetails->accumulated_energy.power[8],
          energyDetails->accumulated_energy.power[9], energyDetails->accumulated_energy.power[10], energyDetails->accumulated_energy.power[11],
          energyDetails->accumulated_energy.power[12], energyDetails->accumulated_energy.power[13], energyDetails->accumulated_energy.power[14],
          energyDetails->accumulated_energy.power[15]);
          ESP_LOGI(TAG, "CRC For Accumulated Energy 0x%02X, 0x%02X\n", energyDetails->accumulated_energy.crc1, energyDetails->accumulated_energy.crc2);
      }
    }

    bool NanoviewMonitor::publishData(NanoViewFullMessage *nanoViewMessage) {
      ESP_LOGI(TAG, "Publishing Data");
      bool result = true;
      if (this->voltage_ != nullptr) {
        this->voltage_->publish_state(nanoViewMessage->live_power.voltage);
        this->voltage_value_ = nanoViewMessage->live_power.voltage;
      }
      if (this->slots != nullptr) {
        for (int i=0; i<this->usedSlots_; i++) { // Only publish used slots
          this->slots[i].set_live_power(nanoViewMessage->live_power.power[i]);
          this->slots[i].set_accumulated_energy(nanoViewMessage->accumulated_energy.power[i]);
        }
      }
      return result;
    }

    NanoviewMonitor::NanoviewMonitor() : uart::UARTDevice() {

    }

    void NanoviewMonitor::dump_config() {
      ESP_LOGCONFIG(TAG, "Nanoview Sensor:");
      ESP_LOGCONFIG(TAG, "  Update Interval: %d", this->get_update_interval());

      ESP_LOGCONFIG(TAG, "Voltage Configuration");
      ESP_LOGCONFIG(TAG, "  Voltage Name: %s", this->voltage_->get_name().c_str());
      ESP_LOGCONFIG(TAG, "  Voltage Unit: %s", this->voltage_->get_unit_of_measurement().c_str());

      ESP_LOGCONFIG(TAG, "Slot Configuration");
      /*for (int i=0; i<NANOVIEW_CHANNEL_COUNT; i++) {
        ESP_LOGCONFIG(TAG, "    Slot Number: %d", i+1);
        ESP_LOGCONFIG(TAG, "    Slot Name: %s", this->power[i].get_name().c_str());
      }*/
    }

    void NanoviewMonitor::set_name(std::string name) { this->name_ = name; };
    void NanoviewMonitor::set_used_slots(int used_slots) { this->usedSlots_ = used_slots; };
    void NanoviewMonitor::set_update_interval(int period) {
      this->updateInterval = period;
      if (this->emulate_uart_) this->nanoview_emulator_->set_update_interval(period);
    };

    const std::string &NanoviewMonitor::get_name() { return this->name_; };
    int NanoviewMonitor::get_used_slots() { return this->usedSlots_; };
    int NanoviewMonitor::get_update_interval() { return this->updateInterval; };

    bool NanoviewMonitor::get_force_update() const { return force_update_; }
    /// Set force update mode.
    void NanoviewMonitor::set_force_update(bool force_update) { force_update_ = force_update; }

    void NanoviewMonitor::set_emulate_uart() {
      this->emulate_uart_ = true;
      if (this->emulate_uart_) this->nanoview_emulator_ = new NanoviewEmulator();
    }

    bool NanoviewMonitor::emulate_uart() {
      return this->emulate_uart_;
    }

    int NanoviewMonitor::available_() {
      int availableBytes = 0;
      if (this->emulate_uart_)
        availableBytes = this->nanoview_emulator_->available();
      else
        availableBytes = this->available();
      if (this->previousBytes != availableBytes)
        ESP_LOGD(TAG, "NanoviewMonitor::available_ %d bytes available", availableBytes);
      
      return availableBytes;
    };

    bool NanoviewMonitor::read_array_(uint8_t *data, size_t len) {
      if (this->emulate_uart_)
        return this->nanoview_emulator_->read_array(data, len);
      return this->read_array(data, len);
    };

    // TODO Ideally should use bool read_byte(uint8_t *data) instead of read
    int NanoviewMonitor::read_() {
      if (this->emulate_uart_)
        return this->nanoview_emulator_->read();
      return this->read();
    };
  } //namespace nanoview_monitor
} //namespace esphome