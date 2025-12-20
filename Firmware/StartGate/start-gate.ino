/*
  40-Yard Dash Timer — Start Gate (ESP32)
  - Detects laser beam break via LTR-4206E phototransistor circuit (when hardware exists)
  - SIM_MODE lets you test by typing 'b' in Serial Monitor (no hardware required)
  - Sends ESP-NOW packet to Base Station when beam breaks (when SIM_MODE=0 and MAC is set)
  License: GPL-3.0
*/

#include <WiFi.h>
#include <esp_now.h>
#include "../shared/protocol.h"


#define SIM_MODE 1  // 1 = simulate beam break via Serial, 0 = real sensor + ESP-NOW

// ===================== USER CONFIG =====================
// Placeholder MAC. You will replace this later when you have hardware.
uint8_t BASE_STATION_MAC[6] = { 0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC };

// Pins (edit later to match wiring)
static const int PIN_BEAM = 25;      // Phototransistor output to GPIO
static const int PIN_LED  = 2;       // Built-in LED on many ESP32 dev boards
static const int PIN_BUZ  = -1;      // Set to GPIO if you add buzzer, else -1

// Beam logic
static const bool FORCE_BEAM_PRESENT_IS_HIGH = true;
static const bool USE_AUTO_DETECT_POLARITY = false;

// Debounce / timing
static const uint32_t DEBOUNCE_MS = 15;
static const uint32_t LOCKOUT_MS  = 1200;
static const uint32_t ALIGN_BLINK_MS = 150;

// Packet config
static const uint8_t GATE_ID_START = GATE_START;

// ===================== GLOBALS =====================
static bool beamPresentIsHigh = true;
static uint32_t seqNum = 0;
static uint32_t lastTriggerMs = 0;

// For debounce
static bool lastRawState = false;
static uint32_t stateChangeMs = 0;
static bool armed = true;

// ===================== HELPERS =====================
static inline void setLed(bool on) {
  if (PIN_LED >= 0) digitalWrite(PIN_LED, on ? HIGH : LOW);
}

static void beep(uint16_t ms = 60) {
  if (PIN_BUZ < 0) return;
  digitalWrite(PIN_BUZ, HIGH);
  delay(ms);
  digitalWrite(PIN_BUZ, LOW);
}

static bool readBeamPresent() {
  int raw = digitalRead(PIN_BEAM);
  if (beamPresentIsHigh) return (raw == HIGH);
  else                   return (raw == LOW);
}

static void onSent(const uint8_t *mac, esp_now_send_status_t status) {
  if (status != ESP_NOW_SEND_SUCCESS) {
    setLed(true); delay(40);
    setLed(false); delay(40);
    setLed(true); delay(40);
    setLed(false);
  }
}

static bool sendStartPacket() {
  TimerPacket pkt;
  pkt.gate_id = GATE_ID_START;
  pkt.event_type = EVENT_TRIGGER;
  pkt.seq = ++seqNum;
  pkt.uptime_ms = millis();

  esp_err_t result = esp_now_send(BASE_STATION_MAC, (uint8_t*)&pkt, sizeof(pkt));
  return (result == ESP_OK);
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(115200);
  delay(200);

  if (PIN_LED >= 0) {
    pinMode(PIN_LED, OUTPUT);
    setLed(false);
  }
  if (PIN_BUZ >= 0) {
    pinMode(PIN_BUZ, OUTPUT);
    digitalWrite(PIN_BUZ, LOW);
  }

#if SIM_MODE
  // In SIM_MODE we do NOT require ESP-NOW, MAC addresses, or a sensor.
  Serial.println();
  Serial.println("=== Start Gate (SIM MODE) ===");
  Serial.println("Type 'b' in Serial Monitor to simulate a beam break.");
  Serial.println("When hardware arrives, set SIM_MODE to 0.");
  setLed(true);
  delay(200);
  setLed(false);
  return;
#else
  // Real mode setup:
  pinMode(PIN_BEAM, INPUT_PULLUP);

  if (USE_AUTO_DETECT_POLARITY) {
    int highCount = 0;
    for (int i = 0; i < 40; i++) {
      highCount += (digitalRead(PIN_BEAM) == HIGH) ? 1 : 0;
      delay(2);
    }
    beamPresentIsHigh = (highCount > 20);
  } else {
    beamPresentIsHigh = FORCE_BEAM_PRESENT_IS_HIGH;
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init FAILED");
    while (true) {
      setLed(true); delay(200);
      setLed(false); delay(200);
    }
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, BASE_STATION_MAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add Base Station as peer");
    while (true) {
      setLed(true); delay(60);
      setLed(false); delay(60);
    }
  }

  Serial.println();
  Serial.println("=== Start Gate Boot ===");
  Serial.print("My MAC: ");
  Serial.println(WiFi.macAddress());

  setLed(true); beep(40); delay(120);
  setLed(false);

  lastRawState = readBeamPresent();
  stateChangeMs = millis();
  armed = true;
#endif
}

// ===================== LOOP =====================
void loop() {
#if SIM_MODE
  // Simulate a beam break by typing 'b' into Serial Monitor.
  if (Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'b' || c == 'B') {
      seqNum++;
      Serial.print("SIM: Beam break -> START event, seq=");
      Serial.print(seqNum);
      Serial.print(", uptime_ms=");
      Serial.println(millis());

      // LED blink feedback
      setLed(true); delay(80);
      setLed(false);
    }
  }
  delay(5);
  return;
#else
  const uint32_t now = millis();

  bool beamPresent = readBeamPresent();
  bool beamBroken  = !beamPresent;

  static uint32_t lastBlinkMs = 0;
  static bool blinkState = false;

  if (beamPresent) {
    setLed(true);
  } else {
    if (now - lastBlinkMs >= ALIGN_BLINK_MS) {
      lastBlinkMs = now;
      blinkState = !blinkState;
      setLed(blinkState);
    }
  }

  if (beamPresent != lastRawState) {
    lastRawState = beamPresent;
    stateChangeMs = now;
  }

  if (!armed && (now - lastTriggerMs >= LOCKOUT_MS)) {
    armed = true;
  }

  if (armed && beamBroken && (now - stateChangeMs >= DEBOUNCE_MS)) {
    bool ok = sendStartPacket();
    Serial.print("START trigger sent. ok=");
    Serial.println(ok ? "true" : "false");

    beep(70);

    armed = false;
    lastTriggerMs = now;
    delay(40);
  }

  delay(2);
#endif
}
