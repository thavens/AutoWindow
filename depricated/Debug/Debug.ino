#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const String SERVER_IP = "IP address";
const String ssid = "ssid";
const String pass = "password";

String data = "";
unsigned long waitTime = 0;

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  

  if(Serial.available()) {
    String str = Serial.readString();
    Serial.println(str);
    data += str;
  }
  
  if (data.length() > 20 && millis() - waitTime > 3000UL && WiFi.status() == WL_CONNECTED) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    http.begin(client, "http://" + SERVER_IP + "/postplain/");
    http.addHeader("Content-Type", "text/html");

    Serial.print("[HTTP] POST...\n");
    int httpCode = http.POST(data);
    data = "";

    if (httpCode > 0) {
      Serial.printf("[HTTP] code: %d\n", httpCode);
      waitTime = millis();
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}
