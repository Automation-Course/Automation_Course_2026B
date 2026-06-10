#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

Servo mosheServo;

// LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RFID pins
#define SS_PIN 10
#define RST_PIN 9
MFRC522 rfid(SS_PIN, RST_PIN);

// Authorized card UID: BC 49 D8 37
byte authorizedUID[4] = {0xBC, 0x49, 0xD8, 0x37};

// Pins
const int buttonPin = 2;
const int soundPin = 3;

const int greenLed = 4;
const int redLed = 5;

const int servoPin = 6;

const int trigPin = 7;
const int echoPin = 8;

// Thresholds
const int alertThreshold = 20;
const int minDistance = 2;
const int maxDistance = 400;

// Variables
long duration;
float distance;

bool lastButtonState = HIGH;
bool alertCleared = false;

volatile bool soundEvent = false;
bool noiseDetected = false;
unsigned long noiseUntil = 0;
const unsigned long noiseHoldTime = 2000; // Keeps noise alert for 2 seconds

int currentMode = -1;
// 0 = Party Mode
// 1 = Moshe Alert
// 2 = Noise Alert
// 3 = Alert Cleared

void setup()
{
  Serial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(soundPin, INPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  mosheServo.attach(servoPin);

  lcd.init();
  lcd.backlight();

  SPI.begin();
  rfid.PCD_Init();

  // Catch fast sound changes on D3
  attachInterrupt(digitalPinToInterrupt(soundPin), soundChanged, CHANGE);

  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);

  mosheServo.write(0);

  lcd.setCursor(0, 0);
  lcd.print("Moshe Alert");
  lcd.setCursor(0, 1);
  lcd.print("System Ready");

  Serial.println("Moshe Alert System Ready");
  Serial.println("RFID Ready");
  Serial.println("Sound sensor ready on D3");

  delay(2000);
  lcd.clear();
}

void loop()
{
  updateNoiseState();
  checkRFID();
  checkButton();

  distance = readDistance();

  if (distance == -1)
  {
    Serial.println("No valid echo - keeping last state");
  }
  else
  {
    bool mosheClose = (distance <= alertThreshold);
    bool dangerDetected = (mosheClose || noiseDetected);

    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.print(" cm | Sound pin: ");
    Serial.print(digitalRead(soundPin));
    Serial.print(" | Noise: ");
    Serial.print(noiseDetected);
    Serial.print(" | ");

    if (!dangerDetected)
    {
      // System is safe again
      alertCleared = false;
      partyMode(distance);
    }
    else
    {
      if (alertCleared)
      {
        clearedMode();
      }
      else if (mosheClose)
      {
        mosheAlertMode(distance);
      }
      else if (noiseDetected)
      {
        noiseAlertMode(distance);
      }
    }
  }

  delay(300);
}

void soundChanged()
{
  soundEvent = true;
}

void updateNoiseState()
{
  int soundState = digitalRead(soundPin);

  // If there was a quick change OR the sensor is currently HIGH,
  // we treat it as noise.
  if (soundEvent || soundState == HIGH)
  {
    soundEvent = false;
    noiseDetected = true;
    noiseUntil = millis() + noiseHoldTime;
  }

  if (millis() > noiseUntil)
  {
    noiseDetected = false;
  }
}

float readDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0)
  {
    return -1;
  }

  float measuredDistance = duration * 0.034 / 2;

  if (measuredDistance < minDistance || measuredDistance > maxDistance)
  {
    return -1;
  }

  return measuredDistance;
}

void partyMode(float d)
{
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);

  mosheServo.write(0);
  currentMode = 0;

  lcd.setCursor(0, 0);
  lcd.print("Party Mode      ");

  lcd.setCursor(0, 1);
  lcd.print("All Clear       ");

  Serial.println("Party Mode - all clear | Servo: 0");
}

void mosheAlertMode(float d)
{
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, HIGH);

  mosheServo.write(90);
  currentMode = 1;

  lcd.setCursor(0, 0);
  lcd.print("Moshe Alert!    ");

  lcd.setCursor(0, 1);
  lcd.print("Too Close       ");

  Serial.println("Moshe is too close | Servo: 90");
}

void noiseAlertMode(float d)
{
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, HIGH);

  mosheServo.write(90);
  currentMode = 2;

  lcd.setCursor(0, 0);
  lcd.print("Noise Alert!    ");

  lcd.setCursor(0, 1);
  lcd.print("High Sound      ");

  Serial.println("High noise detected | Servo: 90");
}

void clearedMode()
{
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, LOW);

  mosheServo.write(0);
  currentMode = 3;

  lcd.setCursor(0, 0);
  lcd.print("Alert Cleared   ");

  lcd.setCursor(0, 1);
  lcd.print("Authorized      ");

  Serial.println("Alert cleared by RFID | Servo: 0");
}

void checkRFID()
{
  if (!rfid.PICC_IsNewCardPresent())
  {
    return;
  }

  if (!rfid.PICC_ReadCardSerial())
  {
    return;
  }

  Serial.print("Card UID: ");

  for (byte i = 0; i < rfid.uid.size; i++)
  {
    if (rfid.uid.uidByte[i] < 0x10)
    {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }

  Serial.println();

  if (isAuthorizedCard())
  {
    Serial.println("Authorized RFID - Alert cleared");

    alertCleared = true;
    noiseDetected = false;
    soundEvent = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Alert Cleared");
    lcd.setCursor(0, 1);
    lcd.print("Authorized");

    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, LOW);

    mosheServo.write(0);
  }
  else
  {
    Serial.println("Unauthorized RFID - Alert still active");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Access Denied");
    lcd.setCursor(0, 1);
    lcd.print("Still Alert");

    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, HIGH);

    delay(1000);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

bool isAuthorizedCard()
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

void checkButton()
{
  bool buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && lastButtonState == HIGH)
  {
    Serial.println("Button Pressed - System Test");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Test");
    lcd.setCursor(0, 1);
    lcd.print("OK");

    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed, HIGH);

    mosheServo.write(45);
    delay(700);

    digitalWrite(redLed, LOW);

    if ((currentMode == 1 || currentMode == 2) && !alertCleared)
    {
      mosheServo.write(90);
    }
    else
    {
      mosheServo.write(0);
    }
  }

  lastButtonState = buttonState;
}