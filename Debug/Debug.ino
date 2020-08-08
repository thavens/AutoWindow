#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

//String ssid = "YANG GANG!!!";
//String pass = "#TryHackingMyShittyWIFILMFAO";
//String ip = "192.168.1.6:25565";

#define SERVER_IP "192.168.1.6:25565"

#ifndef STASSID
#define STASSID "YANG GANG!!!"
#define STAPSK  "#TryHackingMyShittyWIFILMFAO"
#endif

String data = "";
unsigned long waitTime = 0;

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

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
    http.begin(client, "http://" SERVER_IP "/postplain/");
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
