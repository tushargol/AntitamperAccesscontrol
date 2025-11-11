#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "espconfig.h"

#define ADXL345_ADDRESS 0x53

WiFiClientSecure client;

// ---------------- Wi-Fi + Telegram ----------------
void connectToWiFi() {
  Serial.printf("Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++retries > 40) {
      Serial.println("\nWi-Fi reconnecting...");
      WiFi.disconnect(true);
      delay(2000);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      retries = 0;
    }
  }

  Serial.println("\n Wi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

bool sendTelegramMessage(const String &text) {
  if (WiFi.status() != WL_CONNECTED) return false;

  client.setInsecure();
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println(" Telegram connection failed!");
    return false;
  }

  String url = "/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage";
  String body = "chat_id=" + String(TELEGRAM_CHAT_ID) + "&text=" + text;

  String request =
    "POST " + url + " HTTP/1.1\r\n" +
    "Host: api.telegram.org\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "Content-Length: " + String(body.length()) + "\r\n" +
    "Connection: close\r\n\r\n" +
    body;

  client.print(request);
  delay(400);

  Serial.println(" Telegram alert sent!");
  return true;
}

void initAccelerometer() {
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(0x2D);
  Wire.write(0x08);
  Wire.endTransmission();
}

bool checkTamper() {
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(0x32);
  Wire.endTransmission();
  Wire.requestFrom(ADXL345_ADDRESS, 6);
  
  if (Wire.available() >= 6) {
    int16_t x = Wire.read() | (Wire.read() << 8);
    int16_t y = Wire.read() | (Wire.read() << 8);
    int16_t z = Wire.read() | (Wire.read() << 8);
    
    float magnitude = sqrt(x*x + y*y + z*z);
    return magnitude > 400;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);
  initAccelerometer();
  connectToWiFi();

  Serial.println("ESP32 Tamper Monitor online.");
  sendTelegramMessage("ESP32 Tamper Monitor online.");
}

void loop() {
  if (checkTamper()) {
    Serial.println("🚨 Tampering detected!");
    sendTelegramMessage("🚨 ALERT: Tampering detected at device!");
    delay(5000);
  }
  delay(500);
}