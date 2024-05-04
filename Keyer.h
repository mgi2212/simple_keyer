/***********************************************************************
 * File: keyer.h
 * Author: Dan Quigley, N7HQ
 * Date: April 2024
 *
 * Description:
 *     This file contains the implementation of a Morse code keyer for
 *     use in amateur radio communications. The keyer supports both
 *     iambic keying and straight key use. It includes debounce logic
 *     for key presses and can be configured for different words per
 *     minute (WPM) speeds, including Farnsworth timing adjustments.
 *
 * Usage:
 *     Compile with Arduino compiler and upload to an Arduino board.
 *     Connect the appropriate pins to a Morse code key and a speaker
 *     or other output device to receive audio feedback.
 *
 * Dependencies:
 *     Requires Arduino.h for basic Arduino functions and types.
 *     Bounce2 library for debouncing key inputs.
 *     Timer library for managing timing events.
 *     MD_AD9833 library for generating audio tone outputs via SPI.
 *     SPI library for communication.
 *
 * Revisions:
 *     1.0 - Initial release.
 *
 * Notes:
 *     This code is for demonstration purposes only and is not optimized
 *     for production use. It is intended as a functional prototype for
 *     testing and educational purposes.
 ***********************************************************************/

#ifndef Keyer_h
#define Keyer_h

#include <Arduino.h>
#include <AD9833.h>
#include <Bounce2.h>

//#define DEBUG_OUTPUT 1

#define SIDETONE_FREQUENCY 1000
#define INVERT_WPM true   // allows for idiots (like me) that wire the pot backwards
#define NUM_READINGS 10   // Number of samples for debouncing wpm pot

// Define the states of the state machine
enum KeyerState
{
    IDLE,
    TRANSMITTING_DIT,
    TRANSMITTING_DAH,
    WAITING_ELEMENT_SPACE,
    IAMBIC_DIT,
    IAMBIC_DAH,
    WAITING_CHARACTER_SPACE,
    WAITING_WORD_SPACE
};

struct KeyerConfig
{
    int ditPin;
    int dahPin;
    int outputPin;
    int pttPin;
    int ledPin;
    int pttHangTime;
    int wpmSpeedPin;
};

class Keyer
{
public:
    Keyer(KeyerConfig &config, AD9833 &toneGen);
    void setup();
    void update();
    bool sendCharacterSpace();
    bool sendWordSpace();
    void setWPM(int wpm);
    int getWPM() const;
    bool triggerDah();
    bool triggerDit();
    bool isReadyForInput() const;

private:
    KeyerConfig &config;
    AD9833 &toneGen;
    Bounce2::Button debouncerDit;
    Bounce2::Button debouncerDah;

    unsigned long currentTime;
    unsigned long transmissionStartTime;
    unsigned long transmissionEndTime;
    unsigned long lastKeyEndTime;
    unsigned long waitingEndTime;
    unsigned long ditDuration;
    unsigned long dahDuration;
    unsigned long elementSpace;
    unsigned long characterSpace;
    unsigned long wordSpace;
    bool pttTimerStarted;

#ifdef DEBUG_OUTPUT
    char strBuffer[256]; // Buffer to hold formatted strings
#endif    

    int wpm;           // Words per minute for Morse code transmission
    int farnsworthWPM; // Adjusted WPM for Farnsworth timing

    KeyerState currentState; // Current state of the state machine
    KeyerState previousState;

    void sendDit();
    void sendDah();
    void toggleOutput(bool state);
    void updateTiming();
    void handleIambicKeying();
    void beginTransmission();
    void checkEndTransmission();
    void updateWPM();
};

#endif
