/* 1. create website to change state - Done
 * 2. read in state change through API - Done
 * 3. open and close window - Done
 * 4. add inside temperature to the website 
 *    - import - done
 *    - initialize - done
 *    - find data - done
 *    - report - done
 * 5. get outside temperature ever ten minutes
 *    - variables - done
 *    - get request - done
 *    - retport - done
 */
 
#include <ArduinoJson.h>
#include <DHT.h>;
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define dirPin 13     //stepper motor direction pin
#define stepPin 12    //pin to indicate step on motor
#define ENAPin 11     //stepper motor ative pin
#define LIMIT 5       //limit switch pin connected to normally closed # true = high = opened, false = low = closed
#define DHTPin 4     // tempsensor pin (inside house)
#define DHTType DHT22   // DHT 22  (AM2302)

DHT dht(DHTPin, DHTType); // Initialize DHT sensor
ESP8266WebServer server(80);

const char* ssid     = "ssid";
const char* password = "password";
const String APIKEY = "Open Weather API Key";                                 
const String ZIPCode = "ZIP";   //ZIPCode
float outTemp;
float humidity;
float tempMin;
float tempMax;
String servername="api.openweathermap.org";              // remote server program will connect to
String result;
unsigned long oldTime = millis() - 60000;

//############################[html code]######################################

const char* html = "<!DOCTYPE html>\
<html>\
<head>\
    <script>\
        function sendOpen() {\
            var xhr = new XMLHttpRequest();\
            xhr.open('POST', 'http://192.168.1.18/change', true);\
            xhr.setRequestHeader('Content-Type', 'application/json');\
            xhr.send('open')\
        }\
        function sendClose() {\
            var xhr = new XMLHttpRequest();\
            xhr.open('POST', 'http://192.168.1.18/change', true);\
            xhr.setRequestHeader('Content-Type', 'application/json');\
            xhr.send('close')\
        }\
    </script>\
</head>\
<body>\
    <h1> ESP window </h1>\
    <p>Window is: %s</p>\
    <p id='data'>Indoor Temp: %f&#8457, Outdoor Temp: %f&#8457, High Temp: %f&#8457, Low Temp: %f&#8457</p>\
    <button type='button' onClick=sendOpen()>Open Window</button>\
    <button type='button' onClick=sendClose()>Close Window</button>\
</body>\
</html>";

//###########################[end of html]##################################

void setup() {
  Serial.begin(115200);
  dht.begin();

  pinMode(LIMIT, INPUT_PULLUP);

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(ENAPin, OUTPUT);

  digitalWrite(dirPin, HIGH);
  digitalWrite(ENAPin, HIGH);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/change", handleForm);
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  if(millis() > oldTime + 600000) {
    getWeatherData();
    oldTime = millis();
  }
  server.handleClient();
}

//&#8457 is html-code for degree farenheit
void handleRoot() {
  int temp = dht.readTemperature(true);
  char buff[840];
  char* state;
  if(digitalRead(LIMIT)) {
    state = "opened";
  }
  else {
    state = "closed";
  }
  sprintf(buff, html, state, temp, outTemp, tempMax, tempMin);
  
  server.sendContent(buff);
}

void handleForm() {
  String state = "";
  if (server.args()) {
    state = server.arg(0);
  }
  else {
    server.send(400);
    return;
  }
  if(state.indexOf("close") >= 0 && digitalRead(LIMIT)) {
    closeWindow();
  }
  else if(state.indexOf("open") >= 0 && !digitalRead(LIMIT)) {
    openWindow();
  }
  else {
    server.send(400, "text/plain", "can't change");  //attempt to open or close when already in that state
    return;
  }
  server.send(200, "text/plain", "state changed");
  Serial.println(state);
}

//starts with a slow acceleration of steppermotor to prevent steploss and high forces
void openWindow() {
  Serial.println("Opening Window");
  digitalWrite(ENAPin, LOW);
  digitalWrite(dirPin, HIGH);
  for (int i = 0; i < 400; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800 - (i * 0.75));
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800 - (i * 0.75));
  }
  for (int i = 0; i < 2000; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500 - (i * 0.15625));
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500 - (i * 0.15625));
  }
  //7400
  for (int i = 0; i < 7400; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(200);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(200);
  }
  digitalWrite(ENAPin, HIGH);
}

//closes window using limit switch to reduce grinding and user inaccuracy
void closeWindow() {
  Serial.println("Closing Window");
  digitalWrite(ENAPin, LOW);
  digitalWrite(dirPin, LOW); //set direction as close
  //begin accelleration of window while checking limit switch
  for (int i = 0; i < 400 && digitalRead(LIMIT) == HIGH; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(800 - (i * 0.75));
    digitalWrite(stepPin, LOW);
    delayMicroseconds(800 - (i * 0.75));
  }
  for (int i = 0; i < 2000 && digitalRead(LIMIT) == HIGH; i++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500 - (i * 0.15625));
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500 - (i * 0.15625));
  }
  //while checking limit switch move to high speed
  while(digitalRead(LIMIT) == HIGH) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(200);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(200);
  }
  digitalWrite(ENAPin, HIGH);
}

void getWeatherData() {
  WiFiClient client;
  if(client.connect(servername, 80)) {
    client.println("GET /data/2.5/weather?zip="+ZIPCode+"&units=Imperial&APPID="+APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  }
  else {
    Serial.println("There was an error with getting data from open weather");
    return;
  }

  while(client.connected() && !client.available()) {
    delay(10);
  }
  while(client.connected() || client.available()) {
    char c = client.read();
    result += c;
  }
  client.stop();
  result.replace('[', ' ');
  result.replace(']', ' ');
  char jsonArray [result.length()+1];
  result.toCharArray(jsonArray,sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  StaticJsonDocument<1024> json_buf;\
  deserializeJson(json_buf, jsonArray);
  JsonObject root = json_buf.as<JsonObject>();
  outTemp = root["main"]["temp"];
  humidity = root["main"]["humidity"];
  tempMax = root["main"]["temp_max"];
  tempMin = root["main"]["temp_min"];
  result = "";
}
