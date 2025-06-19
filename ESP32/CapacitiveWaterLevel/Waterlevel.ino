// Sensor signal pin
#define sensorPin 2

const int maxValue = 1450;  // Cap value for 100%
int val = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  int rawLevel = readSensor();

  // Cap at maxValue
  if (rawLevel > maxValue) {
    rawLevel = maxValue;
  }

  // Convert to percentage
  int percentLevel = (rawLevel * 100) / maxValue;

  // Print water level percentage
  Serial.print("Water level: ");
  Serial.print(percentLevel);
  Serial.println("%");

  // Warning when full
  if (percentLevel >= 100) {
    Serial.println("⚠️ WARNING: Water level FULL!");
  }

  delay(1000);
}

int readSensor() {
  val = analogRead(sensorPin);  // Read analog value directly
  return val;
}
