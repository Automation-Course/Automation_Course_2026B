#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// --- הגדרות פינים ---
const int BUTTON_PIN = 2;
const int FINGERPRINT_TX = 3;
const int RFID_RST = 4;
const int TRIG_PIN = 5;
const int FINGERPRINT_RX = 6;
const int ECHO_PIN = 7;
const int LED_PIN = 8;
const int SERVO_PIN = 9;
const int RFID_SS = 10;

const int openSpeed = 120;
const int closeSpeed = 60;
const int stopServo = 90;
const int moveTime = 4000;

// --- אובייקטים ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(FINGERPRINT_TX, FINGERPRINT_RX);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
MFRC522 rfid(RFID_SS, RFID_RST);
Servo seaServo;

// --- מכונת מצבים ---
enum SystemState {
  STATE_WELCOME,
  STATE_APPROACH,
  STATE_TOUCH,
  STATE_SEA_OPENED
};

SystemState currentState = STATE_WELCOME;

void setup() {


  // כפתור מחובר בין D2 ל-GND
  // לא לחוץ = HIGH, לחוץ = LOW
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  seaServo.attach(SERVO_PIN);
  seaServo.write(90);
  delay(200);
  seaServo.detach();

  lcd.init();
  lcd.backlight();

  finger.begin(57600);

  SPI.begin();
  rfid.PCD_Init();

  showWelcomeMessage();
}

void loop() {
  switch (currentState) {

    case STATE_WELCOME:
      if (getFingerprintID() > 0) {
       showApproachMessage();

        currentState = STATE_APPROACH;
        delay(1500);
      }
      break;

    case STATE_APPROACH:
      {
        long distance = getDistance();

        if (distance > 0 && distance <= 5) {
          showTouchMessage();

          currentState = STATE_TOUCH;
          delay(1500);
        }
      }
      break;

    case STATE_TOUCH:
      if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

        if (rfid.uid.uidByte[0] == 0xAC &&
            rfid.uid.uidByte[1] == 0x45 &&
            rfid.uid.uidByte[2] == 0xD3 &&
            rfid.uid.uidByte[3] == 0x38) {

          showSeaOpenMessage();
          digitalWrite(LED_PIN, HIGH);
          delay(700);
          openSea();

          currentState = STATE_SEA_OPENED;

        } else {
          showWrongStaffMessage();
          delay(1500);

          showTouchMessage();
        }

        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
      }
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
      break;

    case STATE_SEA_OPENED:
      // עם INPUT_PULLUP:
      // לא לחוץ = HIGH
      // לחוץ = LOW
      if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);

        if (digitalRead(BUTTON_PIN) == LOW) {
          
          showClosingMessage();
          digitalWrite(LED_PIN, LOW);
          closeSea();


          showWelcomeMessage();
          currentState = STATE_WELCOME;

          delay(1000);
        }
      }
      break;
  }
}

// --- פונקציות תנועה לסרבו ---

void openSea() {
  seaServo.attach(SERVO_PIN);
  delay(20);

  seaServo.write(openSpeed);
  delay(moveTime);

  seaServo.write(stopServo);
  delay(200);
  seaServo.detach();
}

void closeSea() {
  seaServo.attach(SERVO_PIN);
  delay(20);

  seaServo.write(closeSpeed);
  delay(moveTime);

  seaServo.write(stopServo);
  delay(200);
  seaServo.detach();
}

// --- פונקציות עזר ---



void showMessage(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}
void showWelcomeMessage() {
  showMessage("RedSea System", "Please Identify");
}

void showApproachMessage() {
  showMessage("Shalom Moshe", "Approach Water");
}

void showTouchMessage() {
  showMessage("Touch Water", "With The Staff");
}

void showSeaOpenMessage() {
  showMessage("THE SEA IS OPEN", "");
}

void showWrongStaffMessage() {
  showMessage("Wrong Staff!", "");
}

void showClosingMessage() {
  showMessage("Closing Sea...", "");
}
void showByeMessage() {
showMessage("Bye Egyptians!","");
}
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) return -1;

  return duration * 0.034 / 2;
}

int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}