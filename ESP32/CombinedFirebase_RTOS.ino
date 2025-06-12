#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "DFRobot_SHT40.h"
#include <SoftwareSerial.h>

// ——— WiFi & Firebase credentials ————————————————————————
#define WIFI_SSID       "YOUR_WIFI_SSID"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
#define API_KEY         "YOUR_API_KEY"
#define USER_EMAIL      "YOUR_USER_EMAIL"
#define USER_PASSWORD   "YOUR_USER_PASSWORD"
#define DATABASE_URL    "YOUR_DATABASE_URL"

// ——— Firebase objects ———————————————————————————
FirebaseData   fbdo;
FirebaseAuth   auth;
FirebaseConfig config;
String         uid, basePath;

// ——— Shared sensor values ————————————————————————
float temperature = 0.0;
float humidity    = 0.0;
float moisture    = 0.0;
float natrium     = 0.0;
float phosphorus  = 0.0;
float kalium      = 0.0;
float waterLevel  = 0.0;

// ——— Timing ————————————————————————————————
unsigned long lastSend = 0;
const unsigned long sendInterval = 180000; // 3 minutes

// ——— Pins & Constants ——————————————————————————
// Soil moisture
const int AirValue   = 300;
const int WaterValue =   0;
const int SensorPin  = 15;

// SHT40
DFRobot_SHT40 sht40(SHT40_AD1B_IIC_ADDR);

// Ultrasonic (A02YYUW) on UART2
#define U_TX 17
#define U_RX 16
HardwareSerial mySerial(2);

// NPK (RS-485 Modbus)
#define RE_PIN 8
#define DE_PIN 7
SoftwareSerial modbusSer(2, 3); // RX, TX
const byte nitroCmd[] = {0x01,0x03,0x00,0x1e,0x00,0x01,0xe4,0x0c};
const byte phosCmd[]  = {0x01,0x03,0x00,0x1f,0x00,0x01,0xb5,0xcc};
const byte potaCmd[]  = {0x01,0x03,0x00,0x20,0x00,0x01,0x85,0xc0};

// ——— Helpers ——————————————————————————————
void initWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi up: " + WiFi.localIP().toString());
}

bool sendFloat(const String &path, float val) {
  if (Firebase.RTDB.setFloat(&fbdo, path, val)) {
    Serial.printf("→ %s = %.2f\n", path.c_str(), val);
    return true;
  }
  else {
    Serial.printf("! Failed %s: %s\n",
                  path.c_str(),
                  fbdo.errorReason().c_str());
    return false;
  }
}

uint16_t readModbus(const byte *cmd) {
  digitalWrite(DE_PIN, HIGH);
  digitalWrite(RE_PIN, HIGH);
  delay(5);
  modbusSer.write(cmd, 8);
  delay(5);
  digitalWrite(DE_PIN, LOW);
  digitalWrite(RE_PIN, LOW);

  unsigned long start = millis();
  while (modbusSer.available() < 7) {
    if (millis() - start > 1000) return 0xFFFF;
  }
  byte buf[7];
  for (uint8_t i=0; i<7; i++) buf[i] = modbusSer.read();
  return (uint16_t)(buf[3] << 8) | buf[4];
}

// ——— Tasks ————————————————————————————————————
void moistureTask(void*) {
  while (1) {
    int raw = analogRead(SensorPin);
    moisture = map(raw, AirValue, WaterValue, 0, 100);
    moisture = constrain(moisture, 0, 100);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void shtTask(void*) {
  while (1) {
    float t = sht40.getTemperature(PRECISION_HIGH);
    float h = sht40.getHumidity   (PRECISION_HIGH);
    if (t != MODE_ERR) temperature = t;
    if (h != MODE_ERR) humidity    = h;
    if (humidity > 80)               // heater if very humid
      sht40.enHeater(POWER_CONSUMPTION_H_HEATER_1S);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void ultrasonicTask(void*) {
  while (1) {
    if (mySerial.available() >= 4) {
      uint8_t d[4];
      for (uint8_t i=0; i<4; i++) d[i] = mySerial.read();
      if (d[0]==0xFF && (((d[0]+d[1]+d[2]) & 0xFF)==d[3])) {
        int cm10 = (d[1]<<8) | d[2];
        float cm  = cm10 / 10.0;
        waterLevel = (cm > 3.0) ? cm : 0.0;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void npkTask(void*) {
  while (1) {
    uint16_t N = readModbus(nitroCmd);
    uint16_t P = readModbus(phosCmd);
    uint16_t K = readModbus(potaCmd);
    if (N != 0xFFFF) natrium    = N;
    if (P != 0xFFFF) phosphorus = P;
    if (K != 0xFFFF) kalium     = K;
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// ——— Setup & Loop —————————————————————————————
void setup() {
  Serial.begin(115200);
  initWiFi();

  // Firebase
  config.api_key            = API_KEY;
  config.database_url       = DATABASE_URL;
  auth.user.email           = USER_EMAIL;
  auth.user.password        = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // wait for UID
  Serial.print("Waiting for UID");
  while (auth.token.uid == "") {
    Serial.print(".");
    delay(500);
  }
  uid      = auth.token.uid.c_str();
  basePath = "/SensorsData/" + uid + "/";

  // init sensors & comms
  sht40.begin();
  mySerial.begin(9600, SERIAL_8N1, U_RX, U_TX);
  pinMode(DE_PIN, OUTPUT);
  pinMode(RE_PIN, OUTPUT);
  digitalWrite(DE_PIN, LOW);
  digitalWrite(RE_PIN, LOW);
  modbusSer.begin(4800);

  // spawn FreeRTOS tasks
  xTaskCreate(moistureTask,   "Moisture",   2048, NULL, 1, NULL);
  xTaskCreate(shtTask,        "SHT40",      4096, NULL, 1, NULL);
  xTaskCreate(ultrasonicTask, "Ultrasonic", 4096, NULL, 1, NULL);
  xTaskCreate(npkTask,        "NPK",        4096, NULL, 1, NULL);
}

void loop() {
  if (millis() - lastSend >= sendInterval) {
    lastSend = millis();
    Serial.println("\n>>> Sending to Firebase");
    sendFloat(basePath + "temperature", temperature);
    sendFloat(basePath + "humidity",    humidity);
    sendFloat(basePath + "moisture",    moisture);
    sendFloat(basePath + "natrium",     natrium);
    sendFloat(basePath + "phosphorus",  phosphorus);
    sendFloat(basePath + "kalium",      kalium);
    sendFloat(basePath + "waterlevel",  waterLevel);
  }
}
