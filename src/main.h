#include <Arduino.h>
#include "Mouse.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> 

void initScreen();
void computeStatsMs();
void drawMsValue(float ms);
void drawStdDevValue(float stddev);
void drawStartupScreen();
void drawInterrupted();
void isr();
void waitForButtonPress();
void printMeasurement();
void printAverage();
void showHeader();
void printError();