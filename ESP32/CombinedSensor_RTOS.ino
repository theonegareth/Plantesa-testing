#include "DFRobot_SHT40.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

// --- Pin definitions ---
// A02YYUW ultrasonic sensor on UART2
#define A02YYUW_TX 17  
#define A02YYUW_RX 16  

// Soil moisture sensor
const int AirValue = 300;    
const int WaterValue = 0;    
const int SensorPin = 15;

// NPK sensor (Modbus-RTU over RS-485)
#define RE 8  
#define DE 7  
SoftwareSerial mod(2, 3);  // RX, TX

// -- Modbus commands for N, P, K (each 8 bytes) --
const byte nitroCmd[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phosCmd[]  = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte potaCmd[]  = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// Create UART2 for ultrasonic
HardwareSerial mySerial(2);

// SHT40
DFRobot_SHT40 SHT40(SHT40_AD1B_IIC_ADDR);

// Shared variables
float temperature = 0.0, humidity = 0.0;
int soilMoisturePercent = 0;

// --- Utility to send a Modbus query and parse a 16-bit response ---
uint16_t read_npk(const byte* cmd) {
  // Enable RS-485 driver
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);

  mod.write(cmd, 8);
  delay(10);

  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);

  unsigned long start = millis();
  while (mod.available() < 7) {
    if (millis() - start > 2000) {
      Serial.println("[NPK] Timeout waiting for response");
      return 0xFFFF;
    }
    delay(5);
  }

  byte resp[7];
  for (int i = 0; i < 7; i++) resp[i] = mod.read();

  // Check CRC or length here if desired...

  // Bytes 3 & 4 = data
  uint16_t value = (resp[3] << 8) | resp[4];
  return value;
}

// --- Task 1: Soil moisture ---
void moistureTask(void* pv) {
  while (1) {
    int raw = analogRead(SensorPin);
    soilMoisturePercent = map(raw, AirValue, WaterValue, 0, 100);
    soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);
    Serial.printf("[Moisture] %d %%\n", soilMoisturePercent);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// --- Task 2: SHT40 temperature & humidity ---
void shtTask(void* pv) {
  while (1) {
    temperature = SHT40.getTemperature(PRECISION_HIGH);
    humidity    = SHT40.getHumidity(PRECISION_HIGH);

    if (temperature == MODE_ERR)
      Serial.println("[SHT40] Temp mode error");
    else
      Serial.printf("[SHT40] Temp: %.2f °C\n", temperature);

    if (humidity == MODE_ERR)
      Serial.println("[SHT40] Humidity mode error");
    else
      Serial.printf("[SHT40] Humidity: %.2f %%RH\n", humidity);

    // Auto‐heater if very humid
    if (humidity > 80)
      SHT40.enHeater(POWER_CONSUMPTION_H_HEATER_1S);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// --- Task 3: Ultrasonic water-level ---
void ultrasonicTask(void* pv) {
  while (1) {
    if (mySerial.available() >= 4) {
      uint8_t d[4];
      for (int i = 0; i < 4; i++) d[i] = mySerial.read();
      if (d[0] == 0xFF && (((d[0]+d[1]+d[2]) & 0xFF) == d[3])) {
        int dist = (d[1] << 8) | d[2];
        if (dist > 30)
          Serial.printf("[Ultrasonic] %.1f cm\n", dist / 10.0);
        else
          Serial.println("[Ultrasonic] Below lower limit");
      } else {
        Serial.println("[Ultrasonic] Checksum/Frame error");
      }
    }
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

// --- Task 4: NPK readings ---
void npkTask(void* pv) {
  while (1) {
    Serial.println("\n[NPK] Reading values...");
    uint16_t N = read_npk(nitroCmd);
    uint16_t P = read_npk(phosCmd);
    uint16_t K = read_npk(potaCmd);

    if (N != 0xFFFF) Serial.printf("[NPK] Nitrogen: %u mg/kg\n", N);
    if (P != 0xFFFF) Serial.printf("[NPK] Phosphorous: %u mg/kg\n", P);
    if (K != 0xFFFF) Serial.printf("[NPK] Potassium: %u mg/kg\n", K);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void setup() {
  Serial.begin(115200);

  // --- Init sensors & comms ---
  mySerial.begin(9600, SERIAL_8N1, A02YYUW_RX, A02YYUW_TX);
  SHT40.begin();
  uint32_t id = SHT40.getDeviceID();
  Serial.printf("SHT40 ID: 0x%08X\n", id);

  // NPK RS-485 pins
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  digitalWrite(RE, LOW);
  digitalWrite(DE, LOW);
  mod.begin(4800);

  // --- Create FreeRTOS tasks ---
  xTaskCreate(moistureTask,    "Moisture", 2048, NULL, 1, NULL);
  xTaskCreate(shtTask,         "SHT40",    4096, NULL, 1, NULL);
  xTaskCreate(ultrasonicTask,  "Ultra",    4096, NULL, 1, NULL);
  xTaskCreate(npkTask,         "NPK",      4096, NULL, 1, NULL);
}

void loop() {
  // Nothing here—everything runs in tasks!
}
