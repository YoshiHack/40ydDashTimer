  #include <WiFi.h>
  #include <esp_now.h>
  #include "../shared/protocol.h"

#define SIM_MODE 1

uint8_t BASE_STATION_MAC[6] = { 0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC };

static const int PIN_BEAM = 25;
static const int PIN_LED  = 2;
static const int PIN_BUZ  = -1;

static const bool FORCE_BEAM_PRESENT_IS_HIGH = true;
static const bool USE_AUTO_DETECT_POLARITY = false;

static const uint32_t DEBOUNCE_MS = 15;
static const uint32_t LOCKOUT_MS  = 1200;
static const uint32_t ALIGN_BLINK_MS = 150;

static const uint8_t GATE_ID_START = GATE_START;

static bool beamPresentIsHigh = true;
static uint32_t seqNum = 0;
static uint32_t lastTriggerMs = 0;

static bool lastRawState = false;
static uint32_t stateChangeMs = 0;
static bool armed = true;

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

    Serial.println();
    Serial.println("=== Start Gate (SIM MODE) ===");
  Serial.println("Type 'b' in Serial Monitor to simulate a beam break.");
Serial.println("When hardware arrives, set SIM_MODE to 0.");
setLed(true);
  delay(200);
  setLed(false);
  return;
  #else

  pinMode(PIN_BEAM, INPUT_PULLUP);

  if (USE_AUTO_DETECT_POLARITY) {
  int highCount = 0;
for (int idx = 0; idx < 40; idx++) {
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

  void loop() {
  #if SIM_MODE

if (Serial.available()) {
char c = (char)Serial.read();
if (c == 'b' || c == 'B') {
seqNum++;
Serial.print("SIM: Beam break -> START event, seq=");
  Serial.print(seqNum);
  Serial.print(", uptime_ms=");
    Serial.println(millis());

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