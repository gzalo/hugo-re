#include <Keyboard.h>

const int ROWS = 4;
const int COLS = 4;

const int rowPins[ROWS] = {5,4,3,2};
const int colPins[COLS] = {9,6,7,8};

// Keypad number HID keycodes (HID usage ID + 0x88)
#define KEY_KP_0 (0x62 + 0x88)
#define KEY_KP_1 (0x59 + 0x88)
#define KEY_KP_2 (0x5A + 0x88)
#define KEY_KP_3 (0x5B + 0x88)
#define KEY_KP_4 (0x5C + 0x88)
#define KEY_KP_5 (0x5D + 0x88)
#define KEY_KP_6 (0x5E + 0x88)
#define KEY_KP_7 (0x5F + 0x88)
#define KEY_KP_8 (0x60 + 0x88)
#define KEY_KP_9 (0x61 + 0x88)

// Phone keypad mapped to USB HID keys
// 1-3 -> KP 7-9, 4-6 -> KP 4-6, 7-9 -> KP 1-3
// * -> ESC, 0 -> KP 0, # -> Enter
// 4th column (A,B,C,D) -> q,w,e,r
const uint8_t keyMap[ROWS][COLS] = {
  {KEY_KP_7, KEY_KP_8, KEY_KP_9, 'q'},
  {KEY_KP_4, KEY_KP_5, KEY_KP_6, 'w'},
  {KEY_KP_1, KEY_KP_2, KEY_KP_3, 'e'},
  {KEY_ESC, KEY_KP_0, KEY_RETURN, 'r'}
};

const int hangUpPin = 10;

bool keyState[ROWS][COLS];
bool prevKeyState[ROWS][COLS];
bool hangUpState = false;
bool prevHangUpState = false;

void setup() {
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
  }
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], INPUT_PULLUP);
  }
  pinMode(hangUpPin, INPUT_PULLUP);

  memset(keyState, false, sizeof(keyState));
  memset(prevKeyState, false, sizeof(prevKeyState));

  Keyboard.begin();
}

void loop() {
  for (int r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);
    delayMicroseconds(50);

    for (int c = 0; c < COLS; c++) {
      keyState[r][c] = (digitalRead(colPins[c]) == LOW);

      if (keyState[r][c] && !prevKeyState[r][c]) {
        Keyboard.press(keyMap[r][c]);
      } else if (!keyState[r][c] && prevKeyState[r][c]) {
        Keyboard.release(keyMap[r][c]);
      }

      prevKeyState[r][c] = keyState[r][c];
    }

    digitalWrite(rowPins[r], HIGH);
  }

  // Hang up button: pin 10 shorted to GND when phone is down
  hangUpState = (digitalRead(hangUpPin) == LOW);
  if (hangUpState && !prevHangUpState) {
    Keyboard.press(KEY_ESC);
    Keyboard.release(KEY_ESC);
  }
  prevHangUpState = hangUpState;

  delay(10);
}
