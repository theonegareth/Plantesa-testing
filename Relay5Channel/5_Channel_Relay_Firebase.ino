#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi & Firebase credentials
#define WIFI_SSID       "your_wifi_ssid"
#define WIFI_PASSWORD   "your_wifi_password"
#define API_KEY         "your_api_key"
#define USER_EMAIL      "your_email"
#define USER_PASSWORD   "your_password"
#define DATABASE_URL    "your_database_url"

// Relay pins
#define RELAY1_PIN 32  // Pump 1
#define RELAY2_PIN 33  // Pump 2
#define RELAY3_PIN 25  // Pump 3
#define RELAY4_PIN 26  // Light
#define RELAY5_PIN 27  // Fan

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected: " + WiFi.localIP().toString());
}

// --- Helper functions to upload relay status ---
void sendPumpStatus(int pumpNum, bool isOn) {
  String path = "/pump_status/pump" + String(pumpNum);
  String status = isOn ? "ON" : "OFF";
  Firebase.RTDB.setString(&fbdo, path.c_str(), status);
}

void sendLightStatus(bool isOn) {
  String path = "/light_status";
  String status = isOn ? "ON" : "OFF";
  Firebase.RTDB.setString(&fbdo, path.c_str(), status);
}

void sendFanStatus(bool isOn) {
  String path = "/fan_status";
  String status = isOn ? "ON" : "OFF";
  Firebase.RTDB.setString(&fbdo, path.c_str(), status);
}

void setup() {
  Serial.begin(115200);
  initWiFi();

  // Firebase init
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Configure relay pins
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(RELAY3_PIN, OUTPUT);
  pinMode(RELAY4_PIN, OUTPUT);
  pinMode(RELAY5_PIN, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    int data = Serial.read();
    if (data == 'a') {
      digitalWrite(RELAY1_PIN, LOW);   // turn on Pump 1
      Serial.println("Relay 1 is OFF");
      sendPumpStatus(1, false);
    } else if (data == 'A') {
      digitalWrite(RELAY1_PIN, HIGH);  // turn off Pump 1
      Serial.println("Relay 1 is ON");
      sendPumpStatus(1, true);
    } else if (data == 'b') {
      digitalWrite(RELAY2_PIN, LOW);   // turn on Pump 2
      Serial.println("Relay 2 is OFF");
      sendPumpStatus(2, false);
    } else if (data == 'B') {
      digitalWrite(RELAY2_PIN, HIGH);  // turn off Pump 2
      Serial.println("Relay 2 is ON");
      sendPumpStatus(2, true);
    } else if (data == 'c') {
      digitalWrite(RELAY3_PIN, LOW);   // turn on Pump 3
      Serial.println("Relay 3 is OFF");
      sendPumpStatus(3, false);
    } else if (data == 'C') {
      digitalWrite(RELAY3_PIN, HIGH);  // turn off Pump 3
      Serial.println("Relay 3 is ON");
      sendPumpStatus(3, true);
    } else if (data == 'd') {
      digitalWrite(RELAY4_PIN, LOW);   // turn on Light
      Serial.println("Relay 4 is OFF");
      sendLightStatus(false);
    } else if (data == 'D') {
      digitalWrite(RELAY4_PIN, HIGH);  // turn off Light
      Serial.println("Relay 4 is ON");
      sendLightStatus(true);
    } else if (data == 'e') {
      digitalWrite(RELAY5_PIN, LOW);   // turn on Fan
      Serial.println("Relay 5 is OFF");
      sendFanStatus(false);
    } else if (data == 'E') {
      digitalWrite(RELAY5_PIN, HIGH);  // turn off Fan
      Serial.println("Relay 5 is ON");
      sendFanStatus(true);
    }
  }
}