# ifndef PinConfig
# define PinConfig

/* KEYPAD */
char keys[4][4] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte keypadRowledGreen[4] = {5, 4, 3, 2};
byte keypadColledGreen[4] = {9, 8, 7, 6};
Keypad keypad = Keypad(makeKeymap(keys), keypadRowledGreen, keypadColledGreen, 4, 4);

const uint8_t locker1RelayPin = 22;
const uint8_t locker2RelayPin = 24;
const uint8_t locker3RelayPin = 26;
const uint8_t locker4RelayPin = 28;

const int LEDRed = 13;
const int LEDGreen = 12;
const int LEDBlue = 11;

# endif
