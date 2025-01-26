# arduino_TFT_display_snow_interactive
An interactive snow animation on a touch screen driven by an Arduino.

It is a touch screen interactive variant of https://github.com/rchampeimont/arduino_TFT_display_snow.

I used this hardware:
* Adafruit RA8875 https://www.adafruit.com/product/1590
* Adafruit 7.0" 40-pin TFT Display - 800x480 with Touchscreen https://www.adafruit.com/product/2354
* Arduino: I tested both Arduino Uno Rev3 and Arduino Wifi Rev2 (which is based on Atmega 4809)

YouTube video showing the result: https://www.youtube.com/watch?v=W1X30Ba9tTY

Result:
![Screenshot](/images/screenshot.jpg?raw=true)

The first time you run the program, it is going to ask you to calibrate the touch screen (see picture below). It then stores the calibration data in EEPROM to skip calibration on future restarts. If you want to force recalibration, you can tie pin 5 to ground and restart the Arduino.
![Calibration](/images/calibration.jpg?raw=true)

What's behind:
![Overview of circuit](/images/overview.jpg?raw=true)

Schematic:
![Schematic](/images/schematic.jpg?raw=true)
