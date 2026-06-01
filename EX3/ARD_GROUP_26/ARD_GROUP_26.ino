#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// ---------- RFID - Moses Staff ----------
#define RFID_SS_PIN 10
#define RFID_RST_PIN 4

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// true = כל תג RFID מתקבל לבדיקה
// false = רק UID ספציפי מתקבל
bool TRUST_ANY_RFID = true;

byte MOSES_UID[] = {0xDE, 0xAD, 0xBE, 0xEF};
byte MOSES_UID_SIZE = 4;

// ---------- Fingerprint ----------
#define FINGER_RX 3
#define FINGER_TX 2

SoftwareSerial fingerSerial(FINGER_RX, FINGER_TX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Pins ----------
#define LED_GREEN A1
#define LED_RED A2
#define LED_YELLOW A3

#define SERVO_PIN 9
#define CLOSE_BUTTON 8
#define LDR_PIN A0

// ---------- Servo 360 ----------
Servo redSeaServo;

#define SERVO_STOP 90
#define SERVO_OPEN 180
#define SERVO_CLOSE 0

// זמני פתיחה/סגירה
const int OPEN_TIME = 600;
const int CLOSE_TIME = 700;

// כמה זמן השער נשאר פתוח
const unsigned long GATE_OPEN_DURATION = 5000;

// ---------- Light Sensor ----------
const int DARK_THRESHOLD = 500;

// ---------- System states ----------
enum State {
  WAIT_FOR_RFID,
  WAIT_FOR_FINGER,
  SEA_OPEN
};

State state = WAIT_FOR_RFID;

unsigned long fingerStateStartTime = 0;
const unsigned long FINGER_GRACE_TIME = 2000;

unsigned long seaOpenStartTime = 0;

// רק אחרי שהשער נסגר אוטומטית הכפתור פעיל
bool buttonEnabledAfterAutoClose = false;

void setup() {
  Serial.begin(9600);

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(CLOSE_BUTTON, INPUT_PULLUP);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_YELLOW, LOW);

  redSeaServo.attach(SERVO_PIN);
  redSeaServo.write(SERVO_STOP);

  lcd.init();
  lcd.backlight();

  SPI.begin();
  rfid.PCD_Init();

  finger.begin(57600);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Loading");
  lcd.setCursor(0, 1);
  lcd.print("Please wait");

  Serial.println("System loading...");

  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor NOT found.");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Finger Error");
    lcd.setCursor(0, 1);
    lcd.print("Check wiring");
    while (1);
  }

  delay(1500);
  resetToRFID();
}

void loop() {
  checkLightSensor();

  if (state == WAIT_FOR_RFID) {
    checkRFID();
  }

  else if (state == WAIT_FOR_FINGER) {
    // הכפתור פעיל רק אחרי שהשער נסגר אוטומטית
    if (buttonEnabledAfterAutoClose && digitalRead(CLOSE_BUTTON) == LOW) {
      resetToRFIDByButton();
      return;
    }

    checkFingerprintBeforeOpen();
  }

  else if (state == SEA_OPEN) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);

    // בזמן שהשער פתוח הכפתור לא עושה כלום
    if (millis() - seaOpenStartTime >= GATE_OPEN_DURATION) {
      autoCloseSea();
    }
  }
}

// ---------- Light Sensor / Pillar of Fire ----------

void checkLightSensor() {
  int lightValue = analogRead(LDR_PIN);

  Serial.print("Light value: ");
  Serial.println(lightValue);

  // עמוד האש נדלק כשחושך
  if (lightValue > DARK_THRESHOLD) {
    digitalWrite(LED_YELLOW, HIGH);
  } else {
    digitalWrite(LED_YELLOW, LOW);
  }
}

// ---------- RFID - Moses Staff ----------

void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print("RFID UID: ");
  printUID();

  bool validRFID = TRUST_ANY_RFID || isMosesUID();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

  if (validRFID) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);

    buttonEnabledAfterAutoClose = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hello Moses");
    lcd.setCursor(0, 1);
    lcd.print("Staff approved");

    Serial.println("Moses staff approved");

    delay(1500);

    prepareFingerScan();
  } else {
    denyRFID();
  }
}

bool isMosesUID() {
  if (rfid.uid.size != MOSES_UID_SIZE) {
    return false;
  }

  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] != MOSES_UID[i]) {
      return false;
    }
  }

  return true;
}

void printUID() {
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print("0x");

    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }

    Serial.print(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      Serial.print(", ");
    }
  }

  Serial.println();
}

void resetRFIDReader() {
  rfid.PCD_Init();
  delay(200);
}

// ---------- Fingerprint ----------

void prepareFingerScan() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Finger");
  lcd.setCursor(0, 1);
  lcd.print("Place finger");

  Serial.println("Waiting for fingerprint...");

  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  fingerStateStartTime = millis();
  state = WAIT_FOR_FINGER;
}

void checkFingerprintBeforeOpen() {
  if (millis() - fingerStateStartTime < FINGER_GRACE_TIME) {
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("Scan Finger     ");
  lcd.setCursor(0, 1);
  lcd.print("Place finger    ");

  int fingerID = getFingerprintID();

  if (fingerID == -1) {
    return;
  }

  if (fingerID > 0) {
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);

    buttonEnabledAfterAutoClose = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Welcome to");
    lcd.setCursor(0, 1);
    lcd.print("Canaan!");

    Serial.print("Fingerprint approved. ID: ");
    Serial.println(fingerID);

    delay(1500);
    openRedSea();
  }

  else if (fingerID == -2) {
    denyFingerprintBeforeOpen();
  }
}

int getFingerprintID() {
  uint8_t p = finger.getImage();

  if (p == FINGERPRINT_NOFINGER) {
    return -1;
  }

  if (p != FINGERPRINT_OK) {
    return -1;
  }

  p = finger.image2Tz();

  if (p != FINGERPRINT_OK) {
    return -1;
  }

  p = finger.fingerFastSearch();

  if (p == FINGERPRINT_OK) {
    return finger.fingerID;
  }

  if (p == FINGERPRINT_NOTFOUND) {
    return -2;
  }

  return -1;
}

// ---------- Access Denied ----------

void denyFingerprintBeforeOpen() {
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  redSeaServo.write(SERVO_STOP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied");
  lcd.setCursor(0, 1);
  lcd.print("Try again");

  Serial.println("Access denied: Finger denied");

  delay(3000);

  digitalWrite(LED_RED, LOW);

  // אחרי טביעה לא נכונה נשארים בשלב האצבע
  prepareFingerScan();
}

void denyRFID() {
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);

  redSeaServo.write(SERVO_STOP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wrong Staff");
  lcd.setCursor(0, 1);
  lcd.print("Access Denied");

  Serial.println("Access denied: Wrong staff");

  delay(3000);

  digitalWrite(LED_RED, LOW);

  resetToRFID();
}

// ---------- Red Sea ----------

void openRedSea() {
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_RED, LOW);

  // Servo 360: פתיחה לזמן קצר ואז עצירה
  redSeaServo.write(SERVO_OPEN);
  delay(OPEN_TIME);
  redSeaServo.write(SERVO_STOP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gate Open");
  lcd.setCursor(0, 1);
  lcd.print("5 sec...");

  Serial.println("Access granted - Red Sea opened");

  seaOpenStartTime = millis();
  state = SEA_OPEN;
}

void autoCloseSea() {
  // סגירה אוטומטית אחרי 5 שניות
  redSeaServo.write(SERVO_CLOSE);
  delay(CLOSE_TIME);
  redSeaServo.write(SERVO_STOP);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Gate Closed");
  lcd.setCursor(0, 1);
  lcd.print("Scan finger");

  Serial.println("Gate auto closed - scan next finger");

  delay(1500);

  // עכשיו הכפתור פעיל, ורק עכשיו לחיצה תחזיר ל-RFID
  buttonEnabledAfterAutoClose = true;

  prepareFingerScan();
}

// ---------- Reset by button ----------

void resetToRFIDByButton() {
  redSeaServo.write(SERVO_STOP);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reset by Moses");
  lcd.setCursor(0, 1);
  lcd.print("Scan staff");

  Serial.println("Button pressed after gate closed - reset to RFID step");

  delay(1500);

  resetToRFID();
}

// ---------- Reset ----------

void resetToRFID() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  buttonEnabledAfterAutoClose = false;

  redSeaServo.write(SERVO_STOP);

  resetRFIDReader();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Moses");
  lcd.setCursor(0, 1);
  lcd.print("Staff");

  Serial.println("System reset - Scan Moses Staff");

  state = WAIT_FOR_RFID;
}