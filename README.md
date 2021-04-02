# AutoWindow
Automatic window for my window!
use to your liking

# How It Works
NODEMCU ESP8266EX gets weather data from the open weather api every 10 min. Runs a webserver that serves html, with the inside/outside temperature data and buttons to open and close the window, to client and recieves post requests to open or close the window. 

# Future
- Would like to reimplement automatic open, but esp8266 seems to run out of resources easily when serving html. May need to switch to raspi.
