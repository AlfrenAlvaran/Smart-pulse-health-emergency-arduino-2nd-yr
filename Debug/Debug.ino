#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareSerial.h>

Adafruit_SSD1306 display(128, 64, &Wire);
MAX30105 particleSensor;

HardwareSerial sim800(2); // RX=16, TX=17

int BPM = 0;
unsigned long lastBeat = 0;
bool dangerSent = false;
String phoneNumber = "+639756037560";

unsigned long lastDisplay = 0;
const unsigned long displayInterval = 1000;

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Initializing...");
  display.display();

  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found. Please check wiring/power.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("MAX30102 not found!");
    display.display();
    for (;;);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0);

  sim800.begin(9600, SERIAL_8N1, 16, 17); // RX=16, TX=17
  delay(8000);

  sendCommand("AT");
  sendCommand("AT+CMGF=1");
  sendCommand("AT+CREG?");
  sendCommand("AT+CSQ");
}

void loop() {
  long irValue = particleSensor.getIR();

  if (irValue < 50000) {
    BPM = 0;
    dangerSent = false;
  } else {
    if (checkForBeat(irValue)) {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      BPM = 60 / (delta / 1000.0);
    }
  }

  if (millis() - lastDisplay >= displayInterval) {
    lastDisplay = millis();

    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);

    if (irValue < 50000) {
      Serial.println("No finger detected");
      display.println("Place finger");
    } else {
      Serial.print("BPM: ");
      Serial.println(BPM);

      display.println("Heart Rate:");
      display.setCursor(0, 20);
      display.setTextSize(2);
      if (BPM > 0) {
        display.print(BPM);
        display.print(" BPM");
      } else {
        display.print("-- BPM");
      }

      if (BPM > 0 && (BPM < 50 || BPM > 120)) {
        Serial.println("Danger: BPM out of safe range!");
        display.setCursor(0, 50);
        display.setTextSize(1);
        display.println("!!! DANGER ALERT !!!");

        if (!dangerSent) {
          bool result = sendSMS("ALERT: Heart rate abnormal! BPM=" + String(BPM));
          dangerSent = result;
        }
      } else {
        dangerSent = false;
      }
    }

    display.display();
  }
}

void sendCommand(String cmd) {
  sim800.println(cmd);
  delay(500);
  while (sim800.available()) {
    Serial.write(sim800.read());
  }
}

bool sendSMS(String message) {
  Serial.println("Sending SMS...");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Sending SMS...");
  display.display();

  sim800.println("AT+CMGF=1");
  sim800.println("AT");
  delay(500);
  String resp = readResponse();

  sim800.print("AT+CMGS=\"");
  sim800.print(phoneNumber);
  sim800.println("\"");
  delay(500);
  resp = readResponse();

  sim800.print(message);
  delay(500);
  sim800.write(26);
  delay(5000);

  resp = readResponse();

  bool success = resp.indexOf("+CMGS:") >= 0 && resp.indexOf("OK") >= 0;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);

  if (success) {
    Serial.println("SMS SENT SUCCESSFULLY ✅");
    display.println("SMS SENT");
  } else {
    Serial.println("SMS FAILED ❌");
    Serial.println("Response: " + resp);
    display.println("SMS FAILED");
  }
  display.display();

  return success;
}

String readResponse() {
  String resp = "";
  unsigned long timeout = millis() + 5000;
  while (millis() < timeout) {
    while (sim800.available()) {
      char c = sim800.read();
      resp += c;
    }
  }
  if (resp.length() > 0) {
    Serial.println("SIM800 Resp: " + resp);
  }
  return resp;
}




