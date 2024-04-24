// N7HQ simple keyer
// April 2024

#include "Keyer.h"

// comment out for milli-second timer
#define USE_HIGHRES_TIMER 1

#ifdef USE_HIGHRES_TIMER
    #define WPM_RESOLUTION 1200000
#else
    #define WPM_RESOLUTION 1200
#endif

Keyer::Keyer(int ditPin, int dahPin, int outputPin) : ditPin(ditPin), dahPin(dahPin), outputPin(outputPin), wpm(20), farnsworthWPM(15), currentState(IDLE)
{
    debouncerDah = Bounce2::Button();
    debouncerDit = Bounce2::Button();

#ifdef USE_HIGHRES_TIMER
    microTimer = Timer(MICROS); // micro-second timer resolution
#else    
    microTimer = Timer();       // milli-second timer resolution
#endif

}

void Keyer::setup()
{
    updateTiming();

    pinMode(ditPin, INPUT_PULLUP);
    pinMode(dahPin, INPUT_PULLUP);
    pinMode(outputPin, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);

    digitalWrite(outputPin, LOW);
    digitalWrite(LED_BUILTIN, LOW);

    debouncerDit.attach(ditPin, INPUT_PULLUP);
    debouncerDah.attach(dahPin, INPUT_PULLUP);

    debouncerDit.interval(5);
    debouncerDah.interval(5);
    microTimer.start();
}

void Keyer::update()
{
    unsigned long currentTime = (microTimer.state() != PAUSED) ? microTimer.read() : 0;
    
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
        else
        {
            microTimer.pause();
        }
        break;

    case IAMBIC_DIT:
    case IAMBIC_DAH:
        if (currentTime >= transmissionEndTime)
        {
            toggleOutput(false); // Ensure output is off before next element
            if (iambicState)
            { // Check if still in iambic mode
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
}

void Keyer::sendDit()
{
    toggleOutput(true);
    transmissionEndTime = microTimer.read() + ditDuration;
}

void Keyer::sendDah()
{
    toggleOutput(true);
    transmissionEndTime = microTimer.read() + dahDuration;
}

void Keyer::toggleOutput(bool state)
{
    if (state)
    {
        microTimer.resume();
    }
    digitalWrite(outputPin, state ? HIGH : LOW);
    digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
}

// these are approximate values for testing, a more robust "fist" can be achieved with some more work.
void Keyer::updateTiming()
{
    ditDuration = 1200000 / wpm;               // Duration of a dit
    dahDuration = 3 * ditDuration;          // Dah is three times the duration of dit
    elementSpace = ditDuration;             // Space between elements
    characterSpace = 3 * ditDuration;       // Space between characters
    wordSpace = (1200000 / farnsworthWPM) * 7; // Space between words
}

void Keyer::sendCharacterSpace()
{
    noInterrupts(); // Disable interrupts to ensure atomicity
    // Transition to character space state only if appropriate
    if (currentState == IDLE || currentState == WAITING_ELEMENT_SPACE)
    {
        toggleOutput(false); // Ensure the output is off
        currentState = WAITING_CHARACTER_SPACE;
        waitingEndTime = microTimer.read() + characterSpace;
    }
    interrupts(); // Re-enable interrupts
}

void Keyer::sendWordSpace()
{
    noInterrupts();
    // Transition to word space state only if appropriate
    if (currentState == IDLE || currentState == WAITING_ELEMENT_SPACE)
    {
        toggleOutput(false); // Ensure the output is off
        currentState = WAITING_WORD_SPACE;
        waitingEndTime = microTimer.read() + wordSpace;
    }
    interrupts();
}

bool Keyer::isReadyForInput() const
{

    return currentState == IDLE; // Only consider ready if truly idle, not just between symbols
}

void Keyer::triggerDit()
{
    if (isReadyForInput()) // Ensures that the keyer is idle and ready
    {
        sendDit();
        currentState = TRANSMITTING_DIT; // Update state appropriately
    }
}

void Keyer::triggerDah()
{
    if (isReadyForInput())
    {
        sendDah();
        currentState = TRANSMITTING_DAH;
    }
}

void Keyer::setWPM(int newWpm)
{
    if (wpm != newWpm)
    {
        wpm = newWpm;
        farnsworthWPM = wpm - 5; // Example adjustment for Farnsworth timing, needs more research
        updateTiming();
        Serial.print(F("WPM="));
        Serial.print(wpm);
        Serial.print(F(" Farnsworth="));
        Serial.println(farnsworthWPM);
    }
}

int Keyer::getWPM() const
{
    return wpm;
}
