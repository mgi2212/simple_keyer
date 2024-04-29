// N7HQ simple keyer
// April 2024

#include <Arduino.h>
#include <Bounce2.h>
#include "MorseCodeTranslator.h"
#include "Keyer.h"

#define SERIAL_BAUD 115200
#define INVERT_WPM true           // allows for idiots (like me) that wire the pot backwards
#define NUM_READINGS 10           // Number of samples for debouncing wpm pot

#define KEYER_DIT_PIN      3     // pin for DIT
#define KEYER_DAH_PIN      2     // pin for DAH
#define KEYER_OUTPUT_PIN  10     // keyer ouput pin
#define KEYER_SPEED_PIN   A0     // wpm wiper (analog) pin

// Pins for SPI comm with the AD9833 IC
#define AD9833_FSYNC_PIN  10	// SPI Load pin number (FSYNC in AD9833 usage)

MD_AD9833 toneGen(AD9833_FSYNC_PIN);
Keyer keyer(KEYER_DIT_PIN, KEYER_DAH_PIN, KEYER_OUTPUT_PIN, toneGen);
MorseCodeTranslator translator(keyer);

/// @brief debounced speed control
void updateWPM()
{
  static int readings[NUM_READINGS]; // Array to store readings
  static int readIndex = 0;          // Index of the current reading
  static int total = 0;              // Running total of readings
  static int average = 0;            // Average of readings
  static int lastWPM = 0;            // Last WPM value to check for significant change

  // Read the new value
  int newReading = analogRead(KEYER_SPEED_PIN);

  total = total - readings[readIndex];        // Subtract the last reading
  readings[readIndex] = newReading;           // Read from the sensor
  total = total + readings[readIndex];        // Add the reading to the total
  readIndex = (readIndex + 1) % NUM_READINGS; // Advance to the next position in the array
  if (readIndex == 0)
  {                                           // Only update WPM once all readings are refreshed
    average = total / NUM_READINGS;           // Calculate the average
    int wpm = map(average, 0, 1023,           // 5-40 wpm
                  (INVERT_WPM ? 40 : 5),
                  (INVERT_WPM ? 5 : 40));     // apply inversion (if set) INVERT_WPM directive

    if (abs(wpm - lastWPM) >= 1)              // 1 wpm change threshold
    { 
      keyer.setWPM(wpm);
      lastWPM = wpm;
    }
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  pinMode(KEYER_SPEED_PIN, INPUT);
  keyer.setup();
}

void loop()
{

  keyer.update();

  updateWPM();

  if (Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    translator.setText(input);
  }

  translator.update();
}
