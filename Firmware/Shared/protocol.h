#pragma once
#include <stdint.h>

static const uint8_t GATE_START  = 0;
static const uint8_t GATE_FINISH = 1;

static const uint8_t EVENT_TRIGGER = 1;
// reserved for later:
// static const uint8_t EVENT_ARM = 2;
// static const uint8_t EVENT_GO  = 3;

#pragma pack(push, 1)
struct TimerPacket {
  uint8_t gate_id;
  uint8_t event_type;
  uint32_t seq;
  uint32_t uptime_ms;
};
#pragma pack(pop)