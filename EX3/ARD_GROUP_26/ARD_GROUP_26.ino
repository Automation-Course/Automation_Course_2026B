#include <IRremote.h>
#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>

// ---------- IR Remote - Moses Approval ----------
#define IR_PIN 11
#define OK_BUTTON_CODE 0x1C

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

#define SERVO_STOP 89
#define SERVO_OPEN 180
#define SERVO_CLOSE 0

// זמני פתיחה/סגירה
const int OPEN_TIME = 500;
const int CLOSE_TIME = 500;

// כמה זמן השער נשאר פתוח
const unsigned long GATE_OPEN_DURATION = 5000;

// ---------- Light Sensor ----------
const int DARK_THRESHOLD = 500;

// ---------- System states ----------
enum State {
  WAIT_FOR_REMOTE,
  WAIT_FOR_FINGER,
  SEA_OPEN
};

State state = WAIT_FOR_REMOTE;

unsigned long fingerStateStartTime = 0;
const unsigned long FINGER_GRACE_TIME = 2000;

unsigned long seaOpenStartTime = 0;

unsigned long remoteStateStartTime = 0;
const unsigned long REMOTE_GRACE_TIME = 700;

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
  delay(1000);

  lcd.init();
  lcd.backlight();

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK);

  finger.begin(57600);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("System Loading"));
  lcd.setCursor(0, 1);
  lcd.print(F("Please wait"));

  Serial.println(F("System loading..."));

  if (finger.verifyPassword()) {
    Serial.println(F("Fingerprint sensor found!"));
  } else {
    Serial.println(F("Fingerprint sensor NOT found."));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Finger Error"));
    lcd.setCursor(0, 1);
    lcd.print(F("Check wiring"));
    while (1);
  }

  delay(1500);
  resetToRemote();
}

void loop() {
  checkLightSensor();

  if (state == WAIT_FOR_REMOTE) {
    checkRemote();
  }

  else if (state == WAIT_FOR_FINGER) {
    // הכפתור פעיל רק אחרי שהשער נסגר אוטומטית
    if (buttonEnabledAfterAutoClose && digitalRead(CLOSE_BUTTON) == LOW) {
      delay(80);

      if (digitalRead(CLOSE_BUTTON) == LOW) {
        while (digitalRead(CLOSE_BUTTON) == LOW) {
          delay(10);
        }

        resetToRemoteByButton();
        return;
      }
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

  // עמוד האש נדלק כשחושך
  if (lightValue > DARK_THRESHOLD) {
    digitalWrite(LED_YELLOW, HIGH);
  } else {
    digitalWrite(LED_YELLOW, LOW);
  }
}

// ---------- IR Remote - Moses Approval ----------

void checkRemote() {
  if (millis() - remoteStateStartTime < REMOTE_GRACE_TIME) {
    if (IrReceiver.decode()) {
      IrReceiver.resume();
    }
    return;
  }

  if (IrReceiver.decode()) {
    byte command = IrReceiver.decodedIRData.command;

    Serial.print(F("Button code: 0x"));
    Serial.println(command, HEX);

    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      IrReceiver.resume();
      return;
    }

    IrReceiver.resume();

    if (command == OK_BUTTON_CODE) {
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, LOW);

      buttonEnabledAfterAutoClose = false;

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Moses approved"));
      lcd.setCursor(0, 1);
      lcd.print(F("Staff approved"));

      Serial.println(F("Moses approved"));

      delay(1500);

      prepareFingerScan();
    }
  }
}

// ---------- Fingerprint ----------

void prepareFingerScan() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Scan Finger"));
  lcd.setCursor(0, 1);
  lcd.print(F("Place finger"));

  Serial.println(F("Waiting for fingerprint..."));

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
  lcd.print(F("Scan Finger     "));
  lcd.setCursor(0, 1);
  lcd.print(F("Place finger    "));

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
    lcd.print(F("Welcome to"));
    lcd.setCursor(0, 1);
    lcd.print(F("Canaan!"));

    Serial.print(F("Fingerprint approved. ID: "));
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
  lcd.print(F("Access Denied"));
  lcd.setCursor(0, 1);
  lcd.print(F("Try again"));

  Serial.println(F("Access denied: Finger denied"));

  delay(3000);

  digitalWrite(LED_RED, LOW);

  // אחרי טביעה לא נכונה נשארים בשלב האצבע
  prepareFingerScan();
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
  lcd.print(F("Gate Open"));
  lcd.setCursor(0, 1);
  lcd.print(F("5 sec..."));

  Serial.println(F("Access granted - Red Sea opened"));

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
  lcd.print(F("Gate Closed"));
  lcd.setCursor(0, 1);
  lcd.print(F("Scan finger"));

  Serial.println(F("Gate auto closed - scan next finger"));

  delay(1500);

  buttonEnabledAfterAutoClose = true;

  prepareFingerScan();
}

// ---------- Reset by button ----------

void resetToRemoteByButton() {
  redSeaServo.write(SERVO_STOP);

  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Reset by Moses"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press OK"));

  Serial.println(F("Button pressed after gate closed - reset to remote step"));

  delay(1500);

  resetToRemote();
}

// ---------- Reset ----------

void resetToRemote() {
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  buttonEnabledAfterAutoClose = false;

  redSeaServo.write(SERVO_STOP);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Moses approve?"));
  lcd.setCursor(0, 1);
  lcd.print(F("Press OK"));

  Serial.println(F("System reset - Wait for OK"));

  remoteStateStartTime = millis();

  if (IrReceiver.decode()) {
    IrReceiver.resume();
  }

  state = WAIT_FOR_REMOTE;
}