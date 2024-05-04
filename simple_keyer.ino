/***********************************************************************
 * File: simple_keyer.ino
 * Author: Dan Quigley, N7HQ
 * Date: April 2024
 *
 * Description:
 *     This sketch implements a versatile Morse code keyer for amateur
 *     radio use. It features adjustable words per minute (WPM) input,
 *     support for both iambic and straight keys, and a Morse code
 *     translator for live text to Morse conversion. This implementation
 *     utilizes debouncing for stable speed adjustment and integrates
 *     with an AD9833 module for tone generation.
 *
 * Usage:
 *     Compile with Arduino IDE and upload to an Arduino board compatible
 *     with the libraries and pin configurations used. Configure hardware
 *     connections as defined in the pin settings within the sketch.
 *
 * Dependencies:
 *     - Arduino.h: Basic Arduino functions and types.
 *     - Bounce2: Debouncing library for button inputs.
 *     - MorseCodeTranslator.h: Custom library for translating text to Morse code.
 *     - Keyer.h: Custom Morse keyer class.
 *     - MD_AD9833: Library for controlling the AD9833 frequency generator via SPI.
 *
 * Revisions:
 *     1.0 - Initial release. Basic functionality for Morse code keying and translation.
 *
 * Notes:
 *     This code is intended for experimental and educational purposes and may
 *     require modifications for specific applications or higher performance needs.
 ***********************************************************************/

#include "Keyer.h"
#include "MorseCodeTranslator.h"

#define SERIAL_BAUD 115200

#define KEYER_DIT_PIN 3     // pin for DIT
#define KEYER_DAH_PIN 2     // pin for DAH
#define KEYER_OUTPUT_PIN 4  // keyer ouput pin
#define KEYER_LED_PIN LED_BUILTIN
#define KEYER_PTT_PIN 5
#define KEYER_PTT_HANG_TIME 250 // in ms

#define KEYER_SPEED_PIN A0  // wpm wiper (analog) pin

// Pins for SPI comm with the AD9833 IC
#define AD9833_FSYNC_PIN 10 // SPI Load pin number (FSYNC in AD9833 usage)
#define AD9833_CLK_PIN 11
#define AD9833_DATA_PIN 12

AD9833 ToneGen(AD9833_FSYNC_PIN, AD9833_CLK_PIN, AD9833_DATA_PIN);

KeyerConfig keyerConfig = 
{
  KEYER_DIT_PIN, 
  KEYER_DAH_PIN, 
  KEYER_OUTPUT_PIN, 
  KEYER_PTT_PIN, 
  KEYER_LED_PIN,
  KEYER_PTT_HANG_TIME,
  KEYER_SPEED_PIN
}; 

Keyer keyer(keyerConfig, ToneGen);
MorseCodeTranslator translator(keyer);

void setup()
{
  Serial.begin(SERIAL_BAUD);
  Serial.println(__FILE__);
  keyer.setup();
}

void loop()
{
  keyer.update();

  if (Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    translator.setText(input);
  }

  translator.update();
}
