#include <esp_now.h>
#include <WiFi.h>

uint8_t baseStationMAC[] = {0x24, 0x6F, 0x28, 0xAA, 0xBB, 0xCC};  
// TODO: Replace with actual Base Station MAC address

struct GatePacket {
  char gate[7];    // "finish"
  uint64_t timestamp;
};

GatePacket packet;

void onSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Packet sent: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.println("Finish Gate Node");

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Init Failed!");
    return;
  }

  esp_now_register_send_cb(onSent);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, baseStationMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  esp_now_add_peer(&peerInfo);

  pinMode(0, INPUT_PULLUP);  
  // TODO: Change this to your actual laser receiver pin
}

void loop() {
  if (digitalRead(0) == LOW) {
    strcpy(packet.gate, "finish");
    packet.timestamp = esp_timer_get_time();

    esp_now_send(baseStationMAC, (uint8_t*)&packet, sizeof(packet));
    Serial.println("Finish event sent!");

    delay(300); 
  }
}

