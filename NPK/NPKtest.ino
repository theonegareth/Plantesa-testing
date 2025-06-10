#include <SoftwareSerial.h>
#include <Wire.h>

#define RE 8
#define DE 7

const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[]  = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[]  = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

SoftwareSerial mod(2, 3);  // RX, TX

void setup() {
    Serial.begin(9600);
    mod.begin(4800);

    pinMode(RE, OUTPUT);
    pinMode(DE, OUTPUT);

    digitalWrite(RE, LOW);
    digitalWrite(DE, LOW);

    delay(500);
}

uint16_t read_npk(const byte* command) {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);

    mod.write(command, 8);
    delay(10);

    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);

    long timeout = millis();
    while (mod.available() < 7) {
        if (millis() - timeout > 2000) {
            Serial.println("Timeout! No data received.");
            return 65535;  // Error indicator
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

    // Extract 16-bit value from response[3] and response[4]
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

    delay(2000);
}
