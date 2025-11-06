#include <Wire.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "Accelerometer.h"
#include "espconfig.h"


WiFiClientSecure client;
Accelerometer accel;

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
      Serial.println("\n⚠️ Wi-Fi reconnecting...");
      WiFi.disconnect(true);
      delay(2000);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      retries = 0;
    }
  }

  Serial.println("\n✅ Wi-Fi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

bool sendTelegramMessage(const String &text) {
  if (WiFi.status() != WL_CONNECTED) return false;

  client.setInsecure();
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("❌ Telegram connection failed!");
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

  Serial.println("📤 Telegram alert sent!");
  return true;
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  connectToWiFi();
  accel.begin();

  Serial.println("✅ Tamper detection system active.");
  sendTelegramMessage("🟢 ESP32 Tamper Monitor online.");
}

// ---------------- LOOP ----------------
void loop() {
  // Check accelerometer for tampering
  if (accel.isTampered(0.3)) {
    Serial.println("🚨 Tampering detected!");
    sendTelegramMessage("🚨 ALERT: Tampering detected at device!");
    delay(5000);  // avoid flooding Telegram
  } else {
    accel.printAcceleration(false);  // optional live feed
  }

  delay(500);  // sampling interval
}