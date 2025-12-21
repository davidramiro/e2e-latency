# e2e-latency

Get a rough estimate on input-to-photon (mouse-to-screen) system latency by attaching a phototransistor to your screen and measuring response time with an Arduino.

## BoM

- Arduino Pro Micro (ATmega32U4)
- SSD1306 I2C OLED screen
- Phototransistor with high frequency, e.g. TEMT6000
- Button

## Wiring

![Wiring](doc/schema.png?raw=true)

## Usage

- Set up a repeatable test scenario where a mouse click would cause a screen change (e.g. FPS game, mouse latency test at (testufo.com)[https://testufo.com/flicker])
- Hold the photo transistor to the screen
- Connect USB
- Press button to start 20 measurements, wait for average to be reported on screen
