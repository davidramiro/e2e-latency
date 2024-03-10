# e2e-latency

Get a rough estimate on end-to-end (mouse-to-screen) system latency by attaching a photo resistor to your screen and measuring response time with an Arduino.

## BoM

- Arduino Pro Micro (ATmega32U4)
- Photoresistor module with analog output, e.g. KY-018 or G5516/LM393

## Schematic

![Schematic](doc/schema.png?raw=true)

## Usage

- Set up a repeatable test scenario where a mouse movement would cause a screen change (e.g. FPS game)
- Tape the photo resistor to the screen
- Connect USB
- Open a serial monitor with 115200 baud, e.g. `screen /dev/ttyACM0 115200`

Test starts 5 seconds after connecting USB. It will do 10 cycles, print the average latency and repeat until disconnected.