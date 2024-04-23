#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_LTR329_LTR303.h>
#include <AccelStepper.h>
#include <ArduinoBLE.h>
#include "Arduino_LED_Matrix.h"

AccelStepper stepper1(1, 2, 3);// direction Digital 2 (CCW), pulses Digital 3 (CLK)
AccelStepper stepper2(1, 4, 5);// direction Digital 4 (CCW), pulses Digital 5 (CLK)


Adafruit_TMP117 tmp117;
Adafruit_LTR303 ltr;
ArduinoLEDMatrix matrix;
int test = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // wait for serial port to open
  }
  matrix.begin();

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

  // 32000 level = 3100 lumen direct light
  // 12000 level = 70000 lux
  // 15000 level = about 50 lumens direct light from iphone
  // 5000 level = 50 lumen flash light about half inch away
  // 32 level = ambient light in corner of room


   // put your setup code here, to run once:
    stepper1.setMaxSpeed(5000); //SPEED = Steps / second
    stepper1.setAcceleration(3600); //ACCELERATION = Steps /(second)^2
  
    stepper2.setMaxSpeed(5000); //SPEED = Steps / second
    stepper2.setAcceleration(3600); //ACCELERATION = Steps /(second)^2
/*
  stepper1.runToNewPosition(-63000); // blackout
  stepper2.runToNewPosition(-85000); // permeable

  stepper1.runToNewPosition(0);
  stepper2.runToNewPosition(0);
  */
  
}

const uint32_t happy[] = {
    0x19819,
    0x80000001,
    0x81f8000
};
const uint32_t heart[] = {
    0x3184a444,
    0x44042081,
    0x100a0040
};


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
    ltr.readBothChannels(visible_plus_ir, infrared);
    
      Serial.print("Visible + IR: ");
      Serial.println(visible_plus_ir);
  }
  
  
  
  if (test == 0) {
  controlBlinds(temperatureFahrenheit, visible_plus_ir);
  test++;
  Serial.println("Control blinds running");
  } else if (test == 3) {
    test = 0;
  } else {
    test++;
  } 
  

  delay(2000);
  
  matrix.loadFrame(happy);
  delay(500);

  matrix.loadFrame(heart);
  delay(500);

}

    int blackout_position;
    int permeable_position;

void controlBlinds(int temperatureFahrenheit, int visible_plus_ir) {
    
    // bool alreadyRun = false;
    Serial.println(blackout_position);
    Serial.println(permeable_position);

    if (visible_plus_ir >= 5000) {
      blackout_position = 0; // Lower blackout to home
      Serial.println("Simulate Lower black - light");
    } else if (visible_plus_ir >= 1000 && visible_plus_ir <= 4999) {
      permeable_position = 0; // Lower permeable to home
      Serial.println("Simulate Lower perm - light");
    } else if (visible_plus_ir <= 5) {
      blackout_position = -60000; // Raise blackout to up position
      permeable_position = -80000;
    } else if (temperatureFahrenheit >= 76 && temperatureFahrenheit <= 79) {
      permeable_position = 0; // Lower permeable to home
      Serial.println("Lower perm");
      // alreadyRun = true;
    } else if (temperatureFahrenheit >= 80 && temperatureFahrenheit <= 100) {
      blackout_position = 0; // Lower blackout to home
      Serial.println("Lower Blackout");
      // alreadyRun = true;
    } else if (temperatureFahrenheit >= 1 && temperatureFahrenheit <= 75) {
      blackout_position = -60000; // raise
      permeable_position = -80000;
      Serial.println("Simulate raise perm - temp");
      // alreadyRun = true;
    } 
    

    // Execute commands for controlling blinds based on positions
        stepper1.runToNewPosition(blackout_position);
        stepper2.runToNewPosition(permeable_position);
}