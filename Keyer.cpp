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

#define PWM_FREQUENCY 600  // PWM sidetone frequency in Hz
#define DIGITAL_PIN_DEBOUNCE_INTERVAL 5
// comment out for milli-second timer
#define USE_HIGHRES_TIMER 1

#ifdef USE_HIGHRES_TIMER
#define WPM_RESOLUTION 1200000
#else
#define WPM_RESOLUTION 1200
#endif

#define TONE_PIN 7

Keyer::Keyer(KeyerConfig &config, AD9833 &toneGen)
  : config(config), toneGen(toneGen), wpm(20), farnsworthWPM(15), currentState(IDLE) {
  debouncerDah = Bounce2::Button();
  debouncerDit = Bounce2::Button();

#ifdef USE_HIGHRES_TIMER
  microTimer = Timer(MICROS);  // micro-second timer resolution
  config.pttHangTime *= 1000;
#else
  microTimer = Timer();  // milli-second timer resolution
#endif
}

void Keyer::setup() {
  updateTiming();

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
  Serial.println(F("AD9833 enabled."));

  toneGen.setWave(AD9833_OFF);
  toneGen.setFrequency(880.0, 0);
  toneGen.setFrequencyChannel(0);

  microTimer.start();
}

void Keyer::update() {
  unsigned long previousTime = currentTime;  // Store previous time
  currentTime = microTimer.read();

  // Detect rollover
  bool rollover = (currentTime < previousTime);

  // Handle rollover by adjusting currentTime
  if (rollover) {
    unsigned long rolloverAdjustment = (ULONG_MAX - previousTime) + currentTime + 1;
    currentTime += rolloverAdjustment;
  }
  
  debouncerDit.update();
  debouncerDah.update();

  bool ditState = !debouncerDit.read();
  bool dahState = !debouncerDah.read();
  bool iambicState = ditState && dahState;

  switch (currentState) {
    case IDLE:
      if (iambicState) {
        sendDit();  // Start with a dit in iambic mode
        currentState = IAMBIC_DIT;
      } else if (ditState) {
        sendDit();
        currentState = TRANSMITTING_DIT;
      } else if (dahState) {
        sendDah();
        currentState = TRANSMITTING_DAH;
      }
      break;

    case IAMBIC_DIT:
    case IAMBIC_DAH:
      if (currentTime >= transmissionEndTime) {
        toggleOutput(false);  // Ensure output is off before next element
        if (iambicState) {    // Check if still in iambic mode
          previousState = currentState;
          waitingEndTime = currentTime + elementSpace;  // Setup waiting end time
        }
        currentState = WAITING_ELEMENT_SPACE;  // Wait for element space
      }
      break;

    case WAITING_ELEMENT_SPACE:
      if (currentTime >= waitingEndTime) {
        if (iambicState) {
          if (previousState == IAMBIC_DIT) {
            currentState = IAMBIC_DAH;
            sendDah();
          } else {
            currentState = IAMBIC_DIT;
            sendDit();
          }
        } else {
          currentState = IDLE;
        }
      }
      break;

    case TRANSMITTING_DIT:
    case TRANSMITTING_DAH:
      if (currentTime >= transmissionEndTime) {
        toggleOutput(false);
        currentState = WAITING_ELEMENT_SPACE;
        waitingEndTime = currentTime + elementSpace;
      }
      break;

    case WAITING_CHARACTER_SPACE:
    case WAITING_WORD_SPACE:
      if (currentTime >= waitingEndTime) {
        currentState = IDLE;  // Transition back to IDLE after the space period
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

    digitalWrite(config.pttPin, HIGH);  // Assert PTT HIGH on transmission start
    transmissionStartTime = currentTime;
    pttTimerStarted = true;
    Serial.println(F("PTT: ON"));

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
        digitalWrite(config.pttPin, LOW);  // Turn off PTT after hang time
        pttTimerStarted = false;           // Reset flag
#ifdef USE_HIGHRES_TIMER
        float pttTime = (currentTime - transmissionStartTime) / 1000000.0;
#else if
        float pttTime = (currentTime - transmissionStartTime) / 1000.0;
#endif 
        sprintf(strBuffer, "PTT: OFF: (%.2fs)", pttTime);
        Serial.println(strBuffer);
    }
}

void Keyer::sendDit() {
  toggleOutput(true);
  transmissionEndTime = microTimer.read() + ditDuration;
  lastKeyEndTime = transmissionEndTime;
}

void Keyer::sendDah() {
  toggleOutput(true);
  transmissionEndTime = microTimer.read() + dahDuration;
  lastKeyEndTime = transmissionEndTime;
}

void Keyer::toggleOutput(bool state) {
  if (state)
  {
    beginTransmission();
  }
  digitalWrite(config.ledPin, state ? HIGH : LOW);
  digitalWrite(config.outputPin, state ? HIGH : LOW);
  toneGen.setWave(state ? AD9833_SINE : AD9833_OFF);
}

// these are approximate values for testing, a more robust "fist" can be achieved with some more work.
void Keyer::updateTiming() {
  ditDuration = WPM_RESOLUTION / wpm;                // Duration of a dit
  dahDuration = 3 * ditDuration;                     // Dah is three times the duration of dit
  elementSpace = ditDuration;                        // Space between elements
  characterSpace = 3 * ditDuration;                  // Space between characters
  wordSpace = (WPM_RESOLUTION / farnsworthWPM) * 7;  // Space between words
}

/// @brief returns true when keyer is ready for input (in IDLE state)
bool Keyer::isReadyForInput() const {
  return currentState == IDLE;  // Only consider ready if truly idle, not just between symbols
}

/// @brief Call only when keyer is ready for input (IDLE state)
bool Keyer::sendCharacterSpace() {
  if (!isReadyForInput()) {
    return false;
  }
  beginTransmission();
  toggleOutput(false);  // Ensure the output is off
  currentState = WAITING_CHARACTER_SPACE;
  waitingEndTime = microTimer.read() + characterSpace;
  return true;
}

/// @brief Call only when keyer is ready for input (IDLE state)
bool Keyer::sendWordSpace() {
  if (!isReadyForInput()) {
    return false;
  }
  beginTransmission();
  toggleOutput(false);  // Ensure the output is off
  currentState = WAITING_WORD_SPACE;
  waitingEndTime = microTimer.read() + wordSpace;
  return true;
}

/// @brief starts sending a dit. Call only when keyer is ready for input (IDLE state)
bool Keyer::triggerDit() {
  if (!isReadyForInput()) {
    return false;
  }
  sendDit();
  currentState = TRANSMITTING_DIT;  // Update state appropriately
  return true;
}

/// @brief starts sending a dah. Call only when keyer is ready for input (IDLE state)
bool Keyer::triggerDah() {
  if (!isReadyForInput()) {
    return false;
  }
  sendDah();
  currentState = TRANSMITTING_DAH;
  return true;
}

void Keyer::setWPM(int newWpm) {
  if (wpm != newWpm) {
    wpm = newWpm;
    farnsworthWPM = wpm - 5;  // Example adjustment for Farnsworth timing, needs more research
    updateTiming();
    Serial.print(F("WPM="));
    Serial.print(wpm);
    Serial.print(F(" Farnsworth="));
    Serial.println(farnsworthWPM);
  }
}

int Keyer::getWPM() const {
  return wpm;
}
