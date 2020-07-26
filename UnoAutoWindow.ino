#include <DHT.h>;

#define dirPin 13     //stepper motor direction pin
#define stepPin 12    //pin to indicate step on motor
#define ENAPin 11     //stepper motor ative pin
#define DHTPin 2     // tempsensor pin (inside house)
#define DHTType DHT22   // DHT 22  (AM2302)
#define LIMIT 3       //limit switch pin connected to normally closed pin of limit switch (HIGH is open window LOW is closed window)
DHT dht(DHTPin, DHTType); // Initialize DHT sensor for normal 16mhz Arduino

String inputString = "";
bool stringComplete = false;

float temp; //internal temperature
float OutTemp; //external temperature

void setup()
{
  Serial.begin(115200);
  dht.begin();
  Serial.println("started");
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(ENAPin, OUTPUT);
  pinMode(LIMIT, INPUT_PULLUP);
  digitalWrite(dirPin, HIGH);
  digitalWrite(ENAPin, HIGH);
}

void loop()
{
  //check string completion, validate string data
  if (stringComplete && inputString.length() <= 14) {
    OutTemp = inputString.substring(0,inputString.indexOf(' ')).toFloat();
    Serial.println(inputString);
    Serial.print("Outside temp: " + String(OutTemp) + "\t");
    temp = dht.readTemperature()*9/5+32;
    Serial.println("Inside temp: " + String(temp));
    Serial.println(digitalRead(LIMIT));

    //remove 0 degree errors, Open window when temperature outside is more optimal than inside
    if(OutTemp != 0.00) {
      if(digitalRead(LIMIT) == LOW && OutTemp <= temp && (OutTemp >= 68 || temp >= 70)) {
        openWindow();
        delay(1800000); //pause to allow temperature change, reduces opens and closes due to temperature error
        serialDump();   //dump serial buffer to ignore old data from delay
      }
      else if((OutTemp > temp || (OutTemp < 68 && temp < 70)) && digitalRead(LIMIT) == HIGH) {
        closeWindow();
        delay(1800000);
        serialDump();
      }
    }
    
    // clear the string:
    inputString = "";
    stringComplete = false;
  }
  else if(stringComplete) {
    Serial.println(inputString);
    inputString = "";
    stringComplete = false;
  }
}

//recieve string data from esp8266 arduino
//ran by arduino after every loop
void serialEvent() { 
  while (Serial.available() && !stringComplete) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    if(inChar != '\n') {
    inputString += inChar;
    } else {
      stringComplete = true;
    }
  }
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

void serialDump() {
  while(Serial.available()) {
    Serial.read();
  }
}
