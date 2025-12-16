#include <Arduino.h>
#include "Mouse.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h> 

void initScreen();
static void computeStatsMs(const uint32_t* us, uint8_t n, float &mean_ms, float &sd_ms);
static inline void beginFrame();
static inline void drawMsValue(float ms);
static inline void drawStdDevValue(float stddev);
void printMeasurement();
void printAverage();
void countdown(int seconds);