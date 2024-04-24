// N7HQ simple keyer
// April 2024

#ifndef MORSE_CODE_TRANSLATOR_H
#define MORSE_CODE_TRANSLATOR_H

#include <Arduino.h>
#include "Keyer.h"

enum TranslatorState
{
    TS_IDLE,
    TS_SENDING_CHARACTER,
    TS_SENDING_SYMBOL,
    TS_END_OF_CHARACTER,
    TS_SENDING_CHARACTER_SPACE,
    TS_END_OF_WORD,
    TS_SENDING_WORD_SPACE
};

class MorseCodeTranslator
{
public:
    MorseCodeTranslator(Keyer &keyer);
    void setText(const String &text);
    void update();

private:
    Keyer &keyer;
    String textToTranslate;
    int currentCharIndex = 0;
    int symbolIndex = 0;
    const char *morse;
    TranslatorState currentState;
    static const char *morseCode[36];
    const char *morseCodeForCharacter(char c);
    void sendCurrentSymbol(const char *morse);
    void trySendSymbol(char symbol);
    void trySendCharacterSpace();
    void trySendWordSpace();
};

#endif // MORSE_CODE_TRANSLATOR_H
