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

struct MorseCodeMapping {
    const char* code;
    char character;
};


class MorseCodeTranslator
{
public:
    MorseCodeTranslator(Keyer &keyer);
    void setText(const String &text);
    void update();
    static const MorseCodeMapping morseMap[];
    static const int morseMapSize;

private:
    Keyer &keyer;
    String textToTranslate;
    bool isSending = false;
    int currentCharIndex = 0;
    int symbolIndex = 0;
    const char *morse;
    TranslatorState currentState;
    const char* getMorse(char c);
    char getChar(const String& morse); 
    bool trySendSymbol(char symbol);
    bool trySendCharacterSpace();
    bool trySendWordSpace();
};

#endif // MORSE_CODE_TRANSLATOR_H
