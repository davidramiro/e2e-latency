#include <Arduino.h>
#include "Mouse.h"
#include <Chrono.h>

/// @brief Analog pin of the photodiode
const int photoDiodePin = A1;
/// @brief Percentage threshold of brightness change that indicates screen movement
const float brightnessChangeThreshold = 0.2;

/// @brief Factor of how far to move the mouse
const int mouseDistance = 8;

int brightness = 0;
uint32_t sumLatency = 0;
int cycles = 0;

Chrono chrono(Chrono::MICROS);

/// @brief Inits serial, analog pin and mouse HID
void setup()
{
  Serial.begin(115200);
  Serial.println("init");
  pinMode(photoDiodePin, INPUT);
  Mouse.begin();
  Serial.println("starting in 5 seconds");
  delay(5000);
}

/// @brief Moves the mouse vertically via USB
/// @param direction true for up, false for down
void moveMouseVertically(bool direction)
{
  char yTravel = direction ? -127 : 127;

  for (uint8_t i = 0; i < mouseDistance; i++)
  {
    Mouse.move(0, yTravel, 0);
  }
}

/// @brief Resets temporary brightness value and mouse position
void reset()
{
  moveMouseVertically(false);
  delay(500);
  brightness = 0;
}

/// @brief Measures brightness, waits for brightness change, saves latency. Shows an average after 10 cycyles.
void loop()
{
  Serial.print("measurement cycle ");
  Serial.print(cycles);
  Serial.println(" start");

  delay(500);

  // get reference brightness
  brightness = analogRead(photoDiodePin);

  Serial.print("reference brightness: ");
  Serial.println(brightness);

  delay(500);

  // reset timer, move mouse
  chrono.restart();
  Serial.println("sending mouse movement");
  moveMouseVertically(true);

  while (true)
  {
    int delta = analogRead(photoDiodePin) - brightness;

    // loop until brightness delta is bigger than threshold
    if (abs(delta) > (brightness * brightnessChangeThreshold))
    {
      // save and sum measured latency
      int latency = chrono.elapsed();
      sumLatency += latency;
      cycles++;

      Serial.print("movement detected after ");
      Serial.print(latency);
      Serial.print(" µs, brightness readout: ");
      Serial.println(brightness + delta);
      delay(500);

      reset();
      break;
    }
  }

  // print summary with average latency
  if (cycles == 10)
  {
    uint32_t avg = sumLatency / cycles;
    Serial.print(sumLatency);
    Serial.println();
    Serial.println("=========================");
    Serial.println("10 cycle average latency:");
    Serial.print(avg);
    Serial.print(" (");
    Serial.print(avg / 1000);
    Serial.println(" ms)");
    Serial.println("=========================");
    Serial.println();

    sumLatency = 0;
    cycles = 0;

    delay(2000);
  }
}
