#include <stdio.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

#include "nanoview_util.h"

static const char *TAG = "nanoview_util";


namespace esphome {
  namespace nanoview_monitor {

    void logHeapSpace() {
      uint32_t free_heap;
#ifdef USE_ARDUINO
      free_heap = ESP.getFreeHeap();
#elif defined(USE_ESP_IDF)
      free_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
#endif
      ESP_LOGI(TAG, "Free Heap Size: %u bytes", free_heap);
    }


    CRCValue CalculateCRC(uint8_t *buffer, int length) {
      CRCValue crc;
      // Use length -2 as all packets end with a two byte CRC.
      // This allows us to pass in just the size of the message.
      for (int i = 0; i<(length - 2); i++) {
        crc.crc1 += buffer[i];
        crc.crc2 = crc.crc2 ^ crc.crc1;
      }
      return crc;
    }

    void updateCrc(uint8_t *crc1, uint8_t *crc2, const uint8_t *data) {
      *crc1 += *data;
      *crc2 = *crc2 ^ *crc1;
    }

    void updateCrc(uint8_t *crc1, uint8_t *crc2, const uint16_t *data) {
      uint8_t *update_byte = (uint8_t *)data;
      updateCrc(crc1, crc2, update_byte);
      update_byte++;
      updateCrc(crc1, crc2, update_byte);
    }

    void updateCrc(uint8_t *crc1, uint8_t *crc2, const uint32_t *data) {
      uint8_t *update_byte = (uint8_t *)data;
      updateCrc(crc1, crc2, update_byte);
      update_byte++;
      updateCrc(crc1, crc2, update_byte);
      update_byte++;
      updateCrc(crc1, crc2, update_byte);
      update_byte++;
      updateCrc(crc1, crc2, update_byte);
    }

    void generateNanoviewTestData(std::list<uint8_t> *data, NanoViewMessageType type) {
      uint8_t crc1 = 0;
      uint8_t crc2 = 0;
      int length = 0;
      data->push_back('\xAA');
      switch(type) {
          case NANOVIEW_FIRMWARE_VERSION:
            length = sizeof(NanoViewFirmwareVersion);
            data->push_back('\x12');
          break;
          case NANOVIEW_LIVE_POWER:
            length = sizeof(NanoViewLivePower);
            data->push_back('\x10');
          break;
          case NANOVIEW_ACCUMULATED_ENERGY:
            length = sizeof(NanoViewAccumulatedEnergy);
            data->push_back('\x11');
          break;
          default:
            ESP_LOGI(TAG, "generateNanoviewTestData:: Invalid message type %d", type);
      }
      // Subtract 2 from length to account for CRC values.
      for (int i=0; i<length-2; i++) {
        uint8_t random_value;
        random_bytes(&random_value, 1);
        data->push_back(random_value);
        updateCrc(&crc1, &crc2, &random_value);
      }
      data->push_back(crc1);
      data->push_back(crc2);
    }

    void generateNanoviewFullMessage(std::list<uint8_t> *data, bool withFirmwareVersion) {
      ESP_LOGI(TAG, "Max Buffer Size is %d", CONF_NANOVIEW_EMULATOR_BUFFER_SIZE);
      ESP_LOGI(TAG, "Nanoview Full Message Size %d", sizeof(NanoViewFullMessage));
      if (data->size() + sizeof(NanoViewFullMessage) <= CONF_NANOVIEW_EMULATOR_BUFFER_SIZE) {
        if (withFirmwareVersion) generateNanoviewTestData(data, NanoViewMessageType::NANOVIEW_FIRMWARE_VERSION);
        generateNanoviewTestData(data, NanoViewMessageType::NANOVIEW_LIVE_POWER);
        generateNanoviewTestData(data, NanoViewMessageType::NANOVIEW_ACCUMULATED_ENERGY);
      }
    }

    /*
      At first read from the UART, you may get a partial packet due to timing. This generates ata to simulate that.
    */
    int generatePartPacket(std::list<uint8_t> *data) {
      /* generate random number between 0 and 15: */
      int length = rand() % CONF_NANOVIEW_PART_PACKET_MAX;
      ESP_LOGI(TAG, "generatePartPacket:: generating %d bytes\n", length);
      for (int i=0; i<length; i++) {
        uint8_t random_value;
        random_bytes(&random_value, 1);
        /*if (randomvalue == '\xAA') i--;
        else*/
        data->push_back(random_value);
      }

      return length;
    }

    void logPartPacket(void *localBuffer, int length) {
      std::string hexData = format_hex_pretty((uint8_t *)localBuffer, length + 2);

      ESP_LOGI(TAG, "Buffer Part Packet Length %d, Data %s\n", length, hexData.c_str());
    };

    static char hex_chars[] = "0123456789ABCDEF"; 
    std::string format_hex_pretty(uint8_t* data, size_t length) {
      if (length == 0)
    	  return "";
      std::string hexString;
      for (size_t i = 0; i < length; i++)
      {
        std::string hexValue{"0x"};
        hexValue.push_back(hex_chars[((data[i] & 0xF0) >> 4)]);
        hexValue.push_back(hex_chars[((data[i] & 0x0F) >> 0)]);
        if (hexString.size() > 0) hexString.append(", ");
        hexString.append(hexValue);
      }
      if (length > 4)
      	hexString.append(" (" + to_string(length) + ")");
      return hexString;
    }

    std::string format_hex_pretty(std::list<uint8_t> *data) {
      if (data->size() == 0)
    	  return "";
      std::string hexString;
      for (const uint8_t value : *data) {
        std::string hexValue{"0x"};
        hexValue.push_back(hex_chars[((value & 0xF0) >> 4)]);
        hexValue.push_back(hex_chars[((value & 0x0F) >> 0)]);
        if (hexString.size() > 0) hexString.append(", ");
        hexString.append(hexValue);
      }
      if (data->size() > 4)
      	hexString.append(" (" + to_string(data->size()) + ")");
      return hexString;
    }

    void logBuffer(std::list<uint8_t> *buffer) {
      std::string hexData = format_hex_pretty(buffer);

      ESP_LOGI(TAG, "Data Length %d, Data %s\n", buffer->size(), hexData.c_str());
    }
    
  } // namespace nanoview_monitor
} // namespace esphome