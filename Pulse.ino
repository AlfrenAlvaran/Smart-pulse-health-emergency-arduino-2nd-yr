const int PULSE_PIN = 34;
int signalValue = 0;
unsigned long lastBeat = 0;
int BPM = 0;
bool beatDetected = false;
unsigned long lastValidBeat = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  signalValue = analogRead(PULSE_PIN);
  unsigned long currentTime = millis();

  if (!beatDetected && signalValue > 2000) {
    if (currentTime - lastBeat > 400) {
      BPM = 60000 / (currentTime - lastBeat);
      lastBeat = currentTime;
      lastValidBeat = currentTime;
      Serial.print("BPM: ");
      Serial.print(BPM);

      if (BPM < 60) {
        Serial.println(" → Low");
      } else if (BPM > 100) {
        Serial.println(" → High");
      } else {
        Serial.println(" → Normal");
      }
    }
    beatDetected = true;
  }

  if (beatDetected && signalValue < 1500) {
    beatDetected = false;
  }

  if (currentTime - lastValidBeat > 5000) {
    Serial.println("No finger detected");
    lastValidBeat = currentTime;
  }

  delay(20);
}
