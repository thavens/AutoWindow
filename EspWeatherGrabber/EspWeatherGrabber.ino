#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>

const char* ssid = "SSID";
const char* password = "Password";
String APIKEY = "Open Weather API Key";                                 
String ZIPCode = "ZIP";   //ZIPCode
String servername="api.openweathermap.org";              // remote server program will connect to
String result;

float Temperature;
float Humidity;
float Temp_Min;
float Temp_Max;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while(WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed Rebooting");
    delay(2000);
    ESP.restart();
  }

  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("myesp8266");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  getWeatherData();
  Serial.println(String(Temperature) + " " + String(Humidity) + " " + String(Temp_Min) + " " + String(Temp_Max));
  delay(5000);

  ArduinoOTA.handle();
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
  Temp_Max = root["main"]["temp_max"];
  Temp_Min = root["main"]["temp_min"];
  
  result = "";
}
