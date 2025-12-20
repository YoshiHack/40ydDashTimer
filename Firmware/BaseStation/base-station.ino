/*
  40-Yard Dash Timer — Base Station (ESP32)
  - Receives START / FINISH events
  - Reaction mode: READY -> SET -> random delay -> GO (arms timer)
  - Detects false starts (START before GO)
  - SIM_MODE allows testing via Serial input (no hardware required)
  License: GPL-3.0
*/

#include <WiFi.h>
#include <esp_now.h>
#include "../shared/protocol.h"


#define SIM_MODE 1  // 1 = Serial simulation, 0 = real ESP-NOW packets

enum RunState { IDLE, ARMED_WAIT_GO, RUNNING };
static RunState state = IDLE;

static uint32_t startTimeMs = 0;
static uint32_t finishTimeMs = 0;

// CSV logging
static void printCSVHeader() {
  Serial.println("CSV_HEADER,count,elapsed_ms,elapsed_s,start_ms,finish_ms,mode");
}

static bool logPackets = true;   // toggle verbose packet logging

// Reaction mode timing
static bool reactionEnabled = true;
static bool goAnnounced = false;
static uint32_t goAtMs = 0;           // when GO happens
static uint32_t reactionStartMs = 0;  // when reaction sequence started

// ===================== CSV LOGGING =====================
static uint32_t runCount = 0;          // increments each completed run
static bool lastRunWasReaction = false;

// Random wait range (ms) between SET and GO
static const uint32_t GO_DELAY_MIN_MS = 1000;
static const uint32_t GO_DELAY_MAX_MS = 3000;

static void printHelp() {
  Serial.println("=== Base Station (SIM MODE) ===");
  Serial.println("Keys:");
  Serial.println("  r  -> start reaction sequence (READY/SET/.../GO)");
  Serial.println("  s  -> simulate START trigger");
  Serial.println("  f  -> simulate FINISH trigger");
  Serial.println("  x  -> reset to IDLE");
  Serial.println("  l  -> toggle packet logging");
  Serial.println("  p  -> print CSV header");
  Serial.println("--------------------------------");
}

static void resetToIdle(const char* reason = nullptr) {
  state = IDLE;
  goAnnounced = false;
  goAtMs = 0;
  reactionStartMs = 0;
  if (reason) {
    Serial.print("[BASE] RESET: ");
    Serial.println(reason);
  }
}

static void startReactionSequence() {
  resetToIdle();
  reactionStartMs = millis();
  state = ARMED_WAIT_GO;

  // Seed RNG (good enough for this use)
  randomSeed((uint32_t)esp_random());

  uint32_t delayMs = random(GO_DELAY_MIN_MS, GO_DELAY_MAX_MS + 1);
  goAtMs = reactionStartMs + delayMs;

  Serial.println("[BASE] READY");
  delay(350);
  Serial.println("[BASE] SET");
  // GO will be printed later when millis() >= goAtMs
}

static void printCSVHeader() {
  Serial.println("CSV_HEADER,count,elapsed_ms,elapsed_s,start_ms,finish_ms,mode");
}

static void printRunCSV(uint32_t elapsedMs) {
  runCount++;

  Serial.print("CSV,");
  Serial.print(runCount); Serial.print(",");
  Serial.print(elapsedMs); Serial.print(",");
  Serial.print(elapsedMs / 1000.0, 3); Serial.print(",");
  Serial.print(startTimeMs); Serial.print(",");
  Serial.print(finishTimeMs); Serial.print(",");
  Serial.print(lastRunWasReaction ? "reaction" : "traditional");
  Serial.println();
}

}

static void logPacket(const TimerPacket& pkt, const char* src) {
  if (!logPackets) return;

  Serial.print("LOG,src,"); Serial.print(src);
  Serial.print(",gate_id,"); Serial.print(pkt.gate_id);
  Serial.print(",event_type,"); Serial.print(pkt.event_type);
  Serial.print(",seq,"); Serial.print(pkt.seq);
  Serial.print(",uptime_ms,"); Serial.print(pkt.uptime_ms);
  Serial.print(",now_ms,"); Serial.println(millis());
}

// Called when a trigger arrives (either SIM or real ESP-NOW)
static void handleTrigger(uint8_t gateId) {
  uint32_t now = millis();

  if (gateId == GATE_START) {
    // If reaction mode is enabled, require GO first
    if (reactionEnabled) {
      if (state == ARMED_WAIT_GO && !goAnnounced) {
        Serial.println("[BASE] FALSE START (START before GO)");
        resetToIdle("false start");
        Serial.println("[BASE] FALSE START (START before GO)"); 
        resetToIdle("false start");
        return;
      }
    }
      if (state == IDLE) {
        Serial.println("[BASE] WARNING: START received while IDLE (no reaction armed).");
        // Allow it to start anyway (traditional mode)
        startTimeMs = now;
        state = RUNNING;
        Serial.println("[BASE] START detected (traditional)");
        return;
      }
    }

    if (state == ARMED_WAIT_GO && goAnnounced) {
      startTimeMs = now;
      state = RUNNING;
      Serial.println("[BASE] START detected");
      return;
    }

    if (state == RUNNING) {
      Serial.println("[BASE] WARNING: START received while already running");
      return;
    }

    // Fallback: start run if not in reaction flow
    if (state == IDLE) {
      startTimeMs = now;
      state = RUNNING;
      Serial.println("[BASE] START detected");
    }
  }

  if (gateId == GATE_FINISH) {
    if (state == RUNNING) {
      finishTimeMs = now;
      uint32_t elapsed = finishTimeMs - startTimeMs;
      lastRunWasReaction = reactionEnabled;  // log what mode we were using


      Serial.println("[BASE] FINISH detected");
      Serial.print("[BASE] 40-yard time: ");
      Serial.print(elapsed / 1000.0, 3);
      Serial.println(" seconds");
      Serial.println("--------------------------------");
      printRunCSV(elapsed);
    } else {
      Serial.println("[BASE] WARNING: FINISH received before START");
    }
  }
}

#if !SIM_MODE
void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  #if !SIM_MODE
void onReceive(const uint8_t *mac, const uint8_t *data, int len) {
  if (len != (int)sizeof(TimerPacket)) return;

  TimerPacket pkt;
  memcpy(&pkt, data, sizeof(pkt));

  // Accept only triggers + known gates
  if (pkt.event_type != EVENT_TRIGGER) return;
  if (pkt.gate_id != GATE_START && pkt.gate_id != GATE_FINISH) return;

  handleTrigger(pkt, "espnow");
}
#endif


void setup() {
  Serial.begin(115200);
  delay(200);

#if SIM_MODE
  Serial.println();
  printHelp();
  printCSVHeader();
#else
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init FAILED");
    while (true) delay(100);
  }

  esp_now_register_recv_cb(onReceive);

  Serial.println();
  Serial.println("=== Base Station Ready ===");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
#endif
}

void loop() {
  uint32_t now = millis();

  // Reaction flow: announce GO at the right moment
  if (reactionEnabled && state == ARMED_WAIT_GO && !goAnnounced && goAtMs != 0 && now >= goAtMs) {
    goAnnounced = true;
    Serial.println("[BASE] GO!");
  }

#if SIM_MODE
  if (Serial.available()) {
    char c = (char)Serial.read();

    if (c == 'r' || c == 'R') startReactionSequence();
    if (c == 's' || c == 'S') {
  Serial.println("SIM INPUT: START");
  TimerPacket pkt{};
  pkt.gate_id = GATE_START;
  pkt.event_type = EVENT_TRIGGER;
  pkt.seq = 1;
  pkt.uptime_ms = millis();
  handleTrigger(pkt, "sim");
}

if (c == 'f' || c == 'F') {
  Serial.println("SIM INPUT: FINISH");
  TimerPacket pkt{};
  pkt.gate_id = GATE_FINISH;
  pkt.event_type = EVENT_TRIGGER;
  pkt.seq = 1;
  pkt.uptime_ms = millis();
  handleTrigger(pkt, "sim");
}

    if (c == 'x' || c == 'X') resetToIdle("manual reset");
    if (c == 'h' || c == 'H') printHelp();
    if (c == 'l' || c == 'L') {
  logPackets = !logPackets;
  Serial.print("[BASE] logPackets=");
  Serial.println(logPackets ? "ON" : "OFF");
}
  }
  if (c == 'p' || c == 'P') printCSVHeader();
  delay(5);
#else
  delay(10);
#endif
}
