# phone-keys-usb

A USB HID-device based on a Arduino pro micro (atmega32u8) that emulates a keyboard. It can be connected to the matrix of a phone keypad and send the corresponding key presses to the computer.

Connect the 8 pins (4 rows and 4 columns) to pins 2-9 of the pro micro. Pin 10 and ground are connected to the hang up button of the phone.

Modify the code if you want some special keys. 1-3 is mapped to 7-9 in the pc keyboard, and viceversa.
