#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid     = "ssid";
const char* password = "password";

ESP8266WebServer server(80);

String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);

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
  server.handleClient();
}

void handleRoot() {
  //char temp[400];
  String temp = "<!DOCTYPE html>\
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
    <p> ESP window </p>\
    <p id='data'></p>\
    <button type='button' onClick=sendOpen()>Open Window</button>\
    <button type='button' onClick=sendClose()>Close Window</button>\
<script>\
    document.getElementById('data').innerHTML = 'data here'\
</script>\
</body>\
</html>";
  server.sendContent(temp);
}

void handleForm() {
  String state = "";
  if (server.args()) {
    state = server.arg(0);
    server.send(200, "text/plain", "state change");
  }
  else {
    server.send(400);
  }
  Serial.println(state);
}
