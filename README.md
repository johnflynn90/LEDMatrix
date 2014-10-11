LEDMatrix
=========

Arduino code for my Burning Man LED Matrix.


Code works with a 5x5 RGB LED Matrix with TLC5940 LED Drivers and an Arduino Mega. Be sure to download and link the tlc5940 library.

Use the following tutorial to set up your Arduino and LED Drivers:
http://playground.arduino.cc/learning/TLC5940

Depending on how you connect your LED pins to you will need to adjust pixel_map. The tlc5940 library numbers pins from 0 to 16*N-1 where N is the number of TLC5940's daisychained together. pixel_map maps a virtual pin address to this phycial address based on the daisychain.

Three active-low pushbottons will toggle the change of state of each of the three colors. Use the following tutorial to implement a pushbutton:
http://www.arduino.cc/en/Tutorial/Pushbutton
