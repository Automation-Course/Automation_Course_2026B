#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

// LCD I2C
// If the LCD does not show text, try changing 0x27 to 0x3F
LiquidCrystal_I2C lcd_1(0x27, 16, 2);

// Servo
Servo gateServo;

// RFID RC522 pins
#define RFID_SS_PIN 10
#define RFID_RST_PIN A2

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// Authorized RFID UID: 6A 53 1B B1
byte authorizedUID[4] = {0x6A, 0x53, 0x1B, 0xB1};

// Fingerprint sensor pins
// Sensor TX -> Arduino D2
// Sensor RX -> Arduino D3
SoftwareSerial fingerSerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// Pins
const int ACTIVATOR_BUTTON_PIN = 4;
const int LED_PIN = 5;
const int SERVO_PIN = 8;
const int LDR_PIN = A0;

// Button logic
const int BUTTON_PRESSED = LOW;

// Thresholds
const int LDR_THRESHOLD = 500;

// Continuous Servo control
// 90 is stop, values below and above 90 rotate in opposite directions
const int SERVO_STOP = 90;
const int SERVO_OPEN_SPEED = 70;
const int SERVO_CLOSE_SPEED = 110;

// Separate movement times
const unsigned long SERVO_OPEN_MOVE_TIME = 700;
const unsigned long SERVO_CLOSE_MOVE_TIME = 805;

// Gate open waiting times
const unsigned long DAY_OPEN_TIME = 3000;     // Cloud mode
const unsigned long NIGHT_OPEN_TIME = 5000;   // Fire mode

// States
const int STATE_IDLE = 0;
const int STATE_IDENTIFIED = 1;
const int STATE_VERIFICATION = 2;
const int STATE_OPEN_GATE = 3;
const int STATE_DENIED = 4;

int currentState = STATE_IDLE;

unsigned long stateStartTime = 0;

bool stateJustChanged = true;
bool nightMode = false;
bool lastNightMode = false;

int ledBrightness = 80;
unsigned long gateOpenTime = DAY_OPEN_TIME;

void setup()
{
  Serial.begin(9600);

  pinMode(ACTIVATOR_BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  gateServo.attach(SERVO_PIN);
  gateServo.write(SERVO_STOP);

  lcd_1.init();
  lcd_1.backlight();

  SPI.begin();
  rfid.PCD_Init();

  finger.begin(57600);

  analogWrite(LED_PIN, 0);

  Serial.println("System started");
  Serial.println("Servo stopped at startup");
  Serial.println("RFID ready");

  if (finger.verifyPassword())
  {
    Serial.println("Fingerprint sensor ready");
  }
  else
  {
    Serial.println("Fingerprint sensor NOT found");

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Finger sensor");
    lcd_1.setCursor(0, 1);
    lcd_1.print("NOT found");

    while (1)
    {
      delay(1000);
    }
  }

  changeState(STATE_IDLE);
}

void loop()
{
  if (currentState == STATE_IDLE)
  {
    handleIdle();
  }
  else if (currentState == STATE_IDENTIFIED)
  {
    handleIdentified();
  }
  else if (currentState == STATE_VERIFICATION)
  {
    handleVerification();
  }
  else if (currentState == STATE_OPEN_GATE)
  {
    handleOpenGate();
  }
  else if (currentState == STATE_DENIED)
  {
    handleDenied();
  }
}

bool buttonPressed(int pin)
{
  return digitalRead(pin) == BUTTON_PRESSED;
}

void changeState(int newState)
{
  currentState = newState;
  stateStartTime = millis();
  stateJustChanged = true;

  Serial.print("State changed to: ");
  Serial.println(stateName(newState));
}

String stateName(int state)
{
  if (state == STATE_IDLE)
  {
    return "IDLE";
  }
  else if (state == STATE_IDENTIFIED)
  {
    return "IDENTIFIED";
  }
  else if (state == STATE_VERIFICATION)
  {
    return "VERIFICATION";
  }
  else if (state == STATE_OPEN_GATE)
  {
    return "OPEN_GATE";
  }
  else if (state == STATE_DENIED)
  {
    return "DENIED";
  }

  return "UNKNOWN";
}

void readEnvironment()
{
  int ldrValue = analogRead(LDR_PIN);

  // Reversed real LDR logic:
  // In your real circuit, higher value means darker.
  if (ldrValue > LDR_THRESHOLD)
  {
    nightMode = true;
    ledBrightness = 255;
    gateOpenTime = NIGHT_OPEN_TIME;
  }
  else
  {
    nightMode = false;
    ledBrightness = 80;
    gateOpenTime = DAY_OPEN_TIME;
  }
}

void showIdleScreen()
{
  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Scan RFID");

  lcd_1.setCursor(0, 1);

  if (nightMode)
  {
    lcd_1.print("Pillar of Fire");
  }
  else
  {
    lcd_1.print("Pillar Cloud");
  }
}

void handleIdle()
{
  readEnvironment();

  gateServo.write(SERVO_STOP);
  analogWrite(LED_PIN, 0);

  if (stateJustChanged || nightMode != lastNightMode)
  {
    showIdleScreen();

    Serial.print("LDR value: ");
    Serial.println(analogRead(LDR_PIN));

    if (nightMode)
    {
      Serial.println("Mode: Pillar of Fire");
    }
    else
    {
      Serial.println("Mode: Pillar of Cloud");
    }

    lastNightMode = nightMode;
    stateJustChanged = false;
  }

  int rfidResult = readRFID();

  if (rfidResult == 1)
  {
    Serial.println("RFID accepted");
    changeState(STATE_IDENTIFIED);
    return;
  }
  else if (rfidResult == -1)
  {
    Serial.println("RFID denied");
    changeState(STATE_DENIED);
    return;
  }
}

void handleIdentified()
{
  if (stateJustChanged)
  {
    gateServo.write(SERVO_STOP);
    analogWrite(LED_PIN, 0);

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("RFID Accepted");

    lcd_1.setCursor(0, 1);
    lcd_1.print("Press button");

    Serial.println("Waiting for operator approval");
    stateJustChanged = false;
  }

  if (buttonPressed(ACTIVATOR_BUTTON_PIN))
  {
    Serial.println("Operator button pressed");
    changeState(STATE_VERIFICATION);
    return;
  }
}

void handleVerification()
{
  if (stateJustChanged)
  {
    gateServo.write(SERVO_STOP);

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("Place finger");

    lcd_1.setCursor(0, 1);
    lcd_1.print("Hold steady");

    Serial.println("Waiting for fingerprint");
    stateJustChanged = false;
  }

  int fingerResult = readFingerprint();

  if (fingerResult == 0)
  {
    return;
  }

  if (fingerResult == 1)
  {
    Serial.println("Fingerprint accepted");
    changeState(STATE_OPEN_GATE);
    return;
  }

  if (fingerResult == -1)
  {
    Serial.println("Fingerprint denied");
    changeState(STATE_DENIED);
    return;
  }
}

void handleOpenGate()
{
  if (stateJustChanged)
  {
    readEnvironment();

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("PASS NOW");

    lcd_1.setCursor(0, 1);

    if (nightMode)
    {
      lcd_1.print("Fire mode 5s");
    }
    else
    {
      lcd_1.print("Cloud mode 3s");
    }

    Serial.println("Opening gate");

    analogWrite(LED_PIN, ledBrightness);

    // Rotate to open position
    gateServo.write(SERVO_OPEN_SPEED);
    delay(SERVO_OPEN_MOVE_TIME);
    gateServo.write(SERVO_STOP);

    Serial.println("Gate opened");
    Serial.print("Waiting open for: ");
    Serial.println(gateOpenTime);

    delay(gateOpenTime);

    Serial.println("Closing gate");

    // Rotate back to closed position
    gateServo.write(SERVO_CLOSE_SPEED);
    delay(SERVO_CLOSE_MOVE_TIME);
    gateServo.write(SERVO_STOP);

    analogWrite(LED_PIN, 0);

    Serial.println("Gate closed");

    waitForButtonRelease();
    waitForRFIDRemoval();

    changeState(STATE_IDLE);
    return;
  }
}

void handleDenied()
{
  if (stateJustChanged)
  {
    gateServo.write(SERVO_STOP);

    lcd_1.clear();
    lcd_1.setCursor(0, 0);
    lcd_1.print("DENIED");

    lcd_1.setCursor(0, 1);
    lcd_1.print("Try again");

    Serial.println("Access denied");

    for (int i = 0; i < 6; i++)
    {
      digitalWrite(LED_PIN, HIGH);
      delay(250);
      digitalWrite(LED_PIN, LOW);
      delay(250);
    }

    waitForButtonRelease();
    waitForRFIDRemoval();

    changeState(STATE_IDLE);
    return;
  }
}

int readRFID()
{
  if (!rfid.PICC_IsNewCardPresent())
  {
    return 0;
  }

  if (!rfid.PICC_ReadCardSerial())
  {
    return 0;
  }

  Serial.print("Scanned UID: ");

  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
    {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1)
    {
      Serial.print(" ");
    }
  }

  Serial.println();

  bool authorized = isAuthorizedUID();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (authorized)
  {
    return 1;
  }

  return -1;
}

bool isAuthorizedUID()
{
  if (rfid.uid.size != 4)
  {
    return false;
  }

  for (byte i = 0; i < 4; i++)
  {
    if (rfid.uid.uidByte[i] != authorizedUID[i])
    {
      return false;
    }
  }

  return true;
}

int readFingerprint()
{
  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_NOFINGER)
  {
    return 0;
  }

  if (p != FINGERPRINT_OK)
  {
    return 0;
  }

  delay(300);

  p = finger.image2Tz();

  if (p != FINGERPRINT_OK)
  {
    return 0;
  }

  Serial.println("Fingerprint converted");

  p = finger.fingerSearch();

  if (p == FINGERPRINT_OK)
  {
    Serial.print("Found finger ID: ");
    Serial.println(finger.fingerID);

    Serial.print("Confidence: ");
    Serial.println(finger.confidence);

    return 1;
  }

  if (p == FINGERPRINT_NOTFOUND)
  {
    Serial.println("Fingerprint not found");
    return -1;
  }

  return 0;
}

void waitForButtonRelease()
{
  while (buttonPressed(ACTIVATOR_BUTTON_PIN))
  {
    delay(20);
  }
}

void waitForRFIDRemoval()
{
  Serial.println("Remove RFID tag");

  lcd_1.clear();
  lcd_1.setCursor(0, 0);
  lcd_1.print("Remove RFID");
  lcd_1.setCursor(0, 1);
  lcd_1.print("Resetting...");

  delay(1500);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}