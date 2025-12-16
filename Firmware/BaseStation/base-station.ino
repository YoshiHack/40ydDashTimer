/*
  40-Yard Dash Timer — Base Station (ESP32)
  - Receives START / FINISH events
  - Computes elapsed sprint time
  - SIM_MODE allows testing via Serial input (no hardware required)
  License: GPL-3.0
*/

#include <WiFi.h>
#include <esp_now.h>

#define SIM_MODE 1  // 1 = Serial simulation, 0 = real ESP-NOW packets

// ===================== PACKET STRUCT =====================
#pragma pack(push, 1)
struct TimerPacket {
  uint8_t gate_id;      // 0=start, 1=finish
  uint8_t event_type;   // 1=trigger
  uint32_t seq;
  uint32_t uptime_ms;
};
#pragma pack(pop)

// Gate IDs
static const uint8_t GATE_START  = 0;
static const uint8_t GATE_FINISH = 1;

// ===================== STATE =====================
enum RunState {
  IDLE,
  RUNNING
};

static RunState state = IDLE;
static uint32_t startTimeMs = 0;
static uint32_t finishTimeMs = 0;

// ===================== HELPERS =====================
void handleTrigger(uint8_t gateId) {
  uint32_t now = millis();

  if (gateId == GATE_START) {
    if (state == IDLE) {
      startTimeMs = now;
      state = RUNNING;
      Serial.println("[BASE] START detected");
    } else {
      Serial.println("[BASE] WARNING: START received while already running");
    }
  }

  if (gateId == GATE_FINISH) {
    if (state == RUNNING) {
      finishTimeMs = now;
      uint32_t elapsed = finishTimeMs - startTimeMs;
      state = IDLE;

      Serial.println("[BASE] FINISH detected");
      Serial.print("[BASE] 40-yard time: ");
      Serial.print(elapsed / 1000.0, 3);
      Serial.println(" seconds");
      Serial.println("--------------------------------");
    } else {
      Serial.println("[BASE] WARNING: FINISH received before START");
    }
  }
}

// ===================== ESP-NOW CALLBACK =====================
#if !SIM_MODE
void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (len != sizeof(TimerPacket)) return;

  TimerPacket pkt;
  memcpy(&pkt, data, sizeof(pkt));

  handleTrigger(pkt.gate_id);
}
#endif

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(200);

#if SIM_MODE
  Serial.println();
  Serial.println("=== Base Station (SIM MODE) ===");
  Serial.println("Type:");
  Serial.println("  s  -> simulate START");
  Serial.println("  f  -> simulate FINISH");
  Serial.println("--------------------------------");
#else
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init FAILED");
    while (true) {
      delay(100);
    }
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println();
  Serial.println("=== Base Station Ready ===");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
#endif
}

// ===================== LOOP =====================
void loop() {
#if SIM_MODE
  if (Serial.available()) {
    char c = (char)Serial.read();

    if (c == 's' || c == 'S') {
      Serial.println("SIM INPUT: START");
      handleTrigger(GATE_START);
    }

    if (c == 'f' || c == 'F') {
      Serial.println("SIM INPUT: FINISH");
      handleTrigger(GATE_FINISH);
    }
  }
  delay(5);
#else
  delay(10);
#endif
}
