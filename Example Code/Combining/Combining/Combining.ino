#include <Wire.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_LTR329_LTR303.h>
#include <AccelStepper.h>
#include "Arduino_LED_Matrix.h"

#include <WiFiS3.h>

#include "arduino_secrets.h"

//WIFI Connection
const char ssid[] = SECRET_SSID; // change your network SSID (name)
const char pass[] = SECRET_PASS; // change your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Light and Temp Control</title><style>body{background-color:#1e3799;color:#fff;font-family:Arial,sans-serif;text-align:center}.row{display:flex;justify-content:center;margin-bottom:10px}.title{margin-bottom:5px;margin-top:5px;color:#f5f6fa;font-size:large}.content{color:#f5f6fa;font-size:medium}.c_light{margin-left:2.5rem}.c_temp{margin-right:2.5rem}.button{background-color:#fff;color:#000;padding:10px 20px;border:none;cursor:pointer;width:5rem;margin:5px}.combo{width:10rem;text-align:center}</style></head><body><div class="row"><label class="title">CURRENT</label></div><div class="row"><div class="content c_temp" id="currentTemp">80 F</div><div class="content c_light" id="currentLight">50 %</div></div><div class="row"><label class="title">ACTIVATION</label></div><div class="row"><div class="content c_temp" id="activationTemp">70 F</div><div class="content c_light" id="activationLight">50 %</div></div><div class="row"><button class="button" onclick="raiseTemp()">RAISE</button><button class="button" onclick="raiseLight()">RAISE</button></div><div class="row"><button class="button" onclick="lowerTemp()">LOWER</button><button class="button" onclick="lowerLight()">LOWER</button></div><div class="row"><select class="combo" id="modeSelect" onchange="changeMode()"><option value="0">Auto</option><option value="1">Up</option><option value="2">Semi</option><option value="3">Blackout</option></select></div><script>let temperatureValue=75,lightValue=50,isRequestInProgress=!1;function raiseTemp(){temperatureValue++,document.getElementById("activationTemp").innerText=temperatureValue+" F",sendPostRequest("/temp",{temp:temperatureValue})}function raiseLight(){lightValue++,document.getElementById("activationLight").innerText=lightValue+" %",sendPostRequest("/light",{light:lightValue})}function lowerTemp(){temperatureValue--,document.getElementById("activationTemp").innerText=temperatureValue+" F",sendPostRequest("/temp",{temp:temperatureValue})}function lowerLight(){lightValue--,lightValue<0&&(lightValue=0),document.getElementById("activationLight").innerText=lightValue+" %",sendPostRequest("/light",{light:lightValue})}function changeMode(){sendPostRequest("/mode",{mode:document.getElementById("modeSelect").value})}function sendPostRequest(e,t){isRequestInProgress||(isRequestInProgress=!0,console.log("Sending POST request to "+e+" with data:",t),fetch(e,{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify(t)}).then((e=>{if(!e.ok)throw new Error("Network response was not ok");return e.json()})).then((e=>{console.log("Response:",e)})).catch((e=>{console.error("There was a problem with the fetch operation:",e)})).finally((()=>{isRequestInProgress=!1})))}function updateCurrentData(){isRequestInProgress||(isRequestInProgress=!0,fetch("/data").then((e=>{if(!e.ok)throw new Error("An error occurred while obtaining the data.");return e.json()})).then((e=>{document.getElementById("currentTemp").textContent=e.temperature,document.getElementById("currentLight").textContent=e.light})).catch((e=>{console.error("Error:",e)})).finally((()=>{isRequestInProgress=!1})))}document.addEventListener("DOMContentLoaded",(function(e){updateCurrentData()})),setInterval(updateCurrentData,5e3);</script></body></html>)rawliteral";

WiFiServer server(80);

int sensorTemp, sensorLight;
int userTemp = 70;
int userLight;
uint8_t userMode;
bool modechange = false;
bool autoMode = true;


//Stepper & Sensor Controls
AccelStepper stepper1(1, 2, 3);// direction Digital 2 (CCW), pulses Digital 3 (CLK), blackout
AccelStepper stepper2(1, 4, 5);// direction Digital 4 (CCW), pulses Digital 5 (CLK), permiable

Adafruit_TMP117 tmp117;
Adafruit_LTR303 ltr;
ArduinoLEDMatrix matrix;

//Blind Positions
int home_Position = 0;
int perm_Raise = -80000;
int BO_Raise = -60000;

int blackout_position;
int permeable_position;
int temperatureFahrenheit;
uint16_t visible_plus_ir, infrared;
int test = 0;



void setup() {
    stepperSetup();
    wifiSetup();
}

//Fun heart display stuff
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
    stepperLoop();
    wifiLoop();

    Serial.print("userTemp: ");
    Serial.println(userTemp);

    overrideCheck();
    //controlBlinds(temperatureFahrenheit, visible_plus_ir);
}

void overrideCheck() {
  //Serial.println("overrideCheck");
  if (modechange){
    Serial.println("ModeChange Detected");
    Serial.print("Mode: ");
    if (userMode == 0) {
        Serial.println("AUTO");
        controlBlinds(temperatureFahrenheit, visible_plus_ir);
        autoMode = true;        
      } else if (userMode == 1) {
        Serial.println("UP");
        changeBlindPosition(BO_Raise, perm_Raise);
        autoMode = false;
      } else if (userMode == 2) {
        Serial.println("SEMI");
        changeBlindPosition(BO_Raise, home_Position);
        autoMode = false;
      } else if (userMode == 3) {
        Serial.println("BLACKOUT");
        changeBlindPosition(home_Position, home_Position);
        autoMode = false;
      } 
      modechange = false;
  }else if(autoMode) {
    //Serial.println("autoMode Detected");
  controlBlinds(temperatureFahrenheit, visible_plus_ir);
  }

}

void stepperSetup() {
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

void stepperLoop() {

  temperatureFahrenheit = sensorTemperatureData();
  visible_plus_ir = sensorLightData();

  delay(2000);
  
  matrix.loadFrame(happy);
  delay(500);

  matrix.loadFrame(heart);
  delay(500);
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

int sensorTemperatureData() {
    // TMP117 temperature measurement
  sensors_event_t temp;
  tmp117.getEvent(&temp);

  // Convert Celsius to Fahrenheit
  int temperatureFahrenheit = (temp.temperature * 9 / 5) + 32;

  Serial.print("Temperature: ");
  Serial.print(temperatureFahrenheit);
  Serial.println(" degrees F");

 return temperatureFahrenheit;
}

uint16_t sensorLightData() {
    if (ltr.newDataAvailable()) {
      ltr.readBothChannels(visible_plus_ir, infrared);
    
      Serial.print("Visible + IR: ");
      Serial.println(visible_plus_ir);
    
  }
  Serial.println(visible_plus_ir);
  return visible_plus_ir;
}


void controlBlinds(int temperatureFahrenheit, int visible_plus_ir) {
    //Need to incorperate into switch statements w/o breaking things.

    int desiredIR = userLight * 10000; // convert userLight to lumens for comparison. 10000 = 100%


    if (visible_plus_ir >= 5000) {
      blackout_position = home_Position; // Lower blackout to home
      permeable_position = home_Position; 
      Serial.println("Simulate Lower black - light");
    } else if (visible_plus_ir >= 1000 && visible_plus_ir <= 4999) { // 
      permeable_position = home_Position; // Lower permeable to home
      blackout_position = BO_Raise;
      Serial.println("Simulate Lower perm - light");
    } else if (visible_plus_ir <= 5) {
      blackout_position = BO_Raise; // Raise blackout to up position
      permeable_position = perm_Raise;
    } else if (temperatureFahrenheit >= (userTemp - 5) && temperatureFahrenheit < userTemp) {
      blackout_position = BO_Raise;
      permeable_position = home_Position; // Lower permeable to home
      Serial.println("Lower perm");
    } else if (temperatureFahrenheit >= userTemp && temperatureFahrenheit <= 110) {
      blackout_position = home_Position; // Lower blackout to home
      permeable_position = home_Position;
      Serial.println("Lower Blackout");
    
    } else if (temperatureFahrenheit >= 1 && temperatureFahrenheit < (userTemp - 5)) {
      blackout_position = BO_Raise; // raise
      permeable_position = perm_Raise;
      Serial.println("Simulate raise perm - temp");
    } 

    // Execute commands for controlling blinds based on positions

    changeBlindPosition(blackout_position, permeable_position);
}

void changeBlindPosition(int blackout_position, int permeable_position) {
    //remove if statements. Just go to new position
        stepper1.runToNewPosition(blackout_position);
        stepper2.runToNewPosition(permeable_position);
}

//WIFI Code
void parseData(String jsonString)
{ 
  if(jsonString.indexOf("temp") != -1)
  {// is something like {"temp": 75}
    int posTemp = jsonString.indexOf("temp") + 6;       
    String tempString = jsonString.substring(posTemp);
    int commaTemp = tempString.indexOf(",");
    tempString = tempString.substring(0, commaTemp);

    Serial.print("tempString: ");
    Serial.print(tempString);
    Serial.print("\n");
    userTemp = tempString.toInt(); // received temp value

    Serial.print("userTemp: ");
    Serial.println(userTemp);
  }

  else if(jsonString.indexOf("light") != -1)
  {// is something like {"light": 50}
    int posLight = jsonString.indexOf("light") + 7;
    String lightString = jsonString.substring(posLight);
    int commaLight = lightString.indexOf(",");
    lightString = lightString.substring(0, commaLight);
    
    userLight = lightString.toInt(); // received light value

    //good spot for sensor data

    //FOr light comparison 100% will be 10,000 lumen

    Serial.print("Light: ");
    Serial.println(userLight);
  }

  else if(jsonString.indexOf("mode") != -1)
  {// is something like {"mode": 0}
    int posMode = jsonString.indexOf("mode") + 7;
    String modeString = jsonString.substring(posMode);
    int commaMode = modeString.indexOf(",");
    modeString = modeString.substring(0, commaMode);
    
    userMode = modeString.toInt(); // received mode value
    modechange = true;
  } 
}

//Display Sensor Data
String getData()
{
  String s_temp = String(temperatureFahrenheit);
  String s_light = String(visible_plus_ir);
  return String("{\"temperature\": ") + s_temp + String(", \"light\": ") + s_light + String("}");
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