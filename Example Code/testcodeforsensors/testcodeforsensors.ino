#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_LTR329_LTR303.h>
#include <AccelStepper.h>

AccelStepper stepper1(1, 2, 3);// direction Digital 2 (CCW), pulses Digital 3 (CLK)
AccelStepper stepper2(1, 4, 5);// direction Digital 4 (CCW), pulses Digital 5 (CLK)


Adafruit_TMP117 tmp117;
Adafruit_LTR303 ltr;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // wait for serial port to open
  }

  // Initialize TMP117
  if (!tmp117.begin()) {
    Serial.println("Failed to find TMP117 chip");
    while (1) delay(10);
  }
  Serial.println("TMP117 Found!");

  // Initialize LTR303
  if (!ltr.begin()) {
    Serial.println("Couldn't find LTR sensor!");
    while (1) delay(10);
  }
  Serial.println("Found LTR sensor!");

  // Set TMP117 resolution and other configurations if needed
  // Set gain of 1 (see advanced demo for all options!
  ltr.setGain(LTR3XX_GAIN_1);
  // Set integration time of 50ms (see advanced demo for all options!
  ltr.setIntegrationTime(LTR3XX_INTEGTIME_50);
  // Set measurement rate of 50ms (see advanced demo for all options!
  ltr.setMeasurementRate(LTR3XX_MEASRATE_50);

   // put your setup code here, to run once:
    stepper1.setMaxSpeed(1600); //SPEED = Steps / second
    stepper1.setAcceleration(800); //ACCELERATION = Steps /(second)^2
  
    stepper2.setMaxSpeed(1600); //SPEED = Steps / second
    stepper2.setAcceleration(800); //ACCELERATION = Steps /(second)^2
}

void loop() {
  // TMP117 temperature measurement
  sensors_event_t temp;
  tmp117.getEvent(&temp);

  // Convert Celsius to Fahrenheit
  float temperatureFahrenheit = (temp.temperature * 9 / 5) + 32;

  Serial.print("Temperature: ");
  Serial.print(temperatureFahrenheit);
  Serial.println(" degrees F");

  // LTR303 light sensor measurement
  bool valid;
  uint16_t visible_plus_ir, infrared;

  if (ltr.newDataAvailable()) {
    valid = ltr.readBothChannels(visible_plus_ir, infrared);
    if (valid) {
      Serial.print("Visible + IR: ");
      Serial.println(visible_plus_ir);
    }
  }
  

  // code to simulate lower and raise
  if (temperatureFahrenheit > 85 || visible_plus_ir > 100) {
    Serial.println("Simulate lower");
    stepper1.runToNewPosition(12800);
  } else {
    Serial.println("Simulate raise");
    stepper1.runToNewPosition(0);
  }

  if (temperatureFahrenheit > 77 || visible_plus_ir > 50) {
    Serial.println("Simulate lower");
    stepper2.runToNewPosition(12800);
  } else {
    Serial.println("Simulate raise");
    stepper2.runToNewPosition(0);
  }
  delay(500);

  delay(1000); // Adjust delay as needed

}