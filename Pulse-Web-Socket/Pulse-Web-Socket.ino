#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "HUAWEI-nGQ2";
const char* password = "y3mqwUhn";

#define PULSE_PIN 34 // Signal Pulse 34 D34
int Signal;
int lastSignal = 0;
bool beatDetected = false;

long lastBeat = 0;
int BPM;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void notifyClients(int bpm) {
  String msg = "{\"bpm\":" + String(bpm) + "}";
  ws.textAll(msg);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.println(WiFi.localIP());

  ws.onEvent([](AsyncWebSocket * server, AsyncWebSocketClient * client,
                 AwsEventType type, void * arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
      Serial.println("Web client connected");
    }
  });

  server.addHandler(&ws);
  server.begin();
}

void loop() {
  Signal = analogRead(PULSE_PIN);

  //  rising up pulse
  if (lastSignal < 2000 && Signal >= 2000 && !beatDetected) {
    beatDetected = true;
    long now = millis();
    long diff = now - lastBeat;
    lastBeat = now;

    // valid  range 30–200 BPM
    if (diff > 300 && diff < 2000) { 
      BPM = 60000 / diff;
      Serial.println("BPM: " + String(BPM));
      notifyClients(BPM);
    }
  }

  // Falling Pulse
  if (lastSignal > 2000 && Signal <= 2000) {
    beatDetected = false;
  }

  lastSignal = Signal;
  delay(10);
}
