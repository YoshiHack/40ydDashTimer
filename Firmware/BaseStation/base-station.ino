#include <esp_now.h>
#include <WiFi.h>

struct GatePacket {
  char gate[7];       // "start" or "finish"
  uint64_t timestamp; 
};

volatile uint64_t startTime = 0;
volatile uint64_t finishTime = 0;

void onReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  GatePacket packet;
  memcpy(&packet, incomingData, sizeof(packet));

  Serial.print("Received from gate: ");
  Serial.println(packet.gate);
  Serial.print("Timestamp: ");
  Serial.println(packet.timestamp);

  if (strcmp(packet.gate, "start") == 0) {
    startTime = packet.timestamp;
  }

  if (strcmp(packet.gate, "finish") == 0) {
    finishTime = packet.timestamp;

    if (startTime > 0) {
      double timeSec = (finishTime - startTime) / 1000000.0;
      Serial.print("40-Yard Dash Time: ");
      Serial.print(timeSec, 4);
      Serial.println(" sec");
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.println("Base Station Node");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed!");
    return;
  }

  esp_now_register_recv_cb(onReceive);

  // Print MAC so you can add it to the gate nodes
  Serial.print("Base Station MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  // Base station mostly waits for packets
}

