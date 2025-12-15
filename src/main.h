#include <Arduino.h>
#include "Mouse.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void initScreen();
void countdown(int seconds);
void moveMouseVertically(bool direction);
void printBufferToScreen(boolean segment, int glyph = 0);