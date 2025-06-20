#include <Arduino.h>
#include <time.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi credentials
#define WIFI_SSID       "WIFI_SSID"
#define WIFI_PASSWORD   "WIFI_PASSWORD"

// Firebase credentials
#define API_KEY         "API_KEY"
#define USER_EMAIL      "USER_EMAIL"
#define USER_PASSWORD   "USER_PASSWORD"
#define DATABASE_URL    "DATABASE_URL"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Data paths
String uid;
String basePath;
String humPath, tempPath, moisPath;
String natPath, phosPath, kalPath;
String waterPath, wateranalogPath;
String relayBasePath;

// Sensor vars
float humidity, temperature, moisture;
float natrium, phosphorus, kalium;
float water, wateranalog;

// Relay states
bool pump1, pump2, pump3, light, fan;

unsigned long lastMillis = 0;
const unsigned long interval = 180000; // 3 minutes

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("."); delay(500);
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());
}

String getTimestamp() {
  time_t raw;
  time(&raw);
  struct tm tmInfo;
  localtime_r(&raw, &tmInfo);
  char buf[20];
  strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M", &tmInfo);
  return String(buf);
}

void sendFloat(const String &path, float v) {
  if (!Firebase.RTDB.setFloat(&fbdo, path.c_str(), v)) {
    Serial.printf("Error %s: %s\n", path.c_str(), fbdo.errorReason().c_str());
  }
}

void sendRelayStatus(const String &name, bool state) {
  String path = relayBasePath + "/" + name;
  String status = state ? "ON" : "OFF";
  if (!Firebase.RTDB.setString(&fbdo, path.c_str(), status)) {
    Serial.printf("Error %s: %s\n", path.c_str(), fbdo.errorReason().c_str());
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi();

  // NTP sync UTC+7
  const long gmtOffset = 7 * 3600;
  const int dstOffset = 0;
  configTime(gmtOffset, dstOffset, "pool.ntp.org", "time.nist.gov");
  Serial.print("Sync time");
  time_t ts;
  do {
    Serial.print("."); delay(500);
    ts = time(nullptr);
  } while (ts < 100000);
  Serial.println("\nTime: " + getTimestamp());

  // Firebase init
  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;
  Firebase.begin(&config, &auth);

  // wait for sign-in
  Serial.print("UID");
  while (auth.token.uid == "") { Serial.print("."); delay(500); }
  uid = auth.token.uid.c_str();
  Serial.println(" -> " + uid);

  basePath = "/SensorsData/" + uid;
  humPath  = basePath + "/humidity";
  tempPath = basePath + "/temperature";
  moisPath = basePath + "/moisture";
  natPath  = basePath + "/natrium";
  phosPath = basePath + "/phosphorus";
  kalPath  = basePath + "/kalium";
  waterPath       = basePath + "/waterlevel";
  wateranalogPath = basePath + "/wateranalog";
  relayBasePath   = basePath + "/relays";
}

void loop() {
  if (!Firebase.ready() || millis() - lastMillis < interval) return;
  lastMillis = millis();

  // Simulate sensor values
  humidity    = random(70, 90);
  temperature = random(25, 30);
  moisture    = random(50, 60);
  natrium     = random(100,150);
  phosphorus  = random(60, 80);
  kalium      = random(50, 60);
  water       = random(20,26);
  wateranalog = random(0,10);

  sendFloat(humPath, humidity);
  sendFloat(tempPath, temperature);
  sendFloat(moisPath, moisture);
  sendFloat(natPath, natrium);
  sendFloat(phosPath, phosphorus);
  sendFloat(kalPath, kalium);
  sendFloat(waterPath, water);
  sendFloat(wateranalogPath, wateranalog);

  // Simulate fake relay states (randomized ON/OFF)
  pump1 = random(0, 2);
  pump2 = random(0, 2);
  pump3 = random(0, 2);
  light = random(0, 2);
  fan   = random(0, 2);

  sendRelayStatus("pump1", pump1);
  sendRelayStatus("pump2", pump2);
  sendRelayStatus("pump3", pump3);
  sendRelayStatus("light", light);
  sendRelayStatus("fan", fan);

  // Timestamp
  String tPath = basePath + "/timestamp";
  Firebase.RTDB.setString(&fbdo, tPath.c_str(), getTimestamp());
}
