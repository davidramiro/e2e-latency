#include <Arduino.h>
#include "Mouse.h"
#include <Chrono.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

void reset();
void initScreen();
void moveMouseVertically(bool direction);
void printBufferToScreen(boolean segment);
void printBufferToScreen(boolean segment, int glyph);