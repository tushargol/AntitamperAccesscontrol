#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include "espconfig.h"

#define ESP32_I2C_ADDRESS 0x08  // ESP32 as I2C slave

WiFiClientSecure client;
volatile bool tamperDetected = false;

// I2C Slave receive callback
void receiveEvent(int numBytes) {
  while (Wire.available()) {
    uint8_t data = Wire.read();
    if (data == 0x01) {  // Tamper signal from Arduino
      tamperDetected = true;
      Serial.println("   I2C: Tamper signal received from Arduino");
    }
  }
}

// ---------------- Wi-Fi + Telegram ----------------
void connectToWiFi() {
  Serial.printf("   Connecting to %s", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++retries > 40) {
      Serial.println("\n   ⚠️ Connection timeout, reconnecting...");
      WiFi.disconnect(true);
      delay(2000);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      retries = 0;
    }
  }

  Serial.println("\n   ✓ WiFi connected!");
  Serial.print("   IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("   MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("   Signal Strength: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n╔═══════════════════════════════════════╗");
  Serial.println("║   ESP32 Anti-Tamper System v1.0     ║");
  Serial.println("╚═══════════════════════════════════════╝\n");

  // Debug: I2C Configuration as SLAVE
  Serial.println("🔧 I2C Configuration:");
  Serial.printf("   Mode: I2C SLAVE\n");
  Serial.printf("   Slave Address: 0x%02X\n", ESP32_I2C_ADDRESS);
  Serial.printf("   SDA Pin: GPIO %d\n", SDA_PIN);
  Serial.printf("   SCL Pin: GPIO %d\n", SCL_PIN);
  
  // Initialize I2C as slave
  Wire.begin(ESP32_I2C_ADDRESS, SDA_PIN, SCL_PIN, 100000);
  Wire.onReceive(receiveEvent);
  Serial.println("   I2C Slave initialized ✓\n");
  
  // Debug: WiFi Configuration
  Serial.println("📡 WiFi Configuration:");
  Serial.printf("   SSID: %s\n", WIFI_SSID);
  Serial.println("   Password: [HIDDEN]");
  
  // Debug: Telegram Configuration
  Serial.println("\n💬 Telegram Configuration:");
  Serial.printf("   Bot Token: %s...%s\n", 
    String(TELEGRAM_BOT_TOKEN).substring(0, 10).c_str(),
    String(TELEGRAM_BOT_TOKEN).substring(String(TELEGRAM_BOT_TOKEN).length() - 10).c_str());
  Serial.printf("   Chat ID: %s\n", TELEGRAM_CHAT_ID);
  Serial.printf("   Alert Cooldown: %lu ms\n\n", TELEGRAM_ALERT_MIN_INTERVAL_MS);

  // Connect to WiFi
  Serial.println("🌐 Connecting to WiFi...");
  connectToWiFi();

  Serial.println("\n✅ ESP32 Tamper Monitor online.");
  Serial.println("🔍 Waiting for tamper signals from Arduino via I2C...\n");
  sendTelegramMessageAsync("ESP32 Tamper Monitor online and ready");
}

void sendTelegramMessageAsync(const String &text) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("   ✗ WiFi not connected, cannot send message");
    return;
  }

  Serial.println("   📤 Connecting to Telegram API...");
  client.setInsecure();
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("   ✗ Telegram connection failed!");
    return;
  }
  Serial.println("   ✓ Connected to api.telegram.org");

  String url = "/bot" + String(TELEGRAM_BOT_TOKEN) + "/sendMessage";
  String body = "chat_id=" + String(TELEGRAM_CHAT_ID) + "&text=" + text;

  Serial.printf("   Message length: %d bytes\n", body.length());

  String request =
    "POST " + url + " HTTP/1.1\r\n" +
    "Host: api.telegram.org\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "Content-Length: " + String(body.length()) + "\r\n" +
    "Connection: close\r\n\r\n" +
    body;

  client.print(request);
  
  // Don't wait for full response - just send and continue
  Serial.println("   ✓ Alert sent (not waiting for confirmation)");
  client.stop();
}

void loop() {
  // Check if tamper signal received from Arduino via I2C
  if (tamperDetected) {
    Serial.println("🚨 TAMPERING DETECTED (via I2C from Arduino)!");
    Serial.println("   Sending Telegram alert...");
    
    // Send immediately without waiting for response
    sendTelegramMessageAsync("ALERT: Tampering detected at device");
    
    tamperDetected = false;  // Reset flag immediately
  }
  
  delay(100);
}