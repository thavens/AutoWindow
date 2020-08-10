#include <DHT.h>;

#define dirPin 13     //stepper motor direction pin
#define stepPin 12    //pin to indicate step on motor
#define ENAPin 11     //stepper motor ative pin
#define DHTPin 4     // tempsensor pin (inside house)
#define DHTType DHT22   // DHT 22  (AM2302)
#define LIMIT 5       //limit switch pin connected to normally closed pin of limit switch (HIGH is open window LOW is closed window)
#define userOpen 2    //Allow the user to open window (toggle open tactile button)
#define userClose 3   //Allow user to close the window (toggle close)
//hitting open or close first will cause manual overide into said open close state. hitting either open or close (doesn't matter) will reset window to automode

DHT dht(DHTPin, DHTType); // Initialize DHT sensor for normal 16mhz Arduino

String inputString = "";
bool stringComplete = false;
bool forceOpen = false;
bool forceClose = false;

float temp; //internal temperature
float outTemp; //external temperature
float Humidity;
float tempMax;
float tempMin;

bool isOpen;

unsigned long waitTime = 0;

void setup()
{
  Serial.begin(115200);
  dht.begin();
  Serial.println("started");

  pinMode(userOpen, INPUT);
  pinMode(userClose, INPUT);
  
  pinMode(LIMIT, INPUT_PULLUP);
  
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(ENAPin, OUTPUT);
  
  digitalWrite(dirPin, HIGH);
  digitalWrite(ENAPin, HIGH);

  attachInterrupt(digitalPinToInterrupt(userOpen), userCtrl, RISING);
  attachInterrupt(digitalPinToInterrupt(userClose), userCtrl, RISING);
}

void loop()
{
  isOpen = digitalRead(LIMIT) == HIGH;
  
  if(forceOpen && forceClose) {
    forceOpen = false;
    forceClose = false;
  }
  
  //user override mode
  if(forceOpen) {
    if(!isOpen) {
      openWindow();
    }
  }
  else if(forceClose) {
    if(isOpen) {
      closeWindow();
    }
  }
  //check string completion
  else if (stringComplete && inputString.length() > 20 && inputString.length() < 25) {
    interpData();
    
    Serial.println("Outside temp: " + String(outTemp) + "\tInside temp: " +
                  String(temp) + "\tMin: " + String(tempMin) + "\tMax: " + String(tempMax));
    Serial.print("Window Open: ");
    if(isOpen) {
      Serial.println("True");
    }
    else {
      Serial.println("False");
    }

    // clear the string:
    inputString = "";
    stringComplete = false;

    if (millis() - waitTime > 720000UL && outTemp != 0.00) {
      if(!isOpen && ((tempMax > 80 && outTemp <= temp && temp > 66)
      || (tempMax < 68 && outTemp > temp)
      || (tempMax <= 80 && tempMax >= 68 && outTemp >= 68 && outTemp <= 76))) {
        openWindow();
        waitTime = millis();
        serialDump();
      }
      else if(isOpen && ((tempMax < 68 && outTemp <= temp)
      || (tempMax >= 80 && (outTemp > temp || temp < 66))
      || (68 <= tempMax && tempMax <= 76 && (68 < outTemp || outTemp < 76)))) {
        closeWindow();
        waitTime = millis();
        serialDump();
      }
    }
  }
  else if (stringComplete) {
    Serial.println("There was an error in the esp 8266");
    Serial.println(inputString);
    inputString = "";
    stringComplete = false;
    serialDump();
  }
  
  delay(200);
}

void interpData() {
  outTemp = inputString.substring(0,inputString.indexOf(' ')).toFloat();
  inputString = inputString.substring(inputString.indexOf(' ')+1);
  Humidity = inputString.substring(0,inputString.indexOf(' ')).toFloat();
  inputString = inputString.substring(inputString.indexOf(' ')+1);
  tempMin = inputString.substring(0,inputString.indexOf(' ')).toFloat();
  inputString = inputString.substring(inputString.indexOf(' ')+1);
  tempMax = inputString.toFloat();
  temp = dht.readTemperature(true);
}

//recieve string data from esp8266 arduino
//ran by arduino after every loop
void serialEvent() { 
  while (Serial.available()) {
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

void userCtrl() {
  Serial.println("usercontrol triggered");
  if(digitalRead(userClose)) {
    forceClose = !forceClose;
    Serial.println("user close");
  }
  else if(digitalRead(userOpen)) {
    forceOpen = !forceOpen;
    Serial.println("user open");
  }
}
