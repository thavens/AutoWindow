#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "RestClient.h"

const char* ssid = "SSID";
const char* password = "Password";
String APIKEY = "openweather API Key";                                 
String ZIPCode = "ZIP";   //ZIPCode
String servername="api.openweathermap.org";              // remote server program will connect to
String result;

float Temperature;
float Humidity;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
}

void loop() {
  getWeatherData();
  Serial.println(String(Temperature) + " " + String(Humidity));
  delay(5000);
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

  Temperature = root["main"]["temp"];
  Humidity = root["main"]["humidity"];
  result = "";
}
