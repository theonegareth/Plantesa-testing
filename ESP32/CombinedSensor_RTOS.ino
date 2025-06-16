#include "DFRobot_SHT40.h"
#include <Arduino.h>
#include <Wire.h>

// --- Pin definitions ---
// A02YYUW ultrasonic sensor on UART2
#define A02YYUW_TX 17  
#define A02YYUW_RX 16  

// Soil moisture sensor
const int AirValue = 300;    
const int WaterValue = 0;    
const int SensorPin = 15;

// NPK sensor (Modbus-RTU over RS-485 auto-direction)
// Use Serial2 (UART2) mapped to GPIO4 (RX) and GPIO5 (TX)
HardwareSerial modbusSerial(2);

// -- Modbus commands for N, P, K (each 8 bytes) --
const byte nitroCmd[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phosCmd[]  = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte potaCmd[]  = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// Create UART2 for ultrasonic
HardwareSerial ultraSerial(2);

// SHT40
DFRobot_SHT40 SHT40(SHT40_AD1B_IIC_ADDR);

// Shared variables
float temperature = 0.0, humidity = 0.0;
int soilMoisturePercent = 0;

// --- Utility to send a Modbus query and parse a 16-bit response ---
uint16_t read_npk(const byte* command) {
  modbusSerial.flush();           // Clear any outgoing data
  modbusSerial.write(command, 8); // Send Modbus command
  delay(10);                      // Short wait for response

  unsigned long start = millis();
  while (modbusSerial.available() < 7) {
    if (millis() - start > 2000) {
      Serial.println("[NPK] Timeout! No data received.");
      return 0xFFFF;
    }
    delay(10);
  }

  byte response[7];
  for (int i = 0; i < 7; i++) response[i] = modbusSerial.read();

  // Debug print response bytes
  Serial.print("[NPK] Response: ");
  for (int i = 0; i < 7; i++) {
    Serial.printf("%02X ", response[i]);
  }
  Serial.println();

  // Combine high and low bytes
  uint16_t value = (response[3] << 8) | response[4];
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

    if (humidity > 80)
      SHT40.enHeater(POWER_CONSUMPTION_H_HEATER_1S);

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// --- Task 3: Ultrasonic water-level ---
void ultrasonicTask(void* pv) {
  while (1) {
    if (ultraSerial.available() >= 4) {
      uint8_t d[4];
      for (int i = 0; i < 4; i++) d[i] = ultraSerial.read();
      if (d[0] == 0xFF && (((d[0] + d[1] + d[2]) & 0xFF) == d[3])) {
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
  ultraSerial.begin(9600, SERIAL_8N1, A02YYUW_RX, A02YYUW_TX);
  SHT40.begin();
  uint32_t id = SHT40.getDeviceID();
  Serial.printf("SHT40 ID: 0x%08X\n", id);

  // NPK RS-485 (auto-direction)
  modbusSerial.begin(4800, SERIAL_8N1, 4, 5);

  // --- Create FreeRTOS tasks ---
  xTaskCreate(moistureTask,    "Moisture", 2048, NULL, 1, NULL);
  xTaskCreate(shtTask,         "SHT40",    4096, NULL, 1, NULL);
  xTaskCreate(ultrasonicTask,  "Ultra",    4096, NULL, 1, NULL);
  xTaskCreate(npkTask,         "NPK",      4096, NULL, 1, NULL);
}

void loop() {
  // Nothing here—everything runs in tasks!
}
