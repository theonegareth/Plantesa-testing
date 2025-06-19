#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#define WIFI_SSID "Gareth"
#define WIFI_PASSWORD "12345678"

unsigned long lastSend = 0;
const unsigned long sendInterval = 180000;

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

void setup() {
  Serial.begin(115200);
  delay(100);
  initWiFi();

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  while (now < 1600000000) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("\nTime synced!");

  char timeFormatted[25];
  struct tm* tm_info = localtime(&now);
  strftime(timeFormatted, sizeof(timeFormatted), "%Y-%m-%d_%H-%M", tm_info);
  Serial.print("[Formatted Time] ");
  Serial.println(timeFormatted);
}

void loop() {
  // Nothing
}
