/***********************************************************************
 * File: keyer.h
 * Author: Dan Quigley, N7HQ
 * Date: April 2024
 *
 * Description:
 *     This file contains the implementation of the Keyer class which
 *     controls a Morse code keyer with PWM sidetone generation. The
 *     keyer is designed for amateur radio applications, supporting
 *     adjustable words per minute (WPM) and sidetone frequency, with
 *     high-resolution timing options for precise control. It supports
 *     both iambic and straight keying modes.
 *
 * Usage:
 *     Include this file in an Arduino project that requires Morse code
 *     keying functionality. Ensure that the hardware is connected to
 *     the specified pins and that the AD9833 module and other dependencies
 *     are properly set up.
 *
 * Dependencies:
 *     - Arduino.h: Basic Arduino library for hardware control.
 *     - Bounce2: Debouncing library for button inputs.
 *     - MD_AD9833.h: Library for controlling AD9833 modules via SPI.
 *
 * Revisions:
 *     1.0 - Initial release. Provides basic keying functionalities and PWM tone generation.
 *
 * Notes:
 *     This implementation uses high-resolution timers to ensure accurate timing
 *     which is critical for Morse code applications. The code structure allows
 *     for easy integration and customization in amateur radio projects.
 ***********************************************************************/

#include "Keyer.h"

#define DIGITAL_PIN_DEBOUNCE_INTERVAL 10
#define SIDETONE_FREQUENCY 880.0
#define WPM_RESOLUTION 1200000

Keyer::Keyer(KeyerConfig &config, AD9833 &toneGen)
    : config(config), toneGen(toneGen), wpm(20), currentState(IDLE)
{
  debouncerDah = Bounce2::Button();
  debouncerDit = Bounce2::Button();
  config.pttHangTime *= 1000;
}

void Keyer::setup()
{
  updateTiming();

  SPI.begin();
  
  pinMode(config.wpmSpeedPin, INPUT);

  pinMode(config.ditPin, INPUT_PULLUP);
  pinMode(config.dahPin, INPUT_PULLUP);
  pinMode(config.outputPin, OUTPUT);
  pinMode(config.pttPin, OUTPUT);
  pinMode(config.ledPin, OUTPUT);

  digitalWrite(config.outputPin, LOW);
  digitalWrite(config.pttPin, LOW);
  digitalWrite(config.ledPin, LOW);

  debouncerDit.attach(config.ditPin, INPUT_PULLUP);
  debouncerDah.attach(config.dahPin, INPUT_PULLUP);

  debouncerDit.interval(DIGITAL_PIN_DEBOUNCE_INTERVAL);
  debouncerDah.interval(DIGITAL_PIN_DEBOUNCE_INTERVAL);

  toneGen.begin();
  toneGen.setWave(AD9833_OFF);
  toneGen.setFrequency(SIDETONE_FREQUENCY, 0);
  toneGen.setFrequencyChannel(0);

}

void Keyer::update()
{
  updateWPM();
  currentTime = micros();
  
  debouncerDit.update();
  debouncerDah.update();

  bool ditState = !debouncerDit.read();
  bool dahState = !debouncerDah.read();
  bool iambicState = ditState && dahState;

  switch (currentState)
  {
  case IDLE:
    if (iambicState)
    {
      sendDit(); // Start with a dit in iambic mode
      currentState = IAMBIC_DIT;
    }
    else if (ditState)
    {
      sendDit();
      currentState = TRANSMITTING_DIT;
    }
    else if (dahState)
    {
      sendDah();
      currentState = TRANSMITTING_DAH;
    }
    break;

  case IAMBIC_DIT:
  case IAMBIC_DAH:
    if (currentTime >= transmissionEndTime)
    {
      toggleOutput(false);  // Ensure output is off before next element
      if (iambicState)      // Check if still in iambic mode
      { 
        previousState = currentState;
        waitingEndTime = currentTime + elementSpace; // Setup waiting end time
      }
      currentState = WAITING_ELEMENT_SPACE; // Wait for element space
    }
    break;

  case WAITING_ELEMENT_SPACE:
    if (currentTime >= waitingEndTime)
    {
      if (iambicState)
      {
        if (previousState == IAMBIC_DIT)
        {
          currentState = IAMBIC_DAH;
          sendDah();
        }
        else
        {
          currentState = IAMBIC_DIT;
          sendDit();
        }
      }
      else
      {
        currentState = IDLE;
      }
    }
    break;

  case TRANSMITTING_DIT:
  case TRANSMITTING_DAH:
    if (currentTime >= transmissionEndTime)
    {
      toggleOutput(false);
      currentState = WAITING_ELEMENT_SPACE;
      waitingEndTime = currentTime + elementSpace;
    }
    break;

  case WAITING_CHARACTER_SPACE:
  case WAITING_WORD_SPACE:
    if (currentTime >= waitingEndTime)
    {
      currentState = IDLE; // Transition back to IDLE after the space period
    }
    break;

  default:
    break;
  }

  checkEndTransmission();
}

void Keyer::beginTransmission()
{
  if (pttTimerStarted || (digitalRead(config.pttPin) == HIGH))
  {
    return;
  }

  digitalWrite(config.pttPin, HIGH); // Assert PTT HIGH on transmission start
  transmissionStartTime = currentTime;
  pttTimerStarted = true;

#ifdef DEBUG_OUTPUT
  Serial.println(F("PTT: ON"));
#endif

}

void Keyer::checkEndTransmission()
{
  if (
      pttTimerStarted &&
      isReadyForInput() &&
      (currentTime > transmissionStartTime) &&
      (currentTime >= lastKeyEndTime + config.pttHangTime) &&
      (currentTime >= waitingEndTime + config.pttHangTime))
  {
    digitalWrite(config.pttPin, LOW); // Turn off PTT after hang time
    pttTimerStarted = false;          // Reset flag

#ifdef DEBUG_OUTPUT
    float pttTime = (currentTime - transmissionStartTime) / 1000000.0;
    sprintf(strBuffer, "PTT: OFF (%.2fs)", pttTime);
    Serial.println(strBuffer);
#endif

  }
}

void Keyer::sendDit()
{
  toggleOutput(true);
  transmissionEndTime = micros() + ditDuration;
  lastKeyEndTime = transmissionEndTime;
}

void Keyer::sendDah()
{
  toggleOutput(true);
  transmissionEndTime = micros() + dahDuration;
  lastKeyEndTime = transmissionEndTime;
}

void Keyer::toggleOutput(bool state)
{
  if (state)
  {
    beginTransmission();
  }
  digitalWrite(config.ledPin, state ? HIGH : LOW);
  digitalWrite(config.outputPin, state ? HIGH : LOW);
  toneGen.setWave(state ? AD9833_SINE : AD9833_OFF);
}

// these are approximate values for testing, a more robust "fist" can be achieved with some more work.
void Keyer::updateTiming()
{
    ditDuration = WPM_RESOLUTION / wpm;               // Duration of a dit
    dahDuration = 3 * ditDuration;                    // Dah is three times the duration of dit
    elementSpace = ditDuration;                       // Space between elements

    // Sliding scale factor for Farnsworth timing
    float farnsworthFactor = 1.2f - 0.02f * wpm;
    farnsworthFactor = max(0.4f, min(farnsworthFactor, 1.0f)); // Clamp the factor between 0.4 and 1.0

    // Apply Farnsworth factor to spaces
    characterSpace = static_cast<int>(3 * ditDuration * farnsworthFactor);
    wordSpace = static_cast<int>((7 * ditDuration) * (1.2f - 0.02f * farnsworthWPM));

#ifdef DEBUG_OUTPUT
    sprintf(strBuffer, "WPM=%d, Farnsworth Factor=%.2f, Char Space=%d, Word Space=%d",
            wpm, farnsworthFactor, characterSpace, wordSpace);
    Serial.println(strBuffer);
#endif
}

/// @brief returns true when keyer is ready for input (in IDLE state)
bool Keyer::isReadyForInput() const
{
  return currentState == IDLE; // Only consider ready if truly idle, not just between symbols
}

/// @brief Call only when keyer is ready for input (IDLE state)
bool Keyer::sendCharacterSpace()
{
  if (!isReadyForInput())
  {
    return false;
  }
  beginTransmission();
  toggleOutput(false); // Ensure the output is off
  currentState = WAITING_CHARACTER_SPACE;
  waitingEndTime = micros() + characterSpace;
  return true;
}

/// @brief Call only when keyer is ready for input (IDLE state)
bool Keyer::sendWordSpace()
{
  if (!isReadyForInput())
  {
    return false;
  }
  beginTransmission();
  toggleOutput(false); // Ensure the output is off
  currentState = WAITING_WORD_SPACE;
  waitingEndTime = micros() + wordSpace;
  return true;
}

/// @brief starts sending a dit. Call only when keyer is ready for input (IDLE state)
bool Keyer::triggerDit()
{
  if (!isReadyForInput())
  {
    return false;
  }
  sendDit();
  currentState = TRANSMITTING_DIT; // Update state appropriately
  return true;
}

/// @brief starts sending a dah. Call only when keyer is ready for input (IDLE state)
bool Keyer::triggerDah()
{
  if (!isReadyForInput())
  {
    return false;
  }
  sendDah();
  currentState = TRANSMITTING_DAH;
  return true;
}

void Keyer::setWPM(int newWpm)
{
  if (wpm == newWpm) return;
  wpm = newWpm;
  updateTiming();
}

int Keyer::getWPM() const
{
  return wpm;
}

void Keyer::updateWPM()
{
  static int readings[NUM_READINGS]; // Array to store readings
  static int readIndex = 0;          // Index of the current reading
  static int total = 0;              // Running total of readings
  static int average = 0;            // Average of readings
  static int lastWPM = 0;            // Last WPM value to check for significant change

  // Read the new value
  int newReading = analogRead(config.wpmSpeedPin);

  total = total - readings[readIndex];        // Subtract the last reading
  readings[readIndex] = newReading;           // Read from the sensor
  total = total + readings[readIndex];        // Add the reading to the total
  readIndex = (readIndex + 1) % NUM_READINGS; // Advance to the next position in the array
  if (readIndex == 0)
  {                                 // Only update WPM once all readings are refreshed
    average = total / NUM_READINGS; // Calculate the average
    int wpm = map(average, 0, 1023, // 5-40 wpm
                  (INVERT_WPM ? 40 : 5),
                  (INVERT_WPM ? 5 : 40)); // apply inversion (if set) INVERT_WPM directive

    if (abs(wpm - lastWPM) >= 1) // 1 wpm change threshold
    {
      setWPM(wpm);
      lastWPM = wpm;
    }
  }
}
