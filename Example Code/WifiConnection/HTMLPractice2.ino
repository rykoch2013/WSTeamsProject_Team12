/*
 * Created by ArduinoGetStarted.com
 *
 * This example code is in the public domain
 *
 * Tutorial page: https://arduinogetstarted.com/tutorials/arduino-web-server
 */

#include <WiFiS3.h>
#include "arduino_secrets.h"

const char ssid[] = SECRET_SSID; // change your network SSID (name)
const char pass[] = SECRET_PASS; // change your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Light and Temp Control</title><style>body{background-color:#1e3799;color:#fff;font-family:Arial,sans-serif;text-align:center}.row{display:flex;justify-content:center;margin-bottom:10px}.title{margin-bottom:5px;margin-top:5px;color:#f5f6fa;font-size:large}.content{color:#f5f6fa;font-size:medium}.c_light{margin-left:2.5rem}.c_temp{margin-right:2.5rem}.button{background-color:#fff;color:#000;padding:10px 20px;border:none;cursor:pointer;width:5rem;margin:5px}.combo{width:10rem;text-align:center}</style></head><body><div class="row"><label class="title">CURRENT</label></div><div class="row"><div class="content c_temp" id="currentTemp">-- F</div><div class="content c_light" id="currentLight">-- %</div></div><div class="row"><label class="title">ACTIVATION</label></div><div class="row"><div class="content c_temp" id="activationTemp">-- F</div><div class="content c_light" id="activationLight">-- %</div></div><div class="row"><button class="button" onclick="raiseTemp()">RAISE</button><button class="button" onclick="raiseLight()">RAISE</button></div><div class="row"><button class="button" onclick="lowerTemp()">LOWER</button><button class="button" onclick="lowerLight()">LOWER</button></div><div class="row"><select class="combo" id="modeSelect" onchange="changeMode()"><option value="0">Auto</option><option value="1">Up</option><option value="2">Semi</option><option value="3">Blackout</option></select></div><script>let temperatureValue=0,lightValue=0,isRequestInProgress=!1;function raiseTemp(){temperatureValue++,document.getElementById("activationTemp").innerText=temperatureValue+" F",sendPostRequest("/temp",{temp:temperatureValue})}function raiseLight(){lightValue++,document.getElementById("activationLight").innerText=lightValue+" %",sendPostRequest("/light",{light:lightValue})}function lowerTemp(){temperatureValue--,document.getElementById("activationTemp").innerText=temperatureValue+" F",sendPostRequest("/temp",{temp:temperatureValue})}function lowerLight(){lightValue--,lightValue<0&&(lightValue=0),document.getElementById("activationLight").innerText=lightValue+" %",sendPostRequest("/light",{light:lightValue})}function changeMode(){sendPostRequest("/mode",{mode:document.getElementById("modeSelect").value})}function sendPostRequest(e,t){isRequestInProgress||(isRequestInProgress=!0,console.log("Sending POST request to "+e+" with data:",t),fetch(e,{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(t)}).then((e=>{if(!e.ok)throw new Error("Network response was not ok");return e.json()})).then((e=>{console.log("Response:",e)})).catch((e=>{console.error("There was a problem with the fetch operation:",e)})).finally((()=>{isRequestInProgress=!1})))}function updateCurrentData(){isRequestInProgress||(isRequestInProgress=!0,fetch("/data").then((e=>{if(!e.ok)throw new Error("An error occurred while obtaining the data.");return e.json()})).then((e=>{document.getElementById("currentTemp").textContent=e.temperature,document.getElementById("currentLight").textContent=e.light})).catch((e=>{console.error("Error:",e)})).finally((()=>{isRequestInProgress=!1})))}document.addEventListener("DOMContentLoaded",(function(e){updateCurrentData()})),setInterval(updateCurrentData,5e3);</script></body></html>)rawliteral";

WiFiServer server(80);

//int f_light, f_temp;
uint8_t d_mode;

//Main function code, internal functions to operate Wifi & Webpage
void setup()
{
  wifiSetup();
}

void loop()
{
  wifiLoop();
}

void parseData(String jsonString)
{
  if(jsonString.indexOf("temp") != -1)
  {
    int posTemp = jsonString.indexOf("temp") + 6;
    String tempString = jsonString.substring(posTemp);
    int commaTemp = tempString.indexOf(",");
    tempString = tempString.substring(0, commaTemp);

    Serial.print("tempString: ");
    Serial.print(tempString);
    Serial.print("\n");
    float temperature = tempString.toInt();

    Serial.print("Temperature: ");
    Serial.println(temperature);
  }

  else if(jsonString.indexOf("light") != -1)
  {
    int posLight = jsonString.indexOf("light") + 7;
    String lightString = jsonString.substring(posLight);
    int commaLight = lightString.indexOf(",");
    lightString = lightString.substring(0, commaLight);
    
    float light = lightString.toInt();

    Serial.print("Light: ");
    Serial.println(light);
  }

  else if(jsonString.indexOf("mode") != -1)
  {
    int posMode = jsonString.indexOf("mode") + 7;
    String modeString = jsonString.substring(posMode);
    int commaMode = modeString.indexOf(",");
    modeString = modeString.substring(0, commaMode);
    
    uint8_t d_mode = modeString.toInt();

    Serial.print("Mode: ");
    switch(d_mode)
    {
      case 0:
      {
        Serial.println("AUTO");
      }
      break;
      case 1:
      {
        Serial.println("UP");
      }
      break;
      case 2:
      {
        Serial.println("SEMI");
      }
      break;
      case 3:
      {
        Serial.println("BLACKOUT");
      }
      break;
      default:
      break;
    }
  } 
}

//for sensor temp readings
float getTemperature()
{
  // return 26.9456;
  //  YOUR SENSOR IMPLEMENTATION HERE
  //  simulate the temperature value
  float temp_x100 = random(0, 10000); // a ramdom value from 0 to 10000
  return temp_x100 / 100;             // return the simulated temperature value from 0 to 100 in float
}

//for sensor Light readings
float getLight()
{
  float light_x100 = random(0, 10000);
  return light_x100 / 100;
}

String getData()
{
  float temp, light;
  temp = getTemperature();
  light = getLight();
  String s_temp = String(temp);
  String s_light = String(light);
  return String("{\"temperature\": ") + s_temp + String(", \"light\": ") + s_light + String("}");
}

void wifiSetup(){
  // Initialize serial and wait for port to open:
  Serial.begin(9600);

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    Serial.println("Please upgrade the firmware");

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED)
  {
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



void wifiLoop(){
// listen for incoming clients
  WiFiClient client = server.available();
  if (client)
  {
    String HTTP_req = "";
    while (client.connected())
    {
      if (client.available())
      {
        Serial.println("New HTTP Request");
        HTTP_req = client.readStringUntil('\n'); // read the first line of HTTP request
        Serial.println(HTTP_req); // print HTTP request to Serial Monitor
        break;
      }
    }
    if (HTTP_req.indexOf("POST") != -1)
    {
      String body = "";
      while (client.available())
      {
        char c = client.read();
        body += c;
      }
      Serial.println("Body data:");
      Serial.println(body);

      if (HTTP_req.indexOf("temp") > -1)
      { // check the path
        Serial.print(HTTP_req);
        Serial.println(" ");
        parseData(body);
      }
      else if (HTTP_req.indexOf("light") > -1)
      { // check the path
        Serial.print(HTTP_req);
        Serial.println(" ");
        parseData(body);
      }
      else if (HTTP_req.indexOf("mode") > -1)
      { // check the path
        Serial.print(HTTP_req);
        Serial.println(" ");
        parseData(body);
      }
      else
      {
        Serial.println("POST No command");
      }
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();
      client.println("{\"s\":\"0k\"}");
      delay(10);
      client.stop();
    }
    // read the HTTP request header line by line
    while (client.connected())
    {
      if (client.available())
      {
        String HTTP_header = client.readStringUntil('\n'); // read the header line of HTTP request

        if (HTTP_header.equals("\r")) // the end of HTTP request
          break;

//        Serial.print("<< ");
//        Serial.println(HTTP_header); // print HTTP request to Serial Monitor
      }
    }

    if (HTTP_req.indexOf("GET") == 0)
    { // check if request method is GET
      if (HTTP_req.indexOf("data") > -1)
      { // check the path
        Serial.println("GET data");

        String response = getData();
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Connection: close");
        client.println();
        client.println(response);
        delay(10);
        client.stop();
      }
      else
      {
        Serial.println("GET No command");

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println("Connection: close"); // the connection will be closed after completion of the response
        client.println();                    // the separator between HTTP header and body
        client.println(index_html);
        client.flush();
        delay(10);
        // close the connection:
        client.stop();
      }
    }
  }

}

void printWifiStatus()
{
  // print your board's IP address:
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // print the received signal strength:
  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}
