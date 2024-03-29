#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_LTR329_LTR303.h>
#include <ArduinoBLE.h>
#include <AccelStepper.h>

AccelStepper stepper1(1, 2, 3);// direction Digital 2 (CCW), pulses Digital 3 (CLK)
AccelStepper stepper2(1, 4, 5);// direction Digital 4 (CCW), pulses Digital 5 (CLK)

Adafruit_TMP117 tmp117;
Adafruit_LTR303 ltr;

BLEService sensorService("180A");
BLEStringCharacteristic temperatureCharacteristic("2A6E", BLERead | BLENotify, 20); // Set the maximum length of the string
BLEStringCharacteristic lightCharacteristic("2A76", BLERead | BLENotify, 20); // Set the maximum length of the string

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // wait for serial port to open
  }

  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1);
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

  // Start BLE
  BLE.setLocalName("SensorDevice");
  BLE.setAdvertisedService(sensorService);
  sensorService.addCharacteristic(temperatureCharacteristic);
  sensorService.addCharacteristic(lightCharacteristic);
  BLE.addService(sensorService);
  temperatureCharacteristic.writeValue("0");
  lightCharacteristic.writeValue("0");
  BLE.advertise();

  Serial.println("BLE device active, waiting for connections...");

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

  // Update BLE characteristic
  temperatureCharacteristic.writeValue(String(temperatureFahrenheit));

  // LTR303 light sensor measurement
  bool valid;
  uint16_t visible_plus_ir, infrared;

  if (ltr.newDataAvailable()) {
    valid = ltr.readBothChannels(visible_plus_ir, infrared);
    if (valid) {
      // Update BLE characteristic
      lightCharacteristic.writeValue(String(visible_plus_ir));
    }
  }

  // code to simulate lower and raise
  if (temperatureFahrenheit > 80 || visible_plus_ir > 100) {
    Serial.println("Simulate lower");
  } else {
    Serial.println("Simulate raise");
  }
  delay(500);

  delay(1000); // Adjust delay as needed
}
