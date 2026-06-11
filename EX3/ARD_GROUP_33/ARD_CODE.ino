#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include <Servo.h>

SoftwareSerial mySerial(3, 4);
Adafruit_Fingerprint finger(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;

#define LED_GREEN        5
#define LED_RED          6
#define LED_YELLOW       7
#define LDR_PIN          A0
#define IR_PIN           11
#define SERVO_PIN        10
#define BTN_ENROLL       12
#define LIGHT_THRESHOLD  250
#define NUM_QUESTIONS    5

const char q0[] PROGMEM = "Locusts plague?";
const char q1[] PROGMEM = "Darkness plague";
const char q2[] PROGMEM = "Blood plague?";
const char q3[] PROGMEM = "Frogs plague?";
const char q4[] PROGMEM = "Firstborn plague";
const char* const questions[] PROGMEM = {q0, q1, q2, q3, q4};
const int answers[] = {8, 9, 1, 2, 10};
const int irCodes[] = {0x15, 0x09, 0x45, 0x46, 0x19};
char qBuffer[17];

void allLedsOff() {
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);
}

void dispenseManna() {
  allLedsOff();
  digitalWrite(LED_GREEN, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Manna incoming!"));
  lcd.setCursor(0, 1);
  lcd.print(F("Gate opening..."));
  Serial.println(F("STATE: Dispensing manna"));

  myServo.attach(SERVO_PIN);
  myServo.write(180);
  delay(1000);
  myServo.write(90);
  delay(1000);
  myServo.detach();

  delay(2000);
  allLedsOff();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Scan finger"));
  Serial.println(F("STATE: Idle"));
}

void rejectUser(const char* reason) {
  allLedsOff();
  digitalWrite(LED_RED, HIGH);
  Serial.print(F("STATE: Rejected - "));
  Serial.println(reason);

  delay(2000);
  allLedsOff();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Scan finger"));
  Serial.println(F("STATE: Idle"));
}

void enrollFail() {
  digitalWrite(LED_YELLOW, LOW);
  digitalWrite(LED_RED, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Enroll failed"));
  Serial.println(F("STATE: Enroll failed"));
  delay(2000);
  allLedsOff();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Scan finger"));
}

void enrollFinger() {
  allLedsOff();
  digitalWrite(LED_YELLOW, HIGH);   // busy indicator ON

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Please add"));
  lcd.setCursor(0, 1);
  lcd.print(F("fingerprint"));
  Serial.println(F("STATE: Enrollment started"));
  delay(1500);

  // First scan - wait until a finger is read
  while (finger.getImage() != FINGERPRINT_OK);
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    enrollFail();
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Remove finger"));
  Serial.println(F("STATE: Remove finger"));
  delay(2000);
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Place same"));
  lcd.setCursor(0, 1);
  lcd.print(F("finger again"));
  Serial.println(F("STATE: Place again"));

  // Second scan
  while (finger.getImage() != FINGERPRINT_OK);
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    enrollFail();
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    enrollFail();
    return;
  }

  // Store as ID #2 (keeps the original ID #1)
  if (finger.storeModel(2) == FINGERPRINT_OK) {
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Enroll success!"));
    Serial.println(F("STATE: Enrolled as ID #2"));
    delay(2000);
    allLedsOff();
  } else {
    enrollFail();
    return;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Scan finger"));
}

void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  delay(100);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(BTN_ENROLL, INPUT_PULLUP);
  allLedsOff();

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);
  randomSeed(analogRead(A1));

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Scan finger"));

  if (finger.verifyPassword()) {
    Serial.println(F("STATE: System ready"));
  } else {
    Serial.println(F("Sensor not found"));
    while (1);
  }
}

void loop() {
  int lightVal = analogRead(LDR_PIN);
  bool isDaytime = (lightVal < LIGHT_THRESHOLD);

  // Check enrollment button first
  if (digitalRead(BTN_ENROLL) == LOW) {
    enrollFinger();
    return;
  }

  // Always update display first
  lcd.clear();
  lcd.setCursor(0, 0);
  if (isDaytime) {
    lcd.print(F("Day: scan finger"));
  } else {
    lcd.print(F("Night: no manna"));
  }
  lcd.setCursor(0, 1);
  lcd.print(F("Light: "));
  lcd.print(lightVal);

  // Try to get fingerprint image
  int result = finger.getImage();
  if (result != FINGERPRINT_OK) {
    delay(300);
    return;
  }

  // Try to convert image - if this fails, no real finger present
  result = finger.image2Tz();
  if (result != FINGERPRINT_OK) {
    delay(300);
    return;
  }

  // Real finger confirmed - search for match
  result = finger.fingerSearch();

  if (result != FINGERPRINT_OK) {
    Serial.println(F("STATE: Unknown finger"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Unknown finger"));
    lcd.setCursor(0, 1);
    lcd.print(F("Access denied"));
    rejectUser("unknown finger");
    return;
  }

  Serial.println(F("STATE: Finger valid"));
  allLedsOff();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Welcome,"));
  lcd.setCursor(0, 1);
  lcd.print(F("Israelite!"));
  delay(1000);

  if (!isDaytime) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("No manna at"));
    lcd.setCursor(0, 1);
    lcd.print(F("night!"));
    rejectUser("no daylight");
    return;
  }

  int q = random(NUM_QUESTIONS);
  strcpy_P(qBuffer, (char*)pgm_read_word(&(questions[q])));

  Serial.print(F("STATE: Question: "));
  Serial.println(qBuffer);
  Serial.print(F("STATE: Answer: "));
  Serial.println(answers[q]);

  allLedsOff();
  digitalWrite(LED_YELLOW, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(qBuffer);
  lcd.setCursor(0, 1);
  lcd.print(F("Which # (1-10)?"));
  Serial.println(F("STATE: Waiting 10 sec"));

  long startTime = millis();
  bool answered = false;
  bool correct = false;

  IrReceiver.resume();

  while (millis() - startTime < 10000) {
    if (IrReceiver.decode()) {
      int irCode = IrReceiver.decodedIRData.command;
      IrReceiver.resume();
      if (irCode == 0) continue;
      if (irCode == irCodes[q]) {
        answered = true;
        correct = true;
      } else {
        answered = true;
        correct = false;
      }
      break;
    }
  }

  allLedsOff();

  if (answered && correct) {
    Serial.println(F("STATE: Correct!"));
    dispenseManna();
  } else if (answered && !correct) {
    Serial.println(F("STATE: Wrong answer"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Wrong answer!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Nice try, Goi!"));
    rejectUser("wrong answer");
  } else {
    Serial.println(F("STATE: Timeout"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Too slow!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Nice try, Goi!"));
    rejectUser("timeout");
  }
}