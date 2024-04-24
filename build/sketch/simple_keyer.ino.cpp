#line 1 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino"
// N7HQ simple keyer
// April 2024

#include <Arduino.h>
#include <Bounce2.h>
#include "MorseCodeTranslator.h"
#include "Keyer.h"

#define SERIAL_BAUD 115200

const int dahPin = 2;
const int ditPin = 3;
const int keyerOutputPin = 10;
const int wpmSpeedPin = A0;

Keyer keyer(ditPin, dahPin, keyerOutputPin);
MorseCodeTranslator translator(keyer);

#line 19 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino"
void updateWPM();
#line 26 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino"
void setup();
#line 33 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino"
void loop();
#line 19 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino"
void updateWPM()
{
  int potValue = analogRead(wpmSpeedPin);
  int wpm = map(potValue, 0, 1023, 5, 40);
  keyer.setWPM(wpm);
}

void setup()
{
  Serial.begin(SERIAL_BAUD);
  pinMode(wpmSpeedPin, INPUT);
  keyer.setup();
}

void loop()
{

  keyer.update();

  updateWPM();

  if (Serial.available() > 0)
  {
    String input = Serial.readStringUntil('\n');
    translator.setText(input);
  }

  translator.update();
}

