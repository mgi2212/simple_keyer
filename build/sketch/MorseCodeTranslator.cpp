#line 1 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\MorseCodeTranslator.cpp"
// N7HQ simple keyer
// April 2024

#include "MorseCodeTranslator.h"

const char *MorseCodeTranslator::morseCode[36] = {
    ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
    "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
    "..-", "...-", ".--", "-..-", "-.--", "--..",
    "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."};

MorseCodeTranslator::MorseCodeTranslator(Keyer &keyer) : keyer(keyer) {}

void MorseCodeTranslator::setText(const String &text)
{
    textToTranslate = text;
    textToTranslate.toUpperCase();
    currentCharIndex = 0;
    symbolIndex = 0;
    currentState = TS_IDLE;
    Serial.print(F("Sending: "));
    Serial.println(textToTranslate);
}

void MorseCodeTranslator::update()
{
    switch (currentState)
    {
    case TS_IDLE:
        if (currentCharIndex < textToTranslate.length())
        {
            currentState = TS_SENDING_CHARACTER;
        }
        break;
    case TS_SENDING_CHARACTER:
        if (textToTranslate[currentCharIndex] == ' ')
        {
            currentState = TS_END_OF_WORD;
        }
        else
        {
            morse = morseCodeForCharacter(textToTranslate[currentCharIndex]);
            symbolIndex = 0; // Reset symbol index for new character
            currentState = TS_SENDING_SYMBOL;
        }
        break;
    case TS_SENDING_SYMBOL:

        if (morse[symbolIndex] == '\0')
        {
            currentState = TS_END_OF_CHARACTER;
        }
        else if (keyer.isReadyForInput())
        {
            trySendSymbol(morse[symbolIndex++]);
        }
        break;

    case TS_END_OF_CHARACTER:
        symbolIndex = 0;
        trySendCharacterSpace();
        currentState = TS_SENDING_CHARACTER_SPACE;
        break;

    case TS_END_OF_WORD:
        trySendWordSpace();
        currentState = TS_SENDING_WORD_SPACE;
        break;

    case TS_SENDING_WORD_SPACE:
    case TS_SENDING_CHARACTER_SPACE:
        if (keyer.isReadyForInput())
        {
            currentCharIndex++;
            currentState = TS_IDLE;
        }
        break;
    }
}

const char *MorseCodeTranslator::morseCodeForCharacter(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return morseCode[c - 'A'];
    }
    else if (c >= '0' && c <= '9')
    {
        return morseCode[26 + (c - '0')];
    }
    return ""; // Return empty string for unsupported characters
}

void MorseCodeTranslator::trySendSymbol(char symbol)
{
    int retries = 10;
    while (retries > 0 && !keyer.isReadyForInput())
    {
        delay(1); // Delay to allow for keyer to become ready
        retries--;
    }
    if (keyer.isReadyForInput())
    {
        if (symbol == '.')
        {
            keyer.triggerDit();
        }
        else if (symbol == '-')
        {
            keyer.triggerDah();
        }
    }
}

void MorseCodeTranslator::trySendCharacterSpace()
{
    int retries = 10;
    while (retries > 0 && !keyer.isReadyForInput())
    {
        delay(1); // Delay to allow for keyer to become ready
        retries--;
    }
    if (keyer.isReadyForInput())
    {
        keyer.sendCharacterSpace(); // Send space between characters
    }
}

void MorseCodeTranslator::trySendWordSpace()
{
    int retries = 10;
    while (retries > 0 && !keyer.isReadyForInput())
    {
        delay(1); // Delay to allow for keyer to become ready
        retries--;
    }
    if (keyer.isReadyForInput())
    {
        keyer.sendWordSpace(); // Send space between characters
    }
}
