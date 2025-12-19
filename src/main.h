#include <Arduino.h>
#include "Mouse.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> 

void initScreen();
void computeStatsMs(const uint32_t* us, uint8_t n, float &mean_ms, float &sd_ms);
void drawMsValue(float ms);
void drawStdDevValue(float stddev);
void waitForButtonPress();
void printMeasurement();
void printAverage();
void showHeader();
void printError();
void showButtonReminder();