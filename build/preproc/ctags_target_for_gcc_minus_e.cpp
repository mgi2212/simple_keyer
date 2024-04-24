# 1 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino"
// N7HQ simple keyer
// April 2024

# 5 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino" 2
# 6 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino" 2
# 7 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino" 2
# 8 "C:\\Users\\danq\\OneDrive\\Documents\\Arduino\\simple_keyer\\simple_keyer.ino" 2



const int dahPin = 2;
const int ditPin = 3;
const int keyerOutputPin = 10;
const int wpmSpeedPin = A0;

Keyer keyer(ditPin, dahPin, keyerOutputPin);
MorseCodeTranslator translator(keyer);

void updateWPM()
{
  int potValue = analogRead(wpmSpeedPin);
  int wpm = map(potValue, 0, 1023, 5, 40);
  keyer.setWPM(wpm);
}

void setup()
{
  Serial.begin(115200);
  pinMode(wpmSpeedPin, 0x0);
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
