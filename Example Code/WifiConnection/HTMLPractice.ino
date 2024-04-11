/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server
 */

#include <WiFiS3.h>
#include "arduino_secrets.h"

const char ssid[] = SECRET_SSID;  // change your network SSID (name)
const char pass[] = SECRET_PASS;   // change your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

float getTemperature() {
  //return 26.9456;
  // YOUR SENSOR IMPLEMENTATION HERE
  // simulate the temperature value
  float temp_x100 = random(0, 10000);  // a ramdom value from 0 to 10000
  return temp_x100 / 100;              // return the simulated temperature value from 0 to 100 in float
}


float getLight() {
  float light_x100 = random(0, 10000);
  return light_x100 / 100;
}


void setup() {
  localWebSetup();
}


void loop() {
  localWebService();
}

void localWebSetup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    Serial.println("Please upgrade the firmware");

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  server.begin();
  // you're connected now, so print out the status:
  Serial.println("Connected");
  printWifiStatus();  

}

void localWebService(){
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    // read the HTTP request header line by line
    while (client.connected()) {
      if (client.available()) {
        String HTTP_header = client.readStringUntil('\n');  // read the header line of HTTP request

        if (HTTP_header.equals("\r"))  // the end of HTTP request
          break;

        Serial.print("<< ");
        Serial.println(HTTP_header);  // print HTTP request to Serial Monitor
      }
    }

    // send the HTTP response
    // send the HTTP response header
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");  // the connection will be closed after completion of the response
    client.println();                     // the separator between HTTP header and body
    // send the HTTP response body
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head>");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    client.println("</head>");

    client.println("<p>");

    client.print("Current Temperature: <span style=\"color: red;\">");
    float temperature = getTemperature();
    client.print(temperature, 2);
    client.println("&deg;C</span>");

    client.print("Current Light: <span style=\"color: blue;\">");
    float light = getLight();
    client.print(light, 2);
    client.println(" Lux</span>");

    client.print("<br>");
    client.print("Desired Temperature: <span style=\"color: red;\">");
    float desTemperature = getTemperature();
    client.print(desTemperature, 2);
    client.println("&deg;C</span>");

    client.print("Desired Light: <span style=\"color: blue;\">");
    float desLight = getLight();
    client.print(desLight, 2);
    client.println(" Lux</span>");

    client.println("</p>");
    client.println("</html>");
    client.flush();


    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
  }  
}

void printWifiStatus() {
  // print your board's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // print the received signal strength:
  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}
