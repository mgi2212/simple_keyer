// N7HQ simple keyer
// April 2024

#ifndef Keyer_h
#define Keyer_h

#include <Arduino.h>
#include <Bounce2.h>
#include <Timer.h>

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

class Keyer
{
public:
    Keyer(int ditPin, int dahPin, int outputPin);
    void setup();
    void update();
    void sendCharacterSpace();
    void sendWordSpace();
    void setWPM(int wpm);
    int getWPM() const;
    void triggerDah();
    void triggerDit();
    bool isReadyForInput() const;

private:
    int ditPin, dahPin, outputPin;
    Bounce2::Button debouncerDit;
    Bounce2::Button debouncerDah;
    Timer microTimer;

    unsigned long transmissionEndTime;
    unsigned long waitingEndTime;
    unsigned long ditDuration;
    unsigned long dahDuration;
    unsigned long elementSpace;
    unsigned long characterSpace;
    unsigned long wordSpace;

    int wpm;           // Words per minute for Morse code transmission
    int farnsworthWPM; // Adjusted WPM for Farnsworth timing

    KeyerState currentState; // Current state of the state machine
    KeyerState previousState;

    void sendDit();
    void sendDah();
    void toggleOutput(bool state);
    void updateTiming();
    void handleIambicKeying();
};

#endif
