const int AirValue = 300;   // Replace with your dry soil (air) calibration value
const int WaterValue = 0; // Replace with your wet soil (water) calibration value
const int SensorPin = 15;

int soilMoistureValue = 0;
int soilMoisturePercent = 0;

void setup() {
  Serial.begin(115200); // Start the Serial Monitor at 115200 baud
}

void loop() {
  soilMoistureValue = analogRead(SensorPin);  // Read from sensor
  soilMoisturePercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);

  // Clamp between 0 and 100
  soilMoisturePercent = constrain(soilMoisturePercent, 0, 100);

  Serial.print("Raw Value: ");
  Serial.print(soilMoistureValue);
  Serial.print(" | Soil Moisture: ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");

  delay(500); // Delay between readings
}
