#include <HardwareSerial.h>
#include <Wire.h>

// No RE/DE control needed — auto-direction module

// Modbus command packets
const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[]  = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[]  = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// Use Serial2 (UART2), mapped to GPIO4 (RX) and GPIO5 (TX)
HardwareSerial mod(2);

void setup() {
    Serial.begin(9600);              // For serial monitor
    mod.begin(4800, SERIAL_8N1, 4, 5); // RS485 on GPIO4 (RX), GPIO5 (TX)

    delay(500); // Short delay for initialization
}

uint16_t read_npk(const byte* command) {
    mod.flush();           // Clear any outgoing data
    mod.write(command, 8); // Send 8-byte Modbus command
    delay(10);             // Give some time for sensor to respond

    long timeout = millis();
    while (mod.available() < 7) {
        if (millis() - timeout > 2000) {
            Serial.println("Timeout! No data received.");
            return 65535;  // Return error value
        }
        delay(10);
    }

    byte response[7];
    for (byte i = 0; i < 7; i++) {
        response[i] = mod.read();
    }

    Serial.print("Response: ");
    for (byte i = 0; i < 7; i++) {
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Combine high and low bytes to get the value
    uint16_t value = (response[3] << 8) | response[4];
    return value;
}

void loop() {
    Serial.println("\nReading NPK values...");

    uint16_t nitrogen    = read_npk(nitro);
    uint16_t phosphorous = read_npk(phos);
    uint16_t potassium   = read_npk(pota);

    if (nitrogen != 65535) {
        Serial.print("Nitrogen: ");
        Serial.print(nitrogen);
        Serial.println(" mg/kg");
    }

    if (phosphorous != 65535) {
        Serial.print("Phosphorous: ");
        Serial.print(phosphorous);
        Serial.println(" mg/kg");
    }

    if (potassium != 65535) {
        Serial.print("Potassium: ");
        Serial.print(potassium);
        Serial.println(" mg/kg");
    }

    delay(2000); // Wait before next reading
}
