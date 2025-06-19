#include <Arduino.h>
#include <Wire.h>
#include "DFRobot_SHT40.h"

// --- Pin definitions ---
#define A02YYUW_TX 17   // Ultrasonic TX → ESP32 TX1
#define A02YYUW_RX 16   // Ultrasonic RX ← ESP32 RX1
#define SensorPin   15  // Soil moisture analog pin
#define WATER_LEVEL_PIN 2  // Water level sensor analog pin

const int AirValue   = 300;  // Soil moisture calibration
const int WaterValue = 0;

// UART instances
HardwareSerial modbusSerial(2);  // NPK sensor on UART2 (GPIO4=RX2, GPIO5=TX2)
HardwareSerial ultraSerial(1);   // Ultrasonic sensor on UART1 (GPIO16=RX1, GPIO17=TX1)

// NPK Modbus commands
const byte nitroCmd[] = {0x01,0x03,0x00,0x1e,0x00,0x01,0xe4,0x0c};
const byte phosCmd[]  = {0x01,0x03,0x00,0x1f,0x00,0x01,0xb5,0xcc};
const byte potaCmd[]  = {0x01,0x03,0x00,0x20,0x00,0x01,0x85,0xc0};

// SHT40 instance
DFRobot_SHT40 SHT40(SHT40_AD1B_IIC_ADDR);

// Shared variables
float temperature = 0.0, humidity = 0.0;
int soilMoisturePercent = 0;

// --- Utility: read 16‑bit NPK value via Modbus-RTU ---
uint16_t read_npk(const byte* cmd) {
  modbusSerial.flush();
  modbusSerial.write(cmd, 8);
  vTaskDelay(pdMS_TO_TICKS(100));

  unsigned long start = millis();
  while (modbusSerial.available() < 7) {
    if (millis() - start > 1000) {
      Serial.println("[NPK] Timeout!");
      return 0xFFFF;
    }
    delay(5);
  }

  byte r[7];
  for (int i = 0; i < 7; i++) r[i] = modbusSerial.read();

  Serial.print("[NPK] Resp: ");
  for (auto b : r) Serial.printf("%02X ", b);
  Serial.println();

  return (r[3] << 8) | r[4];
}

// --- Task: Soil Moisture ---
void moistureTask(void* pv) {
  vTaskDelay(pdMS_TO_TICKS(1000));
  while (1) {
    int raw = analogRead(SensorPin);
    soilMoisturePercent = map(raw, AirValue, WaterValue, 0, 100);
    soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
    Serial.printf("[Moisture] %d %%\n", soilMoisturePercent);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// --- Task: SHT40 Temperature & Humidity ---
void shtTask(void* pv) {
  vTaskDelay(pdMS_TO_TICKS(2000));
  while (1) {
    temperature = SHT40.getTemperature(PRECISION_HIGH);
    humidity    = SHT40.getHumidity(PRECISION_HIGH);

    if (temperature == MODE_ERR)
      Serial.println("[SHT40] Temp error");
    else
      Serial.printf("[SHT40] Temp: %.2f °C\n", temperature);

    if (humidity == MODE_ERR)
      Serial.println("[SHT40] Humidity error");
    else
      Serial.printf("[SHT40] Humidity: %.2f %%RH\n", humidity);

    if (humidity > 80)
      SHT40.enHeater(POWER_CONSUMPTION_H_HEATER_1S);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// --- Task: Ultrasonic Water‑Level ---
void ultrasonicTask(void* pv) {
  vTaskDelay(pdMS_TO_TICKS(3000));
  const float alpha     = 0.2f;
  const float maxHeight = 28.0f;
  static float emaDist  = -1;

  while (1) {
    ultraSerial.flush();
    ultraSerial.write(0x55);
    delay(100);

    if (ultraSerial.available() >= 4) {
      uint8_t high = ultraSerial.read();
      uint8_t low  = ultraSerial.read();
      ultraSerial.read(); // discard
      ultraSerial.read(); // discard

      uint16_t raw = (high << 8) | low;
      float cm = raw / 10.0f;
      if (cm > maxHeight) cm = maxHeight;

      if (emaDist < 0) {
        emaDist = cm;
      } else {
        emaDist = alpha * cm + (1.0f - alpha) * emaDist;
        if (emaDist > maxHeight) emaDist = maxHeight;
      }

      Serial.printf("[Ultrasonic] raw=%.1f cm  ema=%.1f cm (max %.1f cm)\n", cm, emaDist, maxHeight);
    } else {
      Serial.println("[Ultrasonic] No valid data received");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// --- Task: NPK Readings ---
void npkTask(void* pv) {
  vTaskDelay(pdMS_TO_TICKS(5000));
  while (1) {
    Serial.println("\n[NPK] Reading values...");
    uint16_t N = read_npk(nitroCmd);
    uint16_t P = read_npk(phosCmd);
    uint16_t K = read_npk(potaCmd);

    if (N != 0xFFFF) Serial.printf("[NPK] Nitrogen: %u mg/kg\n", N);
    if (P != 0xFFFF) Serial.printf("[NPK] Phosphorous: %u mg/kg\n", P);
    if (K != 0xFFFF) Serial.printf("[NPK] Potassium: %u mg/kg\n", K);

    vTaskDelay(pdMS_TO_TICKS(3000));
  }
}

// --- Task: Analog Water Level Sensor ---
void waterLevelTask(void* pv) {
  const int maxValue = 1450;
  vTaskDelay(pdMS_TO_TICKS(1500));  // startup delay

  while (1) {
    int rawLevel = analogRead(WATER_LEVEL_PIN);
    if (rawLevel > maxValue) rawLevel = maxValue;

    int percentLevel = (rawLevel * 100) / maxValue;
    Serial.printf("[Water Level] %d %%\n", percentLevel);

    if (percentLevel >= 100)
      Serial.println("[Water Level] ⚠️ WARNING: Water level FULL!");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  ultraSerial.begin(9600, SERIAL_8N1, A02YYUW_RX, A02YYUW_TX);
  Wire.begin();
  SHT40.begin();
  Serial.printf("SHT40 ID: 0x%08X\n", SHT40.getDeviceID());

  modbusSerial.begin(4800, SERIAL_8N1, 4, 5);

  xTaskCreate(moistureTask,   "Moisture", 2048, NULL, 1, NULL);
  xTaskCreate(shtTask,        "SHT40",    4096, NULL, 1, NULL);
  xTaskCreate(ultrasonicTask, "Ultra",    4096, NULL, 1, NULL);
  xTaskCreate(npkTask,        "NPK",      4096, NULL, 1, NULL);
  xTaskCreate(waterLevelTask, "WaterLvl", 2048, NULL, 1, NULL);  // 👈 New task added here
}

void loop() {
  // everything runs in tasks
}
