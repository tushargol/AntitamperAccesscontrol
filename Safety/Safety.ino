#include "keypad.h"
#include "ServoGate.h"
#include "Accelerometer.h"
#include "RFIDModule.h"
#include "config.h"
#include <Wire.h>

Keypad keypad;
ServoGate gate;
Accelerometer accel;
RFIDModule rfid;

// RFID authorized UID
const String AUTH_UID = "62 43 10 005";  // Change this to your real UID (check Serial Monitor)

char entered[5];
uint8_t index = 0;
bool rfidVerified = false;

void setup() {
  Serial.begin(9600);
  
  Wire.begin();
  Wire.setClock(100000);
  
  keypad.begin();
  gate.begin();
  accel.begin();
  rfid.begin();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  pinMode(TAMPER_ALERT_PIN, OUTPUT);
  digitalWrite(TAMPER_ALERT_PIN, LOW);

  Serial.println(F("🔰 System Initialized"));
  Serial.println(F("Tap RFID card to begin..."));
}

// 🧩 Buzzer feedback
void buzzerAccessGranted() {
  tone(BUZZER_PIN, 1000, 150);
  delay(150);
  tone(BUZZER_PIN, 1500, 150);
  delay(150);
  noTone(BUZZER_PIN);
}

void buzzerAccessDenied() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 300, 150);
    delay(150);
    noTone(BUZZER_PIN);
  }
}

void buzzerTamperAlarm() {
  for (int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, 600, 200);
    delay(100);
    tone(BUZZER_PIN, 200, 200);
    delay(100);
  }
  noTone(BUZZER_PIN);
}

void sendTamperToESP32() {
  Wire.beginTransmission(0x08);
  Wire.write(0x01);
  Wire.endTransmission();
}

void loop() {
  // 🛡️ Tamper check — always active
  if (accel.isTampered()) {
    Serial.println(F("🚨 Tampering detected! Triggering alarm!"));
    digitalWrite(TAMPER_ALERT_PIN, HIGH);
    sendTamperToESP32();
    buzzerTamperAlarm();
    delay(500);
    digitalWrite(TAMPER_ALERT_PIN, LOW);
    return;
  }

  // RFID authentication first
  if (!rfidVerified) {
    if (rfid.pollCard()) {  // new card detected
      Serial.print(F("UID tag : "));
      Serial.println(rfid.getUID());

      if (rfid.verifyCard(AUTH_UID)) {
        rfidVerified = true;
        Serial.println(F("✅ RFID verified! Proceed to enter passcode."));
        buzzerAccessGranted();
      } else {
        Serial.println(F("❌ Unauthorized RFID!"));
        buzzerAccessDenied();
      }
    }
    delay(300);
    return;
  }

  // Keypad entry after valid RFID
  char key = keypad.getKey();

  if (key) {
    Serial.print(F("Key Pressed: "));
    Serial.println(key);

    if (key == '#') {
      entered[index] = '\0';
      if (keypad.checkPasscode()) {
        Serial.println(F("Access Granted"));
        buzzerAccessGranted();
        gate.openGate();
        delay(2000);
        gate.closeGate();
        rfidVerified = false; // reset for next use
        Serial.println(F("System locked. Tap RFID to start again."));
      } else {
        Serial.println(F("Access Denied"));
        buzzerAccessDenied();
      }
      index = 0;
    } else if (key == '*') {
      index = 0;
      Serial.println(F("Input cleared"));
    } else if (index < 4) {
      entered[index++] = key;
    }
  }

  delay(100);
}
