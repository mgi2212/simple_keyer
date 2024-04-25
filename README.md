# N7HQ Super Simple Keyer and Morse Code Translator

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

## Libraries Used

This project requires the following Arduino libraries:

- **Bounce2**: Used for debouncing button inputs. This library provides stable, reliable button state readings.
- **Timer**: Required for handling precise timing operations. Ensures that Morse code timings are accurate to the specification.

### Installing Libraries

To install these libraries, follow these steps in the Arduino IDE:

1. Open the Arduino IDE.
2. Go to **Sketch** > **Include Library** > **Manage Libraries**.
3. In the Library Manager, enter "Bounce2" in the search bar, find the Bounce2 library in the list, and click **Install**.
4. Search for "Timer" in the Library Manager, find the Timer library, and click **Install**.

Ensure you have these libraries installed before compiling and uploading the sketch to your Arduino board.

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

### Wiring Details:


```
[Arduino]                  [Other Components]
+--------+                 +------------------+
|        |                 | 10k-ohm          |
|        |                 | Linear Pot       |
|        +----(A0)---------+ Center           |
|        |                 |                  |
|        |                 +--(GND)-----------+
|        |                 |                  |
|        |                 +--(VCC)-----------+
|        |                 |                  |
|        |                 | Button           |
|        +----(D2)---------+ Dah              |
|        |                 |                  |
|        |                 | Button           |
|        +----(D3)---------+ Dit              |
|        |                 |                  |
|        |                 |                  |
|        |                 |                  |
|        +----(D10)--------+ Keyer Output     |
|        |                 |                  |
+--------+                 +------------------+
```
- **Potentiometer for Speed Control (WPM):**
  - Connect the center pin of the potentiometer to analog pin `A0` on the Arduino. This pin reads the voltage level to determine the speed (WPM).
  - Connect one side of the potentiometer to the ground (`GND`).
  - Connect the other side to a positive voltage (`VCC`), typically 5V or 3.3V depending on your Arduino model.

- **Buttons for Dah and Dit:**
  - Connect the `Dah` button between digital pin `D2` and ground. When pressed, it will close the circuit and bring `D2` to LOW.
  - Connect the `Dit` button between digital pin `D3` and ground. Similar to the `Dah` button, it closes the circuit and brings `D3` to LOW when pressed.

- **Keyer Output:**
  - Connect an output pin, in this case, digital pin `D10`, to the circuit component that acts as your keyer output. This could be an LED, a sound module, or a relay to simulate the keying.

### Additional Notes:
- Ensure all connections are secure.
- Use pull-up or pull-down resistors on the buttons if required by your specific Arduino model to ensure stable readings.
- Modify the pin assignments in your code if you choose different pins than those listed here.

This diagram provides a clear layout for setting up your hardware components. Adjust the configuration as needed based on your specific requirements and hardware variations.

## Contributing

Contributions to this project are welcome. Please fork the repository and submit a pull request with your enhancements.
