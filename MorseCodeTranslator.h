/***********************************************************************
 * File: MorseCodeTranslator.h
 * Author: Dan Quigley, N7HQ
 * Date: April 2024
 *
 * Description:
 *     Defines the MorseCodeTranslator class that translates text into Morse code
 *     using a linked Keyer class to control signal output. It handles character
 *     and symbol translation, manages Morse code timing, and tracks translation
 *     state.
 *
 * Usage:
 *     Include in projects that require text to Morse code conversion. The translator
 *     needs an instance of the Simple Keyer to function.
 *
 * Dependencies:
 *     - Arduino.h: Basic Arduino library.
 *     - Keyer.h: Manages Morse code keying.
 *
 * Revisions:
 *     1.0 - Initial release.
 *
 * Notes:
 *     Designed for simple Morse code applications, supporting essential translation
 *     from text to Morse code with basic timing control.
 ***********************************************************************/

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

struct MorseCodeMapping
{
    const char *code;
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
    const char *getMorse(char c);
    char getChar(const String &morse);
    bool trySendSymbol(char symbol);
    bool trySendCharacterSpace();
    bool trySendWordSpace();
};

#endif // MORSE_CODE_TRANSLATOR_H
