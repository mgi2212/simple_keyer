// N7HQ simple keyer
// April 2024

#include <Arduino.h>
#include <Bounce2.h>
#include "MorseCodeTranslator.h"
#include "Keyer.h"

#define SERIAL_BAUD 115200
#define INVERT_WPM true
#define NUM_READINGS 10  // Number of samples for averaging

const int dahPin = 2;
const int ditPin = 3;
const int keyerOutputPin = 10;
const int wpmSpeedPin = A0;

Keyer keyer(ditPin, dahPin, keyerOutputPin);
MorseCodeTranslator translator(keyer);

/// @brief debounced speed pot
void updateWPM() {
  static int readings[NUM_READINGS];  // Array to store readings
  static int readIndex = 0;           // Index of the current reading
  static int total = 0;               // Running total of readings
  static int average = 0;             // Average of readings
  static int lastWPM = 0;             // Last WPM value to check for significant change

  // Read the new value
  int newReading = analogRead(wpmSpeedPin);

  // Subtract the last reading
  total = total - readings[readIndex];
  // Read from the sensor
  readings[readIndex] = newReading;
  // Add the reading to the total
  total = total + readings[readIndex];
  // Advance to the next position in the array
  readIndex = (readIndex + 1) % NUM_READINGS;

  // Calculate the average
  if (readIndex == 0) {  // Only update WPM once all readings are refreshed
    average = total / NUM_READINGS;

    // apply inversion (if set) INVERT_WPM directive
    int wpm = map(average, 0, 1023, (INVERT_WPM ? 40 : 5), (INVERT_WPM ? 5 : 40));

    // Check if the change is significant enough to update the WPM
    if (abs(wpm - lastWPM) >= 1) {  // Change threshold, adjust as necessary
      keyer.setWPM(wpm);
      lastWPM = wpm;
    }
  }
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  pinMode(wpmSpeedPin, INPUT);
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
