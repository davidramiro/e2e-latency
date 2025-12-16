#include "main.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
static constexpr int16_t DIVIDER_Y = SCREEN_HEIGHT / 3;
static constexpr int16_t LOWER_CURSOR_Y   = SCREEN_HEIGHT / 2 - 4;
static constexpr uint8_t GLYPH_NEW   = 0x1A;
static constexpr uint8_t GLYPH_AVG   = 0xEC;
static constexpr uint8_t GLYPH_MS    = 0xEA;
static constexpr uint8_t GLYPH_SIGMA = 0xE4;
static constexpr uint8_t GLYPH_LBRAK = 0xAF;
static constexpr uint8_t GLYPH_RBRAK = 0xAE;

static constexpr uint8_t DIODE_PIN = A1;
// arbitrary threshold for brightness change. 0.2 seems good for photoresistor, 0.05 for photodiode.
static constexpr float BRIGHTNESS_THRESHOLD = 0.05;
// 
static constexpr uint8_t NUM_CYCLES = 10;

uint32_t latencies_us[NUM_CYCLES] = {0};
uint8_t cycleIndex = 0;
int baseline = 0;
int measured = 0;
float cycle_latency = 0.0f;
float avg_ms = 0.0f;
float stddev_ms = 0.0f;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/// @brief Inits analog pin and mouse HID
void setup()
{
  initScreen();

  pinMode(DIODE_PIN, INPUT);
  Mouse.begin();
  countdown(5);
}

/// @brief Measures brightness, waits for brightness change, saves latency. Shows an average after last cycle.
void loop()
{
  // get reference brightness
  baseline = analogRead(DIODE_PIN);
  printMeasurement();

  delay(500);

  baseline = analogRead(DIODE_PIN);

  // reset timer, click mouse
  uint32_t start = micros();
  Mouse.click(MOUSE_LEFT);

  while (true)
  {
    int delta = analogRead(DIODE_PIN) - baseline;

    // loop until brightness delta is bigger than threshold
    if (abs(delta) > (baseline * BRIGHTNESS_THRESHOLD))
    {
      // save and sum measured latency
      unsigned long latency = micros() - start;

      // Store this cycle in the array
      if (cycleIndex < NUM_CYCLES) {
        latencies_us[cycleIndex] = latency;
      }

      cycle_latency = latency / 1000.0f;
      measured = baseline + delta;

      printMeasurement();

      delay(500);
      measured = 0;

      cycleIndex++;
      break;
    }
  }

  if (cycleIndex == NUM_CYCLES)
  {   
    computeStatsMs(latencies_us, NUM_CYCLES, avg_ms, stddev_ms);

    cycleIndex = 0;

    printAverage();

    delay(5000);
    countdown(5);
  }
}

/// @brief SSD1306 setup, showing a splashscreen and waiting 2s.
void initScreen()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(15, 0);
  display.write(0x10);
  display.print(" e2e-latency ");
  display.write(0x11);
  display.display();
  delay(2000);
}

/// @brief Calculates average ms latency and standard deviation for an array of us latencies.
static void computeStatsMs(const uint32_t* us, uint8_t n, float &mean_ms, float &sd_ms)
{
  float mean = 0.0f;
  float M2   = 0.0f;

  for (uint8_t i = 0; i < n; ++i) {
    const float x = us[i] / 1000.0f;
    const float delta = x - mean;
    mean += delta / float(i + 1);
    const float delta2 = x - mean;
    M2 += delta * delta2;
  }

  mean_ms = mean;
  sd_ms   = (n > 1) ? sqrtf(M2 / float(n - 1)) : 0.0f; // sample std dev [web:23]
}

/// @brief Refresh screen, draw divier line.
static inline void beginFrame()
{
  display.clearDisplay();
  display.drawFastHLine(0, DIVIDER_Y, SCREEN_WIDTH, WHITE);
  display.setTextColor(WHITE);
}

/// @brief Draws milliseconds onto the lower portion of the screen.
static inline void drawMsValue(float ms)
{
  display.setTextSize(2);
  display.setCursor(0, LOWER_CURSOR_Y);
  display.write(GLYPH_MS);
  display.print(ms, 2);
  display.print("ms");
}

/// @brief Draws std dev onto the lower portion of the screen.
static inline void drawStdDevValue(float stddev)
{
  display.setTextSize(2);
  display.setCursor(0, LOWER_CURSOR_Y + 18);
  display.write(GLYPH_SIGMA);
  display.print(stddev, 2);
}

/// @brief To be called between measurements, to show a live update to the user.
void printMeasurement()
{
  beginFrame();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("base: ");
  display.print(baseline);

  if (measured != 0) {
    display.write(GLYPH_NEW);
    display.print(" new: ");
    display.print(measured);
  }

  display.setCursor(0, 8);
  display.print(cycleIndex + 1);
  display.print(" / ");
  display.print(NUM_CYCLES);

  drawMsValue(cycle_latency);

  display.display();
}

/// @brief To be called after all measurements finish, to show the statistics.
void printAverage()
{
  beginFrame();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("average over");
  display.print(NUM_CYCLES);
  display.println(" cycles:");

  drawMsValue(avg_ms);
  drawStdDevValue(stddev_ms);

  display.display();
}

/// @brief Shows a countdown of 5 seconds on the screen.
void countdown(int seconds)
{
  for (; seconds > 0; --seconds) {
    beginFrame();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("starting in...");

    display.setTextSize(2);
    display.setCursor(0, LOWER_CURSOR_Y);
    display.write(GLYPH_LBRAK);
    display.print(' ');
    display.print(seconds);
    display.print(' ');
    display.write(GLYPH_RBRAK);

    display.display();
    delay(1000);
  }
}