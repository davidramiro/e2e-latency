#pragma once

#include <Arduino.h>
#include "Mouse.h"
#include <math.h> 
#include "display.h"

/// @brief Analog pin connected to signal of photodiode/photoresistor
static const uint8_t SENSOR_PIN = A3;
/// @brief Pin shorted to ground via button
static const uint8_t BUTTON_PIN = 7;
/// @brief RX LED PIN to show fault
static const uint8_t RX_LED_PIN = 17;
/// @brief Sensor threshold for registering a screen change event. 40 mV increments of the analog readout.
static const uint16_t BRIGHTNESS_THRESHOLD = 10;
/// @brief Number of measurements before calculating summary
static const uint8_t NUM_CYCLES = 20;
/// @brief Internal latency of the analog read, this lag will be subtracted from the measured latency
static const uint16_t internalLatency = 112;

static const uint16_t MEASUREMENT_DELAY_MS = 287;
static const double MS_FACTOR = 1000.0;

void initScreen();
void isr();
void measure();
void computeStatsMs(double *mean_ms, double *sd_ms);