#pragma once


namespace esphome {
  namespace nanoview_monitor {
    static const std::string CONST_NANOVIEW_LIVE_POWER_UNIT {"W"};
    static const std::string CONST_NANOVIEW_ACCUMULATED_ENERGY_UNIT {"KwH"};
    static const std::string CONST_NANOVIEW_VOLTAGE_UNIT {"V"};

    static const std::string CONST_NANOVIEW_POWER {"power"};
    static const std::string CONST_NANOVIEW_ENERGY {"energy"};
    static const std::string CONST_NANOVIEW_VOLTAGE {"voltage"};


    #define NANOVIEW_CHANNEL_COUNT    16


    enum NanoViewMessageType : uint8_t {
        NANOVIEW_LIVE_POWER             = 0x10,
        NANOVIEW_ACCUMULATED_ENERGY     = 0x11,
        NANOVIEW_FIRMWARE_VERSION       = 0x12,
        NANOVIEW_MESSAGE_START          = 0xAA,
        NANOVIEW_UNSET                  = 0x00
    };

    enum ReadBufferState : uint8_t {
      BUFFER_EMPTY        = 0x1,
      BUFFER_VALID        = 0x2,
      BUFFER_CORRUPT      = 0x3
    };

    struct __attribute__((__packed__)) CRCValue {
      uint8_t crc1 = 0;
      uint8_t crc2 = 0;
    };

    /*
      Example Firmware Version

      0xAA, 0x125, 0x55
    */
    struct __attribute__((__packed__)) NanoViewFirmwareVersion {
      uint8_t version;
      uint8_t crc1;
      uint8_t crc2;
    };

    /*
      Example Live Power Packet

      0xAA, 0x10, 0xE6, 0x0, 0xE0, 0x2E, 0x64, 0x0, 0xC8, 0x0, 0x2C, 0x1, 0x90, 0x1, 0xF4, 0x1, 0x58, 0x2, 0xBC, \
      0x2, 0x20, 0x3, 0x84, 0x3, 0xE8, 0x3, 0x4C, 0x4, 0xB0, 0x4, 0x14, 0x5, 0x78, 0x5, 0xDC, 0x5, 0xFB, 0xD5

      - Packet ID:        : 0x10
      - Voltage           : 230V
      - Channel 1 (total) : 12000W
      - Channels 2 - 16   : 100W - 1500W (incrementing by 100W for each channel)
      - CRC1, CR2         : 0xFB, 0xD5
    */
    struct __attribute__((__packed__)) NanoViewLivePower {
      uint16_t voltage;
      uint16_t power[NANOVIEW_CHANNEL_COUNT];
      uint8_t crc1;
      uint8_t crc2;
    };

    /*
      Example Accumulated Energy Packet

      0xAA, 0x11, 0xC0, 0xD4, 0x1, 0x0,0xE8, 0x3, 0x0, 0x0,0xD0, 0x7, 0x0, 0x0,0xB8, 0xB, 0x0, 0x0,0xA0, 0xF, 0x0, 0x0, \
      0x88, 0x13, 0x0, 0x0,0x70, 0x17, 0x0, 0x0,0x58, 0x1B, 0x0, 0x0,0x40, 0x1F, 0x0, 0x0,0x28, 0x23, 0x0, 0x0, \
      0x10, 0x27, 0x0, 0x0,0xF8, 0x2A, 0x0, 0x0,0xE0, 0x2E, 0x0, 0x0,0xC8, 0x32, 0x0, 0x0,0xB0, 0x36, 0x0, 0x0, \
      0x98, 0x3A, 0x0, 0x0, 0x21, 0x80

      - Packet ID:        : 0x11
      - Channel 1 (total) : 120kWh
      - Channels 2 - 16   : 1kWh - 15kWh (incrementing by 1kWh for each channel)
      - CRC1, CR2         : 0x21, 0x80
    */
    struct __attribute__((__packed__)) NanoViewAccumulatedEnergy {
      uint32_t power[NANOVIEW_CHANNEL_COUNT];
      uint8_t crc1;
      uint8_t crc2;
    };

    // Note the firmware version is sent randomly, so will often be empty.
    // Seems to get sent several times after a power outage.
    struct NanoViewFullMessage {
      NanoViewFirmwareVersion firmware_version;
      NanoViewLivePower live_power;
      NanoViewAccumulatedEnergy accumulated_energy;

      /*
        Putting the valid flags here allows us to pack the data structure so we can memcpy into it.
      */
      bool firmwareValid = false;
      bool livePowerValid = false;
      bool AccumulatedEnergyValid = false;
    };
  } // namespace nanoview_monitor
} // namespace esphome