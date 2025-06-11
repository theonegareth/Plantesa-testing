#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// WiFi credentials
#define WIFI_SSID "HGG22 (2G)"
#define WIFI_PASSWORD "JPilot123"

// Firebase project credentials
#define API_KEY "AIzaSyBxx9Ni57aDmUisdcPnaopCQVslvdmup-c"
#define USER_EMAIL "theonegareth@gmail.com"
#define USER_PASSWORD "adminadmin"
#define DATABASE_URL "https://plantesa-c3798-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String uid;
String databasePath;
String tempPath, humPath, moisPath, natPath, phosPath, kalPath, waterPath;

float temperature, humidity, moisture, natrium, phosporus, kalium, water;

unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000; // 3 minutes

void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.println("Connected: " + WiFi.localIP().toString());
}

void sendFloat(String path, float value){
  if (Firebase.RTDB.setFloat(&fbdo, path.c_str(), value)) {
    Serial.print("Sent ");
    Serial.print(value);
    Serial.print(" to ");
    Serial.println(path);
  } else {
    Serial.print("FAILED to send to ");
    Serial.print(path);
    Serial.print(". Reason: ");
    Serial.println(fbdo.errorReason());
  }
}

void setup() {
  Serial.begin(115200);
  initWiFi();

  config.api_key = API_KEY;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);
  config.token_status_callback = tokenStatusCallback;
  config.max_token_generation_retry = 5;

  Firebase.begin(&config, &auth); 

  Serial.print("Getting user UID");
  while (auth.token.uid == "") {
    Serial.print(".");
    delay(1000);
  }
  uid = auth.token.uid.c_str();
  Serial.println();
  Serial.println("User UID: " + uid);

  databasePath = "/SensorsData/" + uid;

  humPath = databasePath + "/humidity";
  tempPath = databasePath + "/temperature";
  moisPath = databasePath + "/moisture";

  natPath = databasePath + "/natrium";
  phosPath = databasePath + "/phosphorus";
  kalPath = databasePath + "/kalium";
  
  waterPath = databasePath + "/waterlevel";
}

void loop() {
  if (Firebase.ready() && (millis() - sendDataPrevMillis > timerDelay || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();

    // Fake sensor values
    humidity = random(70, 90);
    temperature = random(25, 30);
    moisture = random(50, 60);

    natrium = random(100, 150);
    phosporus = random(60, 80);
    kalium = random(50, 60);

    water = random(27, 26);

    sendFloat(humPath, humidity);
    sendFloat(tempPath, temperature);
    sendFloat(moisPath, moisture);

    sendFloat(natPath, natrium);
    sendFloat(phosPath, phosporus);
    sendFloat(kalPath, kalium);
    sendFloat(waterPath, water);

  }
}
