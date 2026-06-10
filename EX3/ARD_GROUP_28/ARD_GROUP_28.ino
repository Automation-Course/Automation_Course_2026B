/*
  Arduino Uno Project
  IR Remote + I2C LCD + Servo + Sound Sensor + Door Light Sensor + LEDs + Reset Button

  Logic:
  1. Start:
     - LEDs are OFF.
     - Servo does NOT move at startup.
     - LCD shows "Press Remote".
     - System waits for IR remote button.

  2. IR remote:
     - Buttons 1, 2, and 3 are authorized.
     - Any other valid button shows "Access Denied" and turns on the red LED.
     - Empty IR command 0x0 is ignored.

  3. Voice / sound command:
     - Wait up to 30 seconds.
     - Read the sound sensor every 80 ms.
     - If at least 3 out of 8 samples detect sound, the command is accepted.

  4. Door closing:
     - Servo gets a close command.
     - Servo is detached after a short time.
     - System waits until the door sensor says the door is closed.

  5. Pizza prep:
     - LCD shows "Pizza Prep".
     - Green LED is ON.
     - Wait 10 seconds.

  6. Pizza ready:
     - Servo gets an open command.
     - Servo is detached after a short time.
     - LCD shows "Pizza Ready".
     - Green LED blinks for 10 seconds.

  7. Reset:
     - System waits for reset button.
     - Pressing reset returns to the beginning.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.hpp>

// =====================
// Pin Settings
// =====================

const byte IR_RECEIVER_PIN = 5;      // IR receiver S pin
const byte SERVO_PIN = 6;            // Servo signal pin
const byte SOUND_PIN = 2;            // Sound sensor OUT pin
const byte DOOR_SENSOR_PIN = 3;      // Door light sensor DO pin
const byte RESET_BUTTON_PIN = 4;     // Reset button with INPUT_PULLUP

const byte RED_LED_PIN = 7;
const byte GREEN_LED_PIN = 8;

// =====================
// Servo Settings
// =====================

// Set to false if you want to test the system without moving the servo
const bool SERVO_ENABLED = true;

const int SERVO_CLOSED_ANGLE = 110;
const int SERVO_OPEN_ANGLE = 10;

// The servo receives signal only for this amount of time, then it is detached.
// If the servo does not move enough, increase this value.
// If the servo rotates too much, decrease this value.
const unsigned long SERVO_MOVE_MS = 1200;

// =====================
// IR Remote Settings
// =====================

// Your measured remote commands:
// Button 1 = 0x45
// Button 2 = 0x46
// Button 3 = 0x47
const uint16_t AUTHORIZED_IR_COMMAND_1 = 0x45;
const uint16_t AUTHORIZED_IR_COMMAND_2 = 0x46;
const uint16_t AUTHORIZED_IR_COMMAND_3 = 0x47;

// =====================
// LCD Settings
// =====================

const byte LCD_COLS = 16;
const byte LCD_ROWS = 2;
const byte LCD_DEFAULT_ADDRESS = 0x27;

// =====================
// Sensor Settings
// =====================

// Sound sensor:
// In most sound modules, OUT becomes HIGH when sound is above the threshold.
// If your sound sensor works the opposite way, change this to LOW.
const int SOUND_ACTIVE_STATE = HIGH;

// Door sensor:
// You said the light sensor shows that the door is closed when there is light.
// Therefore HIGH is currently defined as "door closed".
// If the behavior is opposite, change HIGH to LOW.
const int DOOR_CLOSED_STATE = HIGH;

// Door sensor must be stable for this time before accepting it as closed
const unsigned long DOOR_STABLE_MS = 300;

// =====================
// Timing Settings
// =====================

const unsigned long ACCESS_DENIED_MS = 5000;
const unsigned long VOICE_TIMEOUT_MS = 30000;

const unsigned long SOUND_SAMPLE_EVERY_MS = 80;
const byte SOUND_SAMPLES_PER_BATCH = 8;
const byte SOUND_REQUIRED_POSITIVES = 3;

const unsigned long TIMEOUT_MESSAGE_MS = 5000;
const unsigned long PIZZA_PREP_MS = 10000;
const unsigned long READY_BLINK_TOTAL_MS = 10000;
const unsigned long READY_BLINK_EVERY_MS = 500;

const unsigned long BUTTON_DEBOUNCE_MS = 50;

// =====================
// Objects
// =====================

Servo doorServo;
LiquidCrystal_I2C* lcd = nullptr;

// =====================
// State Machine
// =====================

enum SystemState {
  STATE_WAIT_IR,
  STATE_ACCESS_DENIED,
  STATE_WAIT_VOICE,
  STATE_TIMEOUT_MESSAGE,
  STATE_CLOSING_DOOR,
  STATE_PIZZA_PREP,
  STATE_PIZZA_READY_BLINK,
  STATE_WAIT_RESET
};

SystemState currentState = STATE_WAIT_IR;

// =====================
// Variables
// =====================

unsigned long stateStartMs = 0;

unsigned long lastSoundSampleMs = 0;
byte soundSampleCount = 0;
byte soundPositiveCount = 0;

unsigned long lastBlinkMs = 0;
bool greenBlinkState = false;

bool servoMoveActive = false;
unsigned long servoMoveStartMs = 0;

// =====================
// LCD Helper Functions
// =====================

byte findFirstI2CAddress() {
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      return address;
    }
  }

  return LCD_DEFAULT_ADDRESS;
}

void printLine(byte row, const char* text) {
  if (lcd == nullptr) return;

  lcd->setCursor(0, row);

  for (byte i = 0; i < LCD_COLS; i++) {
    lcd->print(' ');
  }

  lcd->setCursor(0, row);
  lcd->print(text);
}

void showMessage(const char* line1, const char* line2 = "") {
  if (lcd == nullptr) return;

  lcd->clear();
  printLine(0, line1);
  printLine(1, line2);
}

// =====================
// LED Functions
// =====================

void setLeds(bool redOn, bool greenOn) {
  digitalWrite(RED_LED_PIN, redOn ? HIGH : LOW);
  digitalWrite(GREEN_LED_PIN, greenOn ? HIGH : LOW);
}

// Startup test to check if the LEDs are wired correctly
void ledStartupTest() {
  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(1000);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  delay(300);

  digitalWrite(RED_LED_PIN, HIGH);
  delay(700);
  digitalWrite(RED_LED_PIN, LOW);

  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(700);
  digitalWrite(GREEN_LED_PIN, LOW);
}

// =====================
// Servo Functions
// =====================

void startServoMove(int angle) {
  if (!SERVO_ENABLED) {
    Serial.println(F("Servo command ignored because SERVO_ENABLED = false"));
    return;
  }

  if (!doorServo.attached()) {
    doorServo.attach(SERVO_PIN);
  }

  doorServo.write(angle);

  servoMoveStartMs = millis();
  servoMoveActive = true;

  Serial.print(F("Servo command angle: "));
  Serial.println(angle);
}

void updateServoDetach() {
  if (!servoMoveActive) {
    return;
  }

  if (millis() - servoMoveStartMs >= SERVO_MOVE_MS) {
    doorServo.detach();
    servoMoveActive = false;
    Serial.println(F("Servo detached"));
  }
}

// =====================
// Sensor Functions
// =====================

bool isSoundDetected() {
  return digitalRead(SOUND_PIN) == SOUND_ACTIVE_STATE;
}

bool isDoorClosedRaw() {
  return digitalRead(DOOR_SENSOR_PIN) == DOOR_CLOSED_STATE;
}

bool isDoorClosedStable() {
  static bool lastRawState = false;
  static bool stableState = false;
  static unsigned long lastChangeMs = 0;

  bool rawState = isDoorClosedRaw();

  if (rawState != lastRawState) {
    lastRawState = rawState;
    lastChangeMs = millis();
  }

  if (millis() - lastChangeMs >= DOOR_STABLE_MS) {
    stableState = rawState;
  }

  return stableState;
}

bool isResetButtonPressed() {
  static int lastRawState = HIGH;
  static int stableState = HIGH;
  static unsigned long lastChangeMs = 0;

  int rawState = digitalRead(RESET_BUTTON_PIN);

  if (rawState != lastRawState) {
    lastRawState = rawState;
    lastChangeMs = millis();
  }

  if (millis() - lastChangeMs >= BUTTON_DEBOUNCE_MS) {
    stableState = rawState;
  }

  return stableState == LOW; // INPUT_PULLUP: pressed = LOW
}

// =====================
// IR Functions
// =====================

bool isAuthorizedIrCommand(uint16_t command) {
  return command == AUTHORIZED_IR_COMMAND_1 ||
         command == AUTHORIZED_IR_COMMAND_2 ||
         command == AUTHORIZED_IR_COMMAND_3;
}

// =====================
// Sound Batch Functions
// =====================

void resetSoundBatch() {
  soundSampleCount = 0;
  soundPositiveCount = 0;
  lastSoundSampleMs = millis() - SOUND_SAMPLE_EVERY_MS;
}

// =====================
// State Change Function
// =====================

void setState(SystemState newState) {
  currentState = newState;
  stateStartMs = millis();

  switch (newState) {
    case STATE_WAIT_IR:
      // Important:
      // The servo does NOT move in the start state.
      // It only moves after a valid command later in the process.
      setLeds(false, false);
      showMessage("Press Remote", "Buttons 1 2 3");
      Serial.println(F("STATE: WAIT_IR"));
      break;

    case STATE_ACCESS_DENIED:
      setLeds(true, false);
      showMessage("Access Denied", "");
      Serial.println(F("STATE: ACCESS_DENIED"));
      break;

    case STATE_WAIT_VOICE:
      setLeds(false, false);
      showMessage("Name OK", "Say Order");
      resetSoundBatch();
      Serial.println(F("STATE: WAIT_VOICE"));
      break;

    case STATE_TIMEOUT_MESSAGE:
      setLeds(false, false);
      showMessage("Time Out", "");
      Serial.println(F("STATE: TIMEOUT_MESSAGE"));
      break;

    case STATE_CLOSING_DOOR:
      setLeds(false, false);
      showMessage("Closing", "The Door");

      // Servo moves only here to close the door
      startServoMove(SERVO_CLOSED_ANGLE);

      Serial.println(F("STATE: CLOSING_DOOR"));
      break;

    case STATE_PIZZA_PREP:
      setLeds(false, true);
      showMessage("Pizza Prep", "");
      Serial.println(F("STATE: PIZZA_PREP"));
      break;

    case STATE_PIZZA_READY_BLINK:
      showMessage("Pizza Ready", "Press Reset");

      // Servo moves only here to open the door
      startServoMove(SERVO_OPEN_ANGLE);

      greenBlinkState = false;
      lastBlinkMs = millis();

      digitalWrite(RED_LED_PIN, LOW);
      digitalWrite(GREEN_LED_PIN, LOW);

      Serial.println(F("STATE: PIZZA_READY_BLINK"));
      break;

    case STATE_WAIT_RESET:
      setLeds(false, false);
      showMessage("Pizza Ready", "Press Reset");
      Serial.println(F("STATE: WAIT_RESET"));
      break;
  }
}

// =====================
// State Handlers
// =====================

void handleWaitIr() {
  if (!IrReceiver.decode()) {
    return;
  }

  // Ignore repeat signals caused by holding a button
  if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
    IrReceiver.resume();
    return;
  }

  uint16_t command = IrReceiver.decodedIRData.command;

  // Ignore invalid / empty IR command.
  // Sometimes the receiver reads 0x0 before the real command.
  if (command == 0x00) {
    Serial.println(F("Ignored empty IR command: 0x0"));
    IrReceiver.resume();
    return;
  }

  Serial.print(F("IR command received: 0x"));
  Serial.println(command, HEX);

  if (isAuthorizedIrCommand(command)) {
    Serial.println(F("IR command authorized"));
    setState(STATE_WAIT_VOICE);
  } else {
    Serial.println(F("IR command denied"));
    setState(STATE_ACCESS_DENIED);
  }

  IrReceiver.resume();
}

void handleAccessDenied() {
  if (millis() - stateStartMs >= ACCESS_DENIED_MS) {
    setState(STATE_WAIT_IR);
  }
}

void handleWaitVoice() {
  unsigned long now = millis();

  if (now - stateStartMs >= VOICE_TIMEOUT_MS) {
    setState(STATE_TIMEOUT_MESSAGE);
    return;
  }

  if (now - lastSoundSampleMs >= SOUND_SAMPLE_EVERY_MS) {
    lastSoundSampleMs = now;

    soundSampleCount++;

    if (isSoundDetected()) {
      soundPositiveCount++;
    }

    Serial.print(F("Sound batch: "));
    Serial.print(soundPositiveCount);
    Serial.print(F(" / "));
    Serial.println(soundSampleCount);

    if (soundSampleCount >= SOUND_SAMPLES_PER_BATCH) {
      if (soundPositiveCount >= SOUND_REQUIRED_POSITIVES) {
        Serial.println(F("Voice command detected"));
        setState(STATE_CLOSING_DOOR);
      } else {
        resetSoundBatch();
      }
    }
  }
}

void handleTimeoutMessage() {
  if (millis() - stateStartMs >= TIMEOUT_MESSAGE_MS) {
    setState(STATE_WAIT_IR);
  }
}

void handleClosingDoor() {
  // Optional: allow reset button while waiting for the door
  if (isResetButtonPressed()) {
    setState(STATE_WAIT_IR);
    return;
  }

  if (isDoorClosedStable()) {
    Serial.println(F("Door closed detected"));
    setState(STATE_PIZZA_PREP);
  }
}

void handlePizzaPrep() {
  if (millis() - stateStartMs >= PIZZA_PREP_MS) {
    setState(STATE_PIZZA_READY_BLINK);
  }
}

void handlePizzaReadyBlink() {
  unsigned long now = millis();

  // Allows pressing reset during the blinking period
  if (isResetButtonPressed()) {
    setState(STATE_WAIT_IR);
    return;
  }

  if (now - lastBlinkMs >= READY_BLINK_EVERY_MS) {
    lastBlinkMs = now;
    greenBlinkState = !greenBlinkState;
    digitalWrite(GREEN_LED_PIN, greenBlinkState ? HIGH : LOW);
  }

  if (now - stateStartMs >= READY_BLINK_TOTAL_MS) {
    setState(STATE_WAIT_RESET);
  }
}

void handleWaitReset() {
  if (isResetButtonPressed()) {
    setState(STATE_WAIT_IR);
  }
}

// =====================
// setup
// =====================

void setup() {
  Serial.begin(9600);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);

  pinMode(SOUND_PIN, INPUT);
  pinMode(DOOR_SENSOR_PIN, INPUT);

  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);

  setLeds(false, false);

  // LED test at startup
  ledStartupTest();

  Wire.begin();

  byte detectedLcdAddress = findFirstI2CAddress();

  Serial.print(F("LCD I2C address detected: 0x"));
  Serial.println(detectedLcdAddress, HEX);

  lcd = new LiquidCrystal_I2C(detectedLcdAddress, LCD_COLS, LCD_ROWS);
  lcd->init();
  lcd->backlight();

  IrReceiver.begin(IR_RECEIVER_PIN, ENABLE_LED_FEEDBACK);

  Serial.println(F("System started"));
  Serial.println(F("IR mode enabled"));
  Serial.println(F("Authorized buttons: 1, 2, 3"));
  Serial.println(F("Servo will not move at startup"));

  setState(STATE_WAIT_IR);
}

// =====================
// loop
// =====================

void loop() {
  updateServoDetach();

  switch (currentState) {
    case STATE_WAIT_IR:
      handleWaitIr();
      break;

    case STATE_ACCESS_DENIED:
      handleAccessDenied();
      break;

    case STATE_WAIT_VOICE:
      handleWaitVoice();
      break;

    case STATE_TIMEOUT_MESSAGE:
      handleTimeoutMessage();
      break;

    case STATE_CLOSING_DOOR:
      handleClosingDoor();
      break;

    case STATE_PIZZA_PREP:
      handlePizzaPrep();
      break;

    case STATE_PIZZA_READY_BLINK:
      handlePizzaReadyBlink();
      break;

    case STATE_WAIT_RESET:
      handleWaitReset();
      break;
  }
}