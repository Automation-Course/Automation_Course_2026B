#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <VL53L1X.h>

#define SS_PIN 10
#define RST_PIN 3

// Pins definition
const int redLedPin = 6;
const int greenLedPin = 5;
const int buttonPin = 2;
const int servoPin = 9;
const int pirPin = 7;

// Authorized RFID tag and distance threshold
const String authorizedUID = "9A 1C C B0";
const int distanceThreshold = 200;

MFRC522 rfid(SS_PIN, RST_PIN);
Servo gateServo;
LiquidCrystal_I2C LCD(0x27, 16, 2);
VL53L1X distanceSensor;

// System state variables
bool rfidApproved = false;
bool gateOpen = false;
bool lastButtonState = LOW;
bool motionDetected = false;   // Saves PIR detection state

void setup() {
  Serial.begin(9600);

  pinMode(redLedPin, OUTPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(pirPin, INPUT);

  // Initialize RFID, distance sensor, servo and LCD
  SPI.begin();
  rfid.PCD_Init();

  Wire.begin();
  distanceSensor.setTimeout(500);

  if (!distanceSensor.init()) {
    Serial.println("Failed to detect distance sensor");
    while (1);
  }

  distanceSensor.startContinuous(50);

  gateServo.attach(servoPin);

  LCD.init();
  LCD.backlight();
  LCD.clear();

  // Set initial waiting state
  showWaitingState();

  Serial.println("Exodus Gate System started");
  Serial.println("Waiting for motion and RFID card...");
}

void loop() {
  checkResetButton();

  int motion = digitalRead(pirPin);

  // If motion is detected, ask user to scan RFID card
  if (motion == HIGH && !motionDetected && !rfidApproved && !gateOpen) {
    motionDetected = true;

    Serial.println("Motion detected near the Exodus gate");

    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("Motion detected");
    LCD.setCursor(0, 1);
    LCD.print("Scan RFID card");
  }

  // Check RFID card only after motion was detected
  if (motionDetected && !rfidApproved && !gateOpen) {
    checkRFID();
  }

  // After RFID approval, check distance before opening the gate
  if (rfidApproved && !gateOpen) {
    checkDistanceAndOpenGate();
  }
}

void checkResetButton() {
  int buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && lastButtonState == LOW) {
    closeGate();
    delay(250);
  }

  lastButtonState = buttonState;
}

void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String cardUID = "";

  // Build UID string from RFID card bytes
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (i > 0) {
      cardUID += " ";
    }

    cardUID += String(rfid.uid.uidByte[i], HEX);
  }

  cardUID.toUpperCase();

  Serial.print("UID: ");
  Serial.println(cardUID);

  // Check if the scanned RFID is authorized
  if (cardUID == authorizedUID) {
    rfidApproved = true;

    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, LOW);

    LCD.clear();
    LCD.setCursor(0, 0);
    LCD.print("RFID approved");
    LCD.setCursor(0, 1);
    LCD.print("Move to gate");

    Serial.println("RFID approved. Waiting for person near gate...");
  } else {
    denyAccess();
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  delay(1000);
}

void checkDistanceAndOpenGate() {
  int distance = distanceSensor.read();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  // Open gate only when person is close enough
  if (distance < distanceThreshold) {
    openGate();
  }

  delay(300);
}

void openGate() {
  gateOpen = true;

  digitalWrite(redLedPin, LOW);
  digitalWrite(greenLedPin, HIGH);

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Access approved");
  LCD.setCursor(0, 1);
  LCD.print("Sea is open");

  Serial.println("Access approved - opening the sea");

  gateServo.write(180);
  delay(700);
  gateServo.write(90);

  Serial.println("Sea opened");
}

void denyAccess() {
  rfidApproved = false;
  gateOpen = false;

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Access denied");
  LCD.setCursor(0, 1);
  LCD.print("Stay in Egypt");

  Serial.println("Access denied");

  delay(2000);

  showWaitingState();
}

void closeGate() {
  rfidApproved = false;
  gateOpen = false;

  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, HIGH);

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Gate closed");
  LCD.setCursor(0, 1);
  LCD.print("Waiting...");

  Serial.println("Reset pressed - closing gate");

  gateServo.write(0);
  delay(700);
  gateServo.write(90);

  showWaitingState();
}

void showWaitingState() {
  rfidApproved = false;
  gateOpen = false;
  motionDetected = false;

  // Initial system state
  digitalWrite(redLedPin, HIGH);
  digitalWrite(greenLedPin, LOW);

  gateServo.write(90);

  LCD.clear();
  LCD.setCursor(0, 0);
  LCD.print("Exodus Gate");
  LCD.setCursor(0, 1);
  LCD.print("Waiting...");
}