// N7HQ simple keyer
// April 2024

#include "MorseCodeTranslator.h"


MorseCodeTranslator::MorseCodeTranslator(Keyer &keyer) : keyer(keyer) { }

// Create an array of MorseCodeMapping with all the mappings
const MorseCodeMapping MorseCodeTranslator::morseMap[] = {
    {"-.-.--", '!'}, {".-..-.", '"'}, {"...-..-", '$'}, {".-...", '&'}, {".----.", '\''},
    {"-.--.", '('}, {"-.--.-", ')'}, {".-.-.", '+'}, {"--..--", ','}, {"-....-", '-'},
    {".-.-.-", '.'}, {"-..-.", '/'}, {"-----", '0'}, {".----", '1'}, {"..---", '2'},
    {"...--", '3'}, {"....-", '4'}, {".....", '5'}, {"-....", '6'}, {"--...", '7'},
    {"---..", '8'}, {"----.", '9'}, {"---...", ':'}, {"-.-.-.", ';'}, {"-...-", '='},
    {"..--..", '?'}, {".--.-.", '@'}, {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'},
    {"-..", 'D'}, {".", 'E'}, {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'},
    {".---", 'J'}, {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {"-.", 'N'}, {"---", 'O'},
    {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'}, {"..-", 'U'},
    {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'}, {"--..", 'Z'},
    {".-...", '['}, {"...-.-", ']'}, {"..--.", '^'}, {".--.-", '_'}, {"....-", '`'},
    {".-.-", 'Ä'}, {".--.-", 'Á'}, {"..-..", 'É'}, {"--.--", 'Ñ'}, {"---.", 'Ö'},
    {"..--", 'Ü'}, {"..._._", '<'}
};

const int MorseCodeTranslator::morseMapSize = sizeof(morseMap) / sizeof(morseMap[0]);

void MorseCodeTranslator::setText(const String &text)
{
    if (isSending)
    {
        Serial.println(F("Currently sending, cannot accept a new line."));
        return;
    }

    textToTranslate = text;
    textToTranslate.toUpperCase();
    if (textToTranslate.length() == 0) 
    {
        Serial.println(F("Nothing to send."));
        return;
    }

    isSending = true;
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
                // complete
                textToTranslate = "";
                currentCharIndex = 0;
                isSending = false;
                Serial.println(F("Sending complete."));
            }
            break;
        case TS_SENDING_CHARACTER:
            if (textToTranslate[currentCharIndex] == ' ')
            {
                currentState = TS_END_OF_WORD;
            }
            else
            {
                morse = getMorse(textToTranslate[currentCharIndex]);
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

char MorseCodeTranslator::getChar(const String& morse) {
    // Ensure the Morse code exists to avoid creating a default entry
    for (int i = 0; i < morseMapSize; i++) {
        if (morse == morseMap[i].code) {
            return morseMap[i].character;
        }
    }
    return '\0'; // Return null character if not found}
}

const char* MorseCodeTranslator::getMorse(char c) {
    for (int i = 0; i < morseMapSize; i++) {
        if (c == morseMap[i].character) {
            return morseMap[i].code;
        }
    }
    return ""; // Return empty string if character not found
}
