/***************************************************
  This is a simulated example for the LTR303 light sensor that
  gets you started fast by setting the default gain/rates
  and only displays random simulated values for visible light and infrared

  Designed specifically to work with the LTR-303 light sensor from Adafruit
  ----> https://www.adafruit.com/product/5610

  These sensors use I2C to communicate, 2 pins are required to
  interface
 ****************************************************/

#include "Adafruit_LTR329_LTR303.h"

Adafruit_LTR303 ltr = Adafruit_LTR303();

void setup() {
  Serial.begin(115200);
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
  // Simulate values for visible plus infrared and infrared channels
  uint16_t simulatedVisibleIR = random(0, 65535);
  uint16_t simulatedInfrared = random(0, 65535);

  Serial.print("Simulated CH0 Visible + IR: ");
  Serial.print(simulatedVisibleIR);
  Serial.print("\t\tCH1 Infrared: ");
  Serial.println(simulatedInfrared);

  delay(1000);
}
