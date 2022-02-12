#pragma once

#include <list>

#include "esphome/core/helpers.h"
#include "nanoview_types.h"
#include "esphome/core/log.h"

#ifdef USE_ESP_IDF
#include <esp_heap_caps.h>
#include <esp_system.h>
#endif

#ifdef USE_ARDUINO
#include <Esp.h>
#endif

#define CONF_NANOVIEW_PART_PACKET_MAX 15
#define CONF_NANOVIEW_EMULATOR_BUFFER_SIZE (10 + CONF_NANOVIEW_PART_PACKET_MAX + (sizeof(NanoViewFullMessage) * 3))

namespace esphome {
  namespace nanoview_monitor {

    void logHeapSpace();

    void generateNanoviewFullMessage(std::list<uint8_t> *data, bool withFirmwareVersion);

    int generatePartPacket(std::list<uint8_t> *data);

    void logBuffer(std::list<uint8_t> *buffer);
    std::string format_hex_pretty(std::list<uint8_t> data);
    std::string format_hex_pretty(uint8_t* data, size_t length);
  } // namespace nanoview_monitor
} // namespace esphome