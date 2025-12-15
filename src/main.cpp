#include "main.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

/// @brief Analog pin of the photodiode
const int photoDiodePin = A1;
/// @brief Percentage threshold of brightness change that indicates screen movement
const float brightnessChangeThreshold = 0.2;

unsigned long sumLatency = 0;
int cycles = 0;

String topScreenBuf = "";
String lowerScreenBuf = "";
char buffer[40];

Chrono chrono;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

/// @brief Inits serial, analog pin and mouse HID
void setup()
{
  Serial.begin(115200);
  Serial.println("init");
  initScreen();

  pinMode(photoDiodePin, INPUT);
  Mouse.begin();
  countdown(5);
}

/// @brief Measures brightness, waits for brightness change, saves latency. Shows an average after 10 cycyles.
void loop()
{

  sprintf(buffer, "cycle %d", cycles + 1);
  printBufferToScreen(true);

  delay(500);

  // get reference brightness
  int brightness = analogRead(photoDiodePin);
  sprintf(buffer, "baseline: %d", brightness);
  printBufferToScreen(true);

  delay(500);

  // reset timer, click mouse
  uint32_t start = micros();
  Mouse.click(MOUSE_LEFT);

  while (true)
  {
    int delta = analogRead(photoDiodePin) - brightness;

    // loop until brightness delta is bigger than threshold
    if (abs(delta) > (brightness * brightnessChangeThreshold))
    {
      // save and sum measured latency
      unsigned long latency = micros() - start;
      sumLatency += latency;

      float latency_ms = latency / 1000.0f;

      // FIXME: truncate output to fit screen
      char ms[16];
      dtostrf(latency_ms, 0, 2, ms);

      sprintf(buffer, "measured: %d", (brightness + delta));
      printBufferToScreen(true);

      sprintf(buffer, "%s ms", ms);
      printBufferToScreen(false);

      delay(500);
      brightness = 0;

      cycles++;
      break;
    }
  }

  // print summary with average latency
  if (cycles == 10)
  {
    unsigned long avg = sumLatency / cycles;
    float avg_ms = avg / 1000.0f;
    char ms[16];
    dtostrf(avg_ms, 0, 2, ms);

    sprintf(buffer, "10 cycles avg:");
    printBufferToScreen(true);

    sprintf(buffer, "  %s ms", ms);
    printBufferToScreen(false, 0xEC);

    sumLatency = 0;
    cycles = 0;

    delay(5000);
    countdown(5);
  }
}

void initScreen()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

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

void printBufferToScreen(boolean segment, int glyph)
{
  if (segment)
  {
    topScreenBuf = String(buffer);
  }
  else
  {
    lowerScreenBuf = String(buffer);
  }

  display.clearDisplay();
  display.drawFastHLine(0, SCREEN_HEIGHT / 3, 128, WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(topScreenBuf);
  display.display();

  display.setTextSize(2);
  display.setCursor(0, SCREEN_HEIGHT / 2);
  display.write(glyph);
  display.println(lowerScreenBuf);
  display.display();
}

void countdown(int seconds)
{
  for (; seconds > 0; seconds--)
  {
    sprintf(buffer, "starting in %d...", seconds);
    printBufferToScreen(true);
    delay(1000);
  }
}