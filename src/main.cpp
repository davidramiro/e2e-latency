#include "main.h"

static constexpr uint8_t SCREEN_WIDTH = 128;
static constexpr uint8_t SCREEN_HEIGHT = 64;
static constexpr int16_t LOWER_CURSOR_Y = SCREEN_HEIGHT / 2 - 4;
static constexpr uint8_t GLYPH_NEW = 0x1A;
static constexpr uint8_t GLYPH_AVG = 0xE5;
static constexpr uint8_t GLYPH_SIGMA = 0xE4;
static constexpr uint8_t GLYPH_LBRAK = 0xAF;
static constexpr uint8_t GLYPH_RBRAK = 0xAE;

/// @brief Analog pin connected to signal of photodiode/photoresistor
static constexpr uint8_t DIODE_PIN = A1;
/// @brief Pin shorted to ground via button
static constexpr uint8_t BUTTON_PIN = 10;
/// @brief RX LED PIN to show fault
static constexpr uint8_t RX_LED_PIN = 17;
/// @brief Arbitrary threshold for brightness change. 0.2 seems good for photoresistor, 0.02 for photodiode.
static constexpr float BRIGHTNESS_THRESHOLD = 0.02;
/// @brief Number of measurements before calculating summary
static constexpr uint8_t NUM_CYCLES = 20;
/// @brief Internal latency of the analog read, this lag will be subtracted from the measured latency
static constexpr uint16_t internalLatency = 112;

static constexpr uint16_t MEASUREMENT_DELAY_MS = 300;

uint32_t latencies_us[NUM_CYCLES] = {0};
uint8_t cycle_index = 0;
uint16_t baseline = 0;
uint16_t measured = 0;
float cycle_latency = 0.0f;
float avg_ms = 0.0f;
float stddev_ms = 0.0f;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/// @brief Inits analog pin and mouse HID
void setup()
{
  initScreen();
  Serial.begin(115200);

  pinMode(DIODE_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void waitForButtonPress()
{
  while (!digitalRead(BUTTON_PIN))
    ;
}

/// @brief Measures brightness, waits for brightness change, saves latency. Shows an average after last cycle.
void loop()
{
  if (cycle_index == 0)
  {
    waitForButtonPress();
  }

  Mouse.begin();

  // get reference brightness
  baseline = analogRead(DIODE_PIN);
  printMeasurement();

  delay(MEASUREMENT_DELAY_MS);

  baseline = analogRead(DIODE_PIN);

  // reset timer, click mouse
  unsigned long start = micros();
  Mouse.click(MOUSE_LEFT);

  while (true)
  {
    int delta = analogRead(DIODE_PIN) - baseline;

    // loop until brightness delta is bigger than threshold
    if (abs(delta) > (baseline * BRIGHTNESS_THRESHOLD))
    {
      // save and sum measured latency
      unsigned long latency = micros() - start;

      if (latency < internalLatency)
      {
        printError();
        break;
      }

      // Store this cycle in the array
      if (cycle_index < NUM_CYCLES)
      {
        latencies_us[cycle_index] = latency - internalLatency;
      }

      cycle_latency = latency / 1000.0f;
      measured = baseline + delta;

      printMeasurement();

      delay(MEASUREMENT_DELAY_MS);
      measured = 0;

      cycle_index++;
      break;
    }
  }

  if (cycle_index == NUM_CYCLES)
  {
    computeStatsMs(latencies_us, NUM_CYCLES, avg_ms, stddev_ms);

    cycle_index = 0;

    printAverage();
  }

  Mouse.end();
}

/// @brief SSD1306 setup and rendering a splashscreen.
void initScreen()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    // Fatal error - blink RX LED
    pinMode(RX_LED_PIN, OUTPUT);
    while (true)
    {
      digitalWrite(RX_LED_PIN, HIGH);
      delay(200);
      digitalWrite(RX_LED_PIN, LOW);
      delay(200);
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(19, 0);
  display.write(0x10);
  display.print(" e2e-latency ");
  display.write(0x11);
  display.setCursor(0, LOWER_CURSOR_Y);
  display.setTextSize(1);
  display.print("Press button to start");
  display.display();
}

/// @brief Calculates average ms latency and standard deviation for an array of us latencies.
void computeStatsMs(const uint32_t *us, uint8_t n, float &mean_ms, float &sd_ms)
{
  float mean = 0.0f;
  float M2 = 0.0f;

  for (uint8_t i = 0; i < n; ++i)
  {
    const float ms = us[i] / 1000.0f;
    const float delta = ms - mean;
    mean += delta / float(i + 1);
    const float delta2 = ms - mean;
    M2 += delta * delta2;
  }

  mean_ms = mean;
  sd_ms = (n > 1) ? sqrtf(M2 / float(n - 1)) : 0.0f;
}

/// @brief Draws milliseconds onto the lower portion of the screen.
void drawMsValue(float ms)
{
  display.setTextSize(2);
  display.setCursor(0, LOWER_CURSOR_Y);
  int digits = log10(ms) + 1;

  display.print(ms, 5 - digits);
  display.print(" ms");
  display.display();
}

/// @brief Draws std dev onto the lower portion of the screen.
void drawStdDevValue(float stddev)
{
  display.setTextSize(1);
  display.setCursor(0, LOWER_CURSOR_Y + 18);
  display.write(GLYPH_SIGMA);
  display.print(" ");
  display.print(stddev, 2);
  display.display();
}

/// @brief To be called between measurements, to show a live update to the user.
void printMeasurement()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("base: ");
  display.print(baseline);

  if (measured != 0)
  {
    display.write(GLYPH_NEW);
    display.print(" new: ");
    display.print(measured);
  }

  display.setCursor(0, 8);
  display.print(cycle_index + 1);
  display.print(" / ");
  display.print(NUM_CYCLES);

  drawMsValue(cycle_latency);
}

void printError()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("implausible value!");
  display.print("repeating cycle ");
  display.print(cycle_index);
  display.display();
}

/// @brief To be called after all measurements finish, to show the statistics.
void printAverage()
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Press button");
  display.println("to restart.");
  display.write(GLYPH_AVG);
  display.print(" over ");
  display.print(NUM_CYCLES);
  display.println(" cycles:");

  drawMsValue(avg_ms);
  drawStdDevValue(stddev_ms);
}