#include "main.h"

uint32_t latencies_us[NUM_CYCLES] = {0};
uint8_t cycle_index = 0;

volatile boolean startRequested = false;
volatile boolean restartRequested = false;
boolean running = false;

/// @brief Inits analog pin and mouse HID
void setup()
{
  initScreen();
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr, FALLING);
}

/// @brief Measures brightness, waits for brightness change, saves latency. Shows an average after last cycle.
void loop()
{
  if (startRequested)
  {
    startRequested = false;
    restartRequested = false;
    Mouse.begin();

    while (cycle_index < NUM_CYCLES)
    {
      measure();

      if (restartRequested)
      {
        Mouse.release();
        cycle_index = 0;

        drawInterrupted();
        delay(1000);
        startRequested = false;
        restartRequested = false;
        running = false;
        drawStartupScreen();
        break;
      }
    }

    if (cycle_index == NUM_CYCLES)
    {

      double mean_ms = 0.0;
      double sd_ms = 0.0;
      computeStatsMs(&mean_ms, &sd_ms);
      printAverage(mean_ms, sd_ms);

      cycle_index = 0;
      running = false;
      startRequested = false;
      restartRequested = false;
      Mouse.end();
    }
  }
}

void measure()
{
  Serial.println("measure()");

  // get reference brightness
  uint16_t baseline = analogRead(SENSOR_PIN);
  printMeasurement(baseline, cycle_index, 0.0);

  delay(MEASUREMENT_DELAY_MS);

  running = true;

  baseline = analogRead(SENSOR_PIN);

  // reset timer, click mouse
  const unsigned long start = micros();
  Mouse.press(MOUSE_LEFT);

  while (true)
  {
    if (restartRequested)
    {
      return;
    }

    const int32_t delta = analogRead(SENSOR_PIN) - baseline;

    // loop until brightness delta is bigger than threshold
    if (abs(delta) > BRIGHTNESS_THRESHOLD)
    {
      // save and sum measured latency
      long latency = micros() - start - internalLatency;
      Mouse.release();

      if (latency <= 0)
      {
        printError();
        delay(1000);
        break;
      }

      // Store this cycle in the array
      if (cycle_index < NUM_CYCLES)
      {
        latencies_us[cycle_index] = latency;
      }

      printMeasurement(baseline, cycle_index, latency / MS_FACTOR, baseline + delta);

      delay(MEASUREMENT_DELAY_MS);

      cycle_index++;
      break;
    }
  }
}

void isr()
{
  if (running)
  {
    restartRequested = true;
  }
  else
  {
    startRequested = true;
  }
}



/// @brief Calculates mean latency and sample standard deviation for an array of us latencies.
void computeStatsMs(double *mean_ms, double *sd_ms)
{
  double sum_us = 0.0;
  double sd_us = 0.0;
  double mean_us = 0.0;
  double variance_us = 0.0;

  if (NUM_CYCLES == 0)
  {
    *mean_ms = 0.0;
    *sd_ms = 0.0;
    return;
  }

  if (NUM_CYCLES == 1)
  {
    *mean_ms = (double)latencies_us[0] / MS_FACTOR;
    *sd_ms = 0.0;
    return;
  }

  // calculate mean
  for (int i = 0; i < NUM_CYCLES; i++)
  {
    sum_us += (double)latencies_us[i];
  }
  mean_us = sum_us / (double)NUM_CYCLES;

  // calculate sample standard deviation

  for (int i = 0; i < NUM_CYCLES; i++)
  {
     const double diff_us = ((double)latencies_us[i]) - mean_us;
    variance_us += diff_us * diff_us;
  }
  sd_us = sqrt(variance_us / (NUM_CYCLES - 1));

  *mean_ms = mean_us / MS_FACTOR;
  *sd_ms = sd_us / MS_FACTOR;
}
