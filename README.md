# N7HQ Simple Keyer and Morse Code Translator

This project includes a super simple Morse code keyer and text->morse translator, designed to be used with Arduino-based systems. It features adjustable speed input, keying for DIT and DAH signals, and translation capabilities to convert plain text to Morse code.

## Features

- **Keyer**: Manages the keying for Morse code using DIT and DAH inputs with adjustable speeds.
- **Morse Code Translator**: Converts plain text into Morse code, handling the encoding in real-time.
- **Adjustable WPM**: The words per minute (WPM) can be adjusted via a potentiometer or an analog input.
- **Iambic Keying**: Supports iambic keying modes, including handling simultaneous DIT and DAH presses.
- **Debounced Inputs**: Implements debouncing for all input signals to ensure clean transitions.

## Components

- `Keyer.cpp` and `Keyer.h`: Implements the Morse code keying logic.
- `MorseCodeTranslator.cpp` and `MorseCodeTranslator.h`: Handles the translation of text to Morse code.
- `simple_keyer.ino`: Arduino sketch that integrates the keyer and translator with hardware setup.

## Hardware Requirements

- Arduino board (Uno, Mega, etc.)
- Input buttons for DIT and DAH
- Output pin for keying signal
- Analog input for WPM adjustment

## Installation

1. Clone this repository to your local machine.
2. Open the `simple_keyer.ino` file with the Arduino IDE.
3. Connect the hardware according to the schematic provided in the hardware section.
4. Upload the sketch to your Arduino board.

## Configuration

- **DIT and DAH Pins**: Configure the input pins in `simple_keyer.ino` based on your hardware setup.
- **Output Pin**: Set the output pin for the keying signal.
- **WPM Adjustment**: Adjust the WPM through the analog input pin mapped in the code.

## Usage

1. Power on the Arduino.
2. Use the DIT and DAH buttons to input Morse code manually.
3. Adjust the WPM as needed to match your transmission or practice speed.
4. Send text through the serial monitor to see it translated and keyed out in Morse code.

## Contributing

Contributions to this project are welcome. Please fork the repository and submit a pull request with your enhancements.
