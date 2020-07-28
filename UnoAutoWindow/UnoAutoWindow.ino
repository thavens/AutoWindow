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
float OutTemp; //external temperature
float Humidity;
float Temp_Max;
float Temp_Min;

void setup()
{
  Serial.begin(115200);
  dht.begin();
  Serial.println("started");

  pinMode(userOpen, INPUT);
  pinMode(userClose, INPUT);

  attachInterrupt(digitalPinToInterrupt(userOpen), userCtrl, RISING);
  attachInterrupt(digitalPinToInterrupt(userClose), userCtrl, RISING);
  
  pinMode(LIMIT, INPUT_PULLUP);
  
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(ENAPin, OUTPUT);
  
  digitalWrite(dirPin, HIGH);
  digitalWrite(ENAPin, HIGH);
}

void loop()
{
  //check string completion
  if (stringComplete) {
    interpData();
    
    Serial.println("Outside temp: " + String(OutTemp) + "\tInside temp: " +
                  String(temp) + "\tMin: " + String(Temp_Min) + "/tMax: " + String(Temp_Max));
    bool isOpen = digitalRead(LIMIT) == HIGH;
    Serial.println("Window Open: " + String(isOpen));

    // clear the string:
    inputString = "";
    stringComplete = false;

    if(forceOpen && forceClose) {
      forceOpen = false;
      forceClose = false;
    }

    //user override mode
    if(!isOpen && forceOpen) {
      openWindow();
    }
    else if(isOpen && forceClose) {
      closeWindow();
    }
    else {
      //automatic mode
      if(!isOpen && OutTemp <= temp && (OutTemp >= 68 || temp >= 70)) {
        openWindow();
        delay(1800000); //pause to allow temperature change, reduces opens and closes due to temperature error
        serialDump();   //dump serial buffer to ignore old data from delay
      }
      else if(isOpen && (OutTemp > temp || (OutTemp < 68 && temp < 70)) {
        closeWindow();
        delay(1800000);
        serialDump();
      }
    }
  }
}

void interpData() {
  OutTemp = inputString.substring(0,inputString.indexOf(' ')).toFloat();
  inputString = inputString.substring(inputString.indexOf(' ')+1);
  Humidity = inputString.substring(0,inputString.indexOf(' ')).toFloat();
  inputString = inputString.substring(inputString.indexOf(' ')+1);
  Temp_Min = inputString.substring(0,inputString.indexOf(' ')).toFloat();
  inputString = inputString.substring(inputString.indexOf(' ')+1);
  Temp_Max = inputString.toFloat();
  temp = dht.readTemperature(true);
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

void userCtrl() {
  if(digitalRead(userClose)) {
    forceClose = !forceClose;
  }
  else if(digitalRead(userOpen)) {
    forceOpen = !forceOpen;
  }
}
