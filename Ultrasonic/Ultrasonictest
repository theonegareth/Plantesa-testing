// Define ESP32 hardware serial port for the A02YYUW sensor
#define A02YYUW_TX 5  // ESP32 TX connected to Sensor RX
#define A02YYUW_RX 18  // ESP32 RX connected to Sensor TX

// Initialize hardware serial for A02YYUW
HardwareSerial mySerial(2);

void setup() {
    Serial.begin(115200);  // Initialize Serial Monitor
    mySerial.begin(9600, SERIAL_8N1, A02YYUW_RX, A02YYUW_TX);  // Initialize UART2
    Serial.println("A02YYUW Distance Sensor Example");
}

void loop() {
    if (mySerial.available() >= 4) {
        uint8_t data[4];
        for (int i = 0; i < 4; i++) {
            data[i] = mySerial.read();
        }

        if (data[0] == 0xFF) {  // Check packet start byte
            int sum = (data[0] + data[1] + data[2]) & 0xFF;
            if (sum == data[3]) {  // Validate checksum
                int distance = (data[1] << 8) + data[2];  // Calculate distance
                if (distance > 30) {  // Ensure valid reading
                    Serial.print("Water Level Distance: ");
                    Serial.print(distance / 10.0);
                    Serial.println(" cm");
                } else {
                    Serial.println("Below the lower limit");
                }
            } else {
                Serial.println("Checksum error");
            }
        }
    }
    delay(100);
}