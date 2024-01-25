/**
 * @file simulated_test.ino
 * @author Your Name
 * @brief Simulates TMP117 sensor data for testing
 * @date 2024-01-25
 */

#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>

Adafruit_TMP117  tmp117;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);     // will pause Zero, Leonardo, etc until serial console opens
  Serial.println("Simulated TMP117 Test!");
  
  // You can comment out the following line as it's not necessary for simulation
  // if (!tmp117.begin()) {
  //   Serial.println("Failed to find TMP117 chip");
  //   while (1) { delay(10); }
  // }
  Serial.println("Simulated TMP117 Found!");
}

void loop() {
  // Simulate temperature data
  float simulatedTemperature = random(200, 300) / 10.0; // Random temperature between 20.0 and 30.0 degrees Celsius

  // Print the simulated temperature
  Serial.print("Simulated Temperature  ");
  Serial.print(simulatedTemperature);
  Serial.println(" degrees C");
  Serial.println("");
  if (simulatedTemperature > 25) {
    Serial.println("Simulate raise");
  } else {
    Serial.println("Simulate lower");
  }

  delay(5000);
}
