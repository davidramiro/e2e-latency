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
static constexpr uint8_t DIODE_PIN = A3;
/// @brief Pin shorted to ground via button
static constexpr uint8_t BUTTON_PIN = 7;
/// @brief RX LED PIN to show fault
static constexpr uint8_t RX_LED_PIN = 17;
/// @brief Sensor threshold for registering a screen change event. 40 mV increments of the analog readout.
static constexpr uint16_t BRIGHTNESS_THRESHOLD = 20;
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
double mean_ms = 0.0f;
double sd_ms = 0.0f;
volatile boolean interrupted = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/// @brief Inits analog pin and mouse HID
void setup()
{
  initScreen();
  Serial.begin(115200);

  pinMode(DIODE_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), isr, FALLING);
}

void waitForButtonPress()
{
  while (digitalRead(BUTTON_PIN))
    ;
}

/// @brief Measures brightness, waits for brightness change, saves latency. Shows an average after last cycle.
void loop()
{
  if (cycle_index == 0)
  {
    waitForButtonPress();
    Mouse.begin();
  }

  // get reference brightness
  baseline = analogRead(DIODE_PIN);
  printMeasurement();

  delay(MEASUREMENT_DELAY_MS);

  baseline = analogRead(DIODE_PIN);

  // reset timer, click mouse
  unsigned long start = micros();
  Mouse.press(MOUSE_LEFT);

  while (true)
  {
    if (interrupted) {
        interrupted = false;
        Mouse.release();
        cycle_index = 0;
        
        drawInterrupted();
        delay(1000);
        drawStartupScreen();
        break;
    }

    int delta = analogRead(DIODE_PIN) - baseline;

    // loop until brightness delta is bigger than threshold
    if (abs(delta) > BRIGHTNESS_THRESHOLD)
    {
      // save and sum measured latency
      unsigned long latency = micros() - start;
      Mouse.release();

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
    computeStatsMs();   
    printAverage();

    cycle_index = 0;
    Mouse.end();
  }
}

void isr() {
  if (cycle_index == 0) {
    return;
  }

  interrupted = true;
}

void drawStartupScreen() {
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

void drawInterrupted() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(19, 0);
  display.print("Interrupted.");
  display.setCursor(0, LOWER_CURSOR_Y);
  display.setTextSize(1);
  display.print("Restarting...");
  display.display();
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

  drawStartupScreen();
}

/// @brief Calculates mean latency and sample standard deviation for an array of us latencies.
void computeStatsMs()
{
 if (NUM_CYCLES <= 1) {
    mean_ms = (double)latencies_us[0] / 1000.0;
    sd_ms = 0.0;
    return;
  }
  
  // calculate mean
  double sum_ms = 0.0;
  for (int i = 0; i < NUM_CYCLES; i++) {
      sum_ms += (double)latencies_us[i] / 1000.0;
  }
  mean_ms = sum_ms / NUM_CYCLES;
  
  // calculate sample standard deviation
  double variance_ms = 0.0;
  for (int i = 0; i < NUM_CYCLES; i++) {
      double diff_ms = ((double)latencies_us[i] / 1000.0) - mean_ms;
      variance_ms += diff_ms * diff_ms;
  }
  sd_ms = sqrt(variance_ms / (NUM_CYCLES - 1));
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

  drawMsValue(mean_ms);
  drawStdDevValue(sd_ms);
}