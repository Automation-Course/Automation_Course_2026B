#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include "HX711.h"
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>

// ================= LCD =================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ================= RFID =================
#define RFID_SS_PIN 10
#define RFID_RST_PIN 9
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// ================= SERVO 360 =================
#define SERVO_PIN 7
Servo seaServo;

const int SERVO_STOP = 90;
const int SERVO_OPEN_DIR = 180;
const int SERVO_CLOSE_DIR = 0;

const unsigned long SERVO_MOVE_TIME_MS = 700;
const unsigned long GATE_OPEN_TIME_MS = 5000;

// ================= ULTRASONIC HC-SR04 =================
#define TRIG_PIN A0
#define ECHO_PIN A1

// ================= HX711 WEIGHT SENSOR =================
#define HX711_DT_PIN 3
#define HX711_SCK_PIN 2
HX711 scale;

float calibrationFactor = 1000.0;

// ================= LED + BUTTON =================
#define LED_PIN 5
#define BUTTON_PIN 4

// ================= SYSTEM SETTINGS =================
const float PERSON_DISTANCE_CM = 12.0;
const int REQUIRED_DETECTIONS = 4;
const float MATZAH_THRESHOLD_G = 2.0;

// ================= GENERAL FUNCTIONS =================

void showMessage(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2.0;
}

bool personIsReallyDetected() {
  int count = 0;

  for (int i = 0; i < REQUIRED_DETECTIONS; i++) {
    float d = readDistanceCm();

    Serial.print("Distance check ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(d);
    Serial.println(" cm");

    if (d > 0 && d <= PERSON_DISTANCE_CM) {
      count++;
    }

    delay(150);
  }

  return count == REQUIRED_DETECTIONS;
}

// ================= RFID FUNCTIONS =================

String getRfidUid() {
  String uidString = "";

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      uidString += "0";
    }

    uidString += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      uidString += " ";
    }
  }

  uidString.toUpperCase();
  return uidString;
}

bool waitForRfidCard() {
  Serial.println("Waiting for RFID card...");
  showMessage("Show RFID", "Tap card now");

  unsigned long lastPrintTime = 0;

  while (true) {
    if (millis() - lastPrintTime >= 1000) {
      Serial.println("Still waiting for RFID card...");
      lastPrintTime = millis();
    }

    // אין כרטיס חדש ליד הקורא
    if (!rfid.PICC_IsNewCardPresent()) {
      delay(50);
      continue;
    }

    Serial.println("RFID card detected near reader");

    // יש כרטיס, אבל לא הצלחנו לקרוא את המספר שלו
    if (!rfid.PICC_ReadCardSerial()) {
      Serial.println("RFID read failed, try again");
      showMessage("RFID failed", "Try again");
      delay(1000);
      showMessage("Show RFID", "Tap card now");
      continue;
    }

    // קריאת UID
    String uid = getRfidUid();

    Serial.print("RFID UID: ");
    Serial.println(uid);

    // בשלב זה אנחנו מאשרים כל כרטיס
    Serial.println("RFID accepted");

    showMessage("RFID accepted", "Place Matzah");
    delay(1500);

    // סיום תקשורת עם הכרטיס הנוכחי
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return true;
  }
}

// ================= BUTTON + WEIGHT =================

bool isButtonPressed() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(40);

    if (digitalRead(BUTTON_PIN) == LOW) {
      while (digitalRead(BUTTON_PIN) == LOW) {
        delay(10);
      }

      return true;
    }
  }

  return false;
}

float readWeightGrams() {
  float weight = scale.get_units(10);

  if (weight < 0) {
    weight = -weight;
  }

  return weight;
}

// ================= SERVO 360 FUNCTIONS =================

void stopServo() {
  seaServo.write(SERVO_STOP);
  delay(100);
}

void openGate360() {
  Serial.println("Servo opening direction");
  seaServo.write(SERVO_OPEN_DIR);
  delay(SERVO_MOVE_TIME_MS);
  stopServo();
}

void closeGate360() {
  Serial.println("Servo closing direction");
  seaServo.write(SERVO_CLOSE_DIR);
  delay(SERVO_MOVE_TIME_MS);
  stopServo();
}

void openAndCloseGate() {
  Serial.println("Enough Matzah for crossing");
  Serial.println("Opening sea gate");

  showMessage("Enough Matzah", "Gate opens");

  digitalWrite(LED_PIN, HIGH);

  openGate360();

  Serial.println("Gate is open");
  showMessage("Gate is open", "Pass safely");

  delay(GATE_OPEN_TIME_MS);

  Serial.println("Closing sea gate");
  showMessage("Gate closing", "Please wait");

  closeGate360();

  digitalWrite(LED_PIN, LOW);

  showMessage("Gate closed", "System reset");

  Serial.println("Gate closed");
  Serial.println("System reset");

  delay(2000);
}

void resetToStandby() {
  stopServo();
  digitalWrite(LED_PIN, LOW);

  showMessage("MatzahGate", "Come closer");

  Serial.println("Returning to standby mode");
  Serial.println("Waiting for person near the gate");
}

// ================= SETUP =================

void setup() {
  Serial.begin(9600);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  digitalWrite(LED_PIN, LOW);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  lcd.init();
  lcd.backlight();

  showMessage("MatzahGate", "Starting...");
  Serial.println("System starting...");

  seaServo.attach(SERVO_PIN);
  stopServo();
  Serial.println("Servo 360 initialized on D7 and stopped");

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID initialized");

  scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
  scale.set_scale(calibrationFactor);

  showMessage("Remove weight", "Taring...");
  Serial.println("Initializing HX711");
  Serial.println("Remove all weight from scale");
  delay(3000);

  if (!scale.is_ready()) {
    showMessage("HX711 Error", "Check wiring");
    Serial.println("HX711 is NOT ready - check wiring");

    while (true) {
      delay(100);
    }
  }

  scale.tare();

  Serial.println("HX711 initialized");
  Serial.println("Tare complete");

  delay(1000);

  resetToStandby();
}

// ================= LOOP =================

void loop() {
  showMessage("MatzahGate", "Come closer");

  Serial.println("Standby: checking distance...");

  if (!personIsReallyDetected()) {
    delay(500);
    return;
  }

  Serial.println("Person detected near gate");

  showMessage("Person detected", "Show RFID");
  delay(700);

  // כאן מתבצע שלב העברת הכרטיס
  bool rfidOk = waitForRfidCard();

  if (!rfidOk) {
    Serial.println("RFID was not accepted");
    showMessage("RFID Error", "Try again");
    delay(1500);
    resetToStandby();
    return;
  }

  // אחרי שהכרטיס עבר בהצלחה — ממשיכים למשקל
  showMessage("Place Matzah", "Press button");
  Serial.println("Waiting for button press to measure weight");

  while (true) {
    if (isButtonPressed()) {
      Serial.println("Button pressed");
      Serial.println("Measuring weight...");

      float weightGrams = readWeightGrams();

      Serial.print("Weight measured: ");
      Serial.print(weightGrams, 1);
      Serial.println(" g");

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Weight:");
      lcd.print(weightGrams, 1);
      lcd.print("g");

      delay(1200);

      if (weightGrams > MATZAH_THRESHOLD_G) {
        Serial.println("Weight is above threshold");
        Serial.println("Enough Matzah");

        openAndCloseGate();
        resetToStandby();

        break;
      } else {
        Serial.println("Weight is not enough");
        Serial.println("Add Matzah and press button again");

        showMessage("Add Matzah", "Press again");
        delay(1200);

        showMessage("Place Matzah", "Press button");
      }
    }

    delay(50);
  }

  delay(500);
}