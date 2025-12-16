#pragma once
#include <stdint.h>

enum GateId : uint8_t { GATE_START = 0, GATE_FINISH = 1 };
enum EventType : uint8_t { EVENT_TRIGGER = 1 };

#pragma pack(push, 1)
struct TimerPacket {
  uint8_t gate_id;      // start/finish
  uint8_t event_type;   // trigger
  uint32_t seq;         // packet sequence
  uint32_t uptime_ms;   // millis() on sender
};
#pragma pack(pop)
