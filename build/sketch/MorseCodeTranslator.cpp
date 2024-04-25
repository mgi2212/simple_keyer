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
    if (textToTranslate.length() == 0)
    {
        return;
    }

    switch (currentState)
    {
        case TS_IDLE:
            if (currentCharIndex < textToTranslate.length())
            {
                currentState = TS_SENDING_CHARACTER;
            }
            else
            {
                // done sending
                textToTranslate = "";
                currentCharIndex = 0;
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
                symbolIndex = 0;
                currentState = TS_END_OF_CHARACTER;
            }
            else if (trySendSymbol(morse[symbolIndex]))
            {
                symbolIndex++;
            }
            break;

        case TS_END_OF_CHARACTER:
            if (trySendCharacterSpace())
            {
                currentState = TS_SENDING_CHARACTER_SPACE;
            }
            break;

        case TS_END_OF_WORD:
            if (trySendWordSpace())
            {
                currentState = TS_SENDING_WORD_SPACE;
            }
            break;

        case TS_SENDING_WORD_SPACE:
        case TS_SENDING_CHARACTER_SPACE:
            if (keyer.isReadyForInput())
            {
                currentCharIndex++;
                currentState = TS_IDLE;
            }
            break;

        default:
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

bool MorseCodeTranslator::trySendSymbol(char symbol)
{
    if (symbol == '.')
    {
        return keyer.triggerDit();
    }
    else if (symbol == '-')
    {
        return keyer.triggerDah();
    }
    else
    {
        return false;
    }
}

bool MorseCodeTranslator::trySendCharacterSpace()
{
    return keyer.sendCharacterSpace(); // Send space between characters
}

bool MorseCodeTranslator::trySendWordSpace()
{
    return keyer.sendWordSpace(); // Send space between characters
}
