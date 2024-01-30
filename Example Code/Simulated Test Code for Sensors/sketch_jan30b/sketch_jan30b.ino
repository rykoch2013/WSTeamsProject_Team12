/**
 * @file simulated_test.ino
 * @author Your Name
 * @brief Simulates TMP117 sensor data for testing
 * @date 2024-01-25
 */

#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_LTR329_LTR303.h"

Adafruit_LTR303 ltr = Adafruit_LTR303();

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

  Serial.begin(9600);
  while (!Serial) {
    delay(10); // wait for serial port to open
  }
  Serial.println("Simulated Adafruit LTR-303 Test");

  // You can comment out the following line as it's not necessary for simulation
  // if (!ltr.begin()) {
  //   Serial.println("Couldn't find LTR sensor!");
  //   while (1) delay(10);
  // }
  Serial.println("Simulated LTR sensor found!");

  // Set gain of 1 (see advanced demo for all options!)
  ltr.setGain(LTR3XX_GAIN_1);
  // Set integration time of 50ms (see advanced demo for all options!)
  ltr.setIntegrationTime(LTR3XX_INTEGTIME_50);
  // Set measurement rate of 50ms (see advanced demo for all options!)
  ltr.setMeasurementRate(LTR3XX_MEASRATE_50);
}

void loop() {
  // Simulate temperature data
  float simulatedTemperature = random(200, 300) / 10.0; // Random temperature between 20.0 and 30.0 degrees Celsius

  // Print the simulated temperature
  Serial.print("Simulated Temperature  ");
  Serial.print(simulatedTemperature);
  Serial.println(" degrees C");
  Serial.println("");

  // Simulate values for visible plus infrared and infrared channels
  uint16_t simulatedVisibleIR = random(0, 65535);

  Serial.print("Simulated CH0 Visible + IR: ");
  Serial.println(simulatedVisibleIR);

  delay(1000);

  if (simulatedTemperature > 25 || simulatedVisibleIR > 30000) {
    Serial.println("Simulate raise");
  } else {
    Serial.println("Simulate lower");
  }
  delay(5000);

}
