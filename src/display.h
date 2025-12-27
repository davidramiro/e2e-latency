#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static const uint8_t SCREEN_WIDTH = 128;
static const uint8_t SCREEN_HEIGHT = 64;
static const int16_t LOWER_CURSOR_Y = SCREEN_HEIGHT / 2 - 4;
static const uint8_t GLYPH_NEW = 0x1A;
static const uint8_t GLYPH_AVG = 0xE5;
static const uint8_t GLYPH_SIGMA = 0xE4;
static const uint8_t GLYPH_LBRAK = 0xAF;
static const uint8_t GLYPH_RBRAK = 0xAE;
static const uint8_t DISPLAY_I2C_ADDR = 0x3C;
static const uint8_t BLINK_DELAY = 200;

void drawMsValue(float mean_ms);
void drawStdDevValue(float sd_ms);
void printMeasurement(uint16_t baseline, uint8_t cycle_index, double cycle_latency, uint16_t measured = 0);
void printAverage(double mean_ms, double sd_ms);
void showHeader();
void printError();
void drawStartupScreen();
void drawInterrupted();