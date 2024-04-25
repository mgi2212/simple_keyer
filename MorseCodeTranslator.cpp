// N7HQ simple keyer
// April 2024

#include "MorseCodeTranslator.h"

MorseCodeTranslator::MorseCodeTranslator(Keyer &keyer) : keyer(keyer) {}

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
                morse = charToMorse(textToTranslate[currentCharIndex]);
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

// Function to translate characters to Morse code
const char* MorseCodeTranslator::charToMorse(char c) 
{
    switch (c) 
    {
        case '!': return "-.-.--";   // exclamation mark
        case '"': return ".-..-.";   // RR
        case '$': return "...-..-";  // SX
        case '&': return ".-...";    // ampersand
        case '\'': return ".----.";  // DN
        case '(': return "-.--.";    // KN
        case ')': return "-.--.-";   // KK
        case '+': return ".-.-.";    // AR
        case ',': return "--..--";   // comma
        case '-': return "-....-";   // hyphen
        case '.': return ".-.-.-";   // period
        case '/': return "-..-.";    // DN
        case '0': return "-----";
        case '1': return ".----";
        case '2': return "..---";
        case '3': return "...--";
        case '4': return "....-";
        case '5': return ".....";
        case '6': return "-....";
        case '7': return "--...";
        case '8': return "---..";
        case '9': return "----.";
        case ':': return "---...";   // colon
        case ';': return "-.-.-.";   // AA
        case '=': return "-...-";    // BT
        case '?': return "..--..";   // ?
        case '@': return ".--.-.";   // AC
        case 'A': return ".-";
        case 'B': return "-...";
        case 'C': return "-.-.";
        case 'D': return "-..";
        case 'E': return ".";
        case 'F': return "..-.";
        case 'G': return "--.";
        case 'H': return "....";
        case 'I': return "..";
        case 'J': return ".---";
        case 'K': return "-.-";
        case 'L': return ".-..";
        case 'M': return "--";
        case 'N': return "-.";
        case 'O': return "---";
        case 'P': return ".--.";
        case 'Q': return "--.-";
        case 'R': return ".-.";
        case 'S': return "...";
        case 'T': return "-";
        case 'U': return "..-";
        case 'V': return "...-";
        case 'W': return ".--";
        case 'X': return "-..-";
        case 'Y': return "-.--";
        case 'Z': return "--..";
        case '[': return ".-...";   // AS
        case ']': return "...-.-";  // SK
        case '^': return "..--.";   // caret
        case '_': return ".--.-";   // underscore
        case '`': return "....-";   // alternative SK
        case 'Ä': return ".-.-";    // Ä
        case 'Á': return ".--.-";   // Á
        case 'É': return "..-..";   // É
        case 'Ñ': return "--.--";   // Ñ
        case 'Ö': return "---.";    // Ö
        case 'Ü': return "..--";    // Ü
        case '<': return "..._._";  // SK
        default: return "";         // default case for unsupported characters
    }
}
