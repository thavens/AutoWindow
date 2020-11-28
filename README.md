# AutoWindow
Use to your liking

# How It Works
This window opener gets and parses weather data from Openweather API using an ESP8266 (ESP-01) which is then serially transmits the data to the Arduino UNO. The arduino UNO is programmed to control the window with stepper motors interfaced to the window with a rack and pinion. The window has a limit switch for the closed state to guarantee accurate closing ever time.

WARNING esp-01 will burn out after a period of useage. I speculate that this is due to ESP OTA causing over heating.

# Complete
-User window override <br>
-Some sound reduction

# Currently in Development
-rewriting to use NODEMCU ESP 8266 instead of arduino UNO combined with ESP8266-01 <br>
-small lcd status display <br>
-More robust decision making <br>

# Future
-Remote / IR user window override
-window locking system
