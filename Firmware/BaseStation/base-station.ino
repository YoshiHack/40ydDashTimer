  #include <WiFi.h>
  #include <esp_now.h>
  #include "../shared/protocol.h"

#define SIM_MODE 1

enum RunState { IDLE, ARMED_WAIT_GO, RUNNING };
static RunState state = IDLE;

static uint32_t startTimeMs = 0;
static uint32_t finishTimeMs = 0;

static void printCSVHeader() {
Serial.println("CSV_HEADER,count,elapsed_ms,elapsed_s,mph,start_ms,finish_ms,mode");
}

static bool logPackets = true;

static const uint32_t RUN_TIMEOUT_MS = 15000;
  static const uint32_t ARM_TIMEOUT_MS = 10000;
static uint32_t stateEnteredMs = 0;

static bool reactionEnabled = true;
static bool goAnnounced = false;
static uint32_t goAtMs = 0;
static uint32_t reactionStartMs = 0;

static uint32_t runCount = 0;
static bool lastRunWasReaction = false;

static const uint32_t GO_DELAY_MIN_MS = 1000;
static const uint32_t GO_DELAY_MAX_MS = 3000;

static float calcMPH(uint32_t elapsedMs) {
float t = elapsedMs / 1000.0f;
if (t <= 0.0f) return 0.0f;
return 81.818f / t;
}

static const uint8_t HISTORY_N = 5;
  static uint32_t historyMs[HISTORY_N] = {0};
  static uint8_t historyCount = 0;
  static uint8_t historyIndex = 0;
static uint32_t bestMs = 0;

static void printHelp() {
Serial.println("=== Base Station (SIM MODE) ===");
Serial.println("Keys:");
Serial.println("  r  -> start reaction sequence (READY/SET/.../GO)");
Serial.println("  s  -> simulate START trigger");
Serial.println("  f  -> simulate FINISH trigger");
Serial.println("  data  -> reset to IDLE");
Serial.println("  l  -> toggle packet logging");
  Serial.println("  p  -> print CSV header");
  Serial.println("--------------------------------");
  }

  static void resetToIdle(const char* reason = nullptr) {
  state = IDLE;
  goAnnounced = false;
  stateEnteredMs = millis();
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

randomSeed((uint32_t)esp_random());

  uint32_t delayMs = random(GO_DELAY_MIN_MS, GO_DELAY_MAX_MS + 1);
  goAtMs = reactionStartMs + delayMs;

Serial.println("[BASE] READY");
  delay(350);
  Serial.println("[BASE] SET");

  }

static void printCSVHeader() {
  Serial.println("CSV_HEADER,count,elapsed_ms,elapsed_s,mph,start_ms,finish_ms,mode");
  }

  static void printRunCSV(uint32_t elapsedMs) {
runCount++;

float mph = calcMPH(elapsedMs);

Serial.print("CSV,");
Serial.print(runCount); Serial.print(",");
Serial.print(elapsedMs); Serial.print(",");
  Serial.print(elapsedMs / 1000.0, 3); Serial.print(",");
Serial.print(mph, 2); Serial.print(",");
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

  static void statsClear() {
  for (uint8_t index = 0; index < HISTORY_N; index++) historyMs[index] = 0;
  historyCount = 0;
  historyIndex = 0;
  bestMs = 0;
Serial.println("[BASE] Stats cleared.");
}

  static void statsAdd(uint32_t elapsedMs) {

  if (bestMs == 0 || elapsedMs < bestMs) bestMs = elapsedMs;

historyMs[historyIndex] = elapsedMs;
historyIndex = (historyIndex + 1) % HISTORY_N;
if (historyCount < HISTORY_N) historyCount++;
  }

static float statsAvgLastN() {
  if (historyCount == 0) return 0.0f;
  uint32_t combined = 0;
  for (uint8_t index = 0; index < historyCount; index++) combined += historyMs[index];
  return (float)combined / (float)historyCount;
}

static void statsPrint() {
  if (historyCount == 0) {
  Serial.println("[BASE] Stats: no runs yet.");
  return;
  }

float avgMs = statsAvgLastN();

  Serial.print("[BASE] Best: ");
    Serial.print(bestMs / 1000.0, 3);
    Serial.println(" s");

Serial.print("[BASE] Avg last ");
  Serial.print(historyCount);
Serial.print(": ");
  Serial.print(avgMs / 1000.0, 3);
  Serial.println(" s");
  }

  static void handleTrigger(uint8_t gateId) {
  uint32_t now = millis();

  if (state == ARMED_WAIT_GO && (now - stateEnteredMs) > ARM_TIMEOUT_MS) {
Serial.println("[BASE] TIMEOUT: armed too long without starting");
Serial.print("CSV,timeout,armed,at_ms,");
Serial.print(now);
Serial.println();
  resetToIdle("arm timeout");
}

  if (state == RUNNING && (now - stateEnteredMs) > RUN_TIMEOUT_MS) {
  Serial.println("[BASE] TIMEOUT: run took too long (missing FINISH?)");
  Serial.print("CSV,timeout,run,at_ms,");
  Serial.print(now);
  Serial.println();
resetToIdle("run timeout");
}
if (gateId == GATE_START) {

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
    lastRunWasReaction = reactionEnabled;

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
    void onReceive(const uint8_t *mac, const uint8_t *data, int length) {
      #if !SIM_MODE
    void onReceive(const uint8_t *mac, const uint8_t *data, int length) {
  if (length != (int)sizeof(TimerPacket)) return;

TimerPacket pkt;
memcpy(&pkt, data, sizeof(pkt));

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

  if (c == 'data' || c == 'X') resetToIdle("manual reset");
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