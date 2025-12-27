#include "display.h"
#include "main.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/// @brief SSD1306 setup and rendering a splashscreen.
void initScreen()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDR))
  {
    // Fatal error - blink RX LED
    pinMode(RX_LED_PIN, OUTPUT);
    while (true)
    {
      digitalWrite(RX_LED_PIN, HIGH);
      delay(BLINK_DELAY);
      digitalWrite(RX_LED_PIN, LOW);
      delay(BLINK_DELAY);
    }
  }

  drawStartupScreen();
}

void drawStartupScreen()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(19, 0);
  display.write(GLYPH_LBRAK);
  display.print(" m2p-latency ");
  display.write(GLYPH_RBRAK);
  display.setCursor(0, LOWER_CURSOR_Y);
  display.setTextSize(1);
  display.print("Press button to start");
  display.display();
}

void drawInterrupted()
{
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

/// @brief Draws milliseconds onto the lower portion of the screen.
void drawMsValue(double ms)
{
  display.setTextSize(2);
  display.setCursor(0, LOWER_CURSOR_Y);
  const int digits = log10(ms) + 1;

  display.print(ms, 5 - digits);
  display.print(" ms");
  display.display();
}

/// @brief Draws std dev onto the lower portion of the screen.
void drawStdDevValue(double stddev)
{
  display.setTextSize(1);
  display.setCursor(0, LOWER_CURSOR_Y + 18);
  display.write(GLYPH_SIGMA);
  display.print(" ");
  display.print(stddev, 2);
  display.display();
}

/// @brief To be called between measurements, to show a live update to the user.
void printMeasurement(uint16_t baseline, uint8_t cycle_index, double cycle_latency, uint16_t measured)
{
  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("base: ");
  display.print(baseline);
  display.print(" ");

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
  display.print("repeating cycle...");
  display.display();
}

/// @brief To be called after all measurements finish, to show the statistics.
void printAverage(double mean_ms, double sd_ms)
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