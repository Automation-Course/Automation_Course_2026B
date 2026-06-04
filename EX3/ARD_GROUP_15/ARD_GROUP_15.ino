#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <VL53L1X.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// ---------- Pin Definitions ----------
#define GREEN_LED_PIN 4
#define RED_LED_PIN 5
#define BUTTON_PIN 6
#define ULTRASONIC_TRIG_PIN 7
#define ULTRASONIC_ECHO_PIN 8
#define RFID_RST_PIN 9
#define RFID_SS_PIN 10
#define SERVO_PIN 3

// ---------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- Laser ----------
VL53L1X laser;

// ---------- RFID ----------
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

// ---------- Servo ----------
Servo gateServo;

// ---------- System States ----------
enum SystemState {
  WAITING,
  USER_DETECTED,
  RFID_SCAN,
  LASER_CHECK,
  ACCESS_GRANTED,
  ACCESS_DENIED
};
SystemState currentState = WAITING;

// ---------- Constants ----------
const int USER_DETECTION_DISTANCE_CM = 30;
const int LASER_MIN_DISTANCE_CM = 1;
const int LASER_MAX_DISTANCE_CM = 8;
const int RFID_WAIT_TIME = 5000;
const int GATE_OPEN_TIME = 500;
const int GATE_CLOSE_TIME = 1950;

// ---------- Authorized RFID tags ----------
byte authorizedUID[] = {0xC3, 0x3F, 0xB6, 0xA9};

// ---------- Variables ----------
int ultrasonicDistance = 0;
int laserDistance = 0;
bool rfidApproved = false;
bool laserDistanceOk = false;
bool lastButtonState = LOW;

// ---------- Setup ----------
void setup() {
  Serial.begin(9600);
  Serial.println("--- Smart Camp Gate System Initialized ---");

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);

  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  gateServo.attach(SERVO_PIN);
  gateServo.write(90);
  Serial.println("Servo initialized - gate stopped");

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID initialized");

  Wire.begin();
  Wire.setClock(400000);
  delay(500);

  laser.setTimeout(500);
  if (!laser.init()) {
    Serial.println("ERROR: Laser not found!");
  } else {
    laser.setDistanceMode(VL53L1X::Long);
    laser.setMeasurementTimingBudget(50000);
    laser.startContinuous(50);
    Serial.println("Laser initialized");
  }

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("CAMP GATE");
  lcd.setCursor(0, 1);
  lcd.print("SYSTEM START");

  delay(2000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WAITING...");
  lcd.setCursor(0, 1);
  lcd.print("Come closer");

  Serial.println("Waiting for user...");
}

// ---------- Loop ----------
void loop() {
  switch (currentState) {
    case WAITING:
      handleWaitingState();
      break;
    case USER_DETECTED:
      handleUserDetectedState();
      break;
    case RFID_SCAN:
      handleRFIDScanState();
      break;
    case LASER_CHECK:
      handleLaserCheckState();
      break;
    case ACCESS_GRANTED:
      handleAccessGrantedState();
      break;
    case ACCESS_DENIED:
      handleAccessDeniedState();
      break;
  }
  delay(200);
}

// ---------- State Functions ----------

void handleWaitingState() {
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);

  ultrasonicDistance = readUltrasonicDistance();
  Serial.print("WAITING - Ultrasonic distance: ");
  Serial.print(ultrasonicDistance);
  Serial.println(" cm");

  if (ultrasonicDistance > 0 && ultrasonicDistance <= USER_DETECTION_DISTANCE_CM) {
    Serial.println("User detected near the gate");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("User detected");
    lcd.setCursor(0, 1);
    lcd.print("Press button");
    currentState = USER_DETECTED;
  }
}

void handleUserDetectedState() {
  ultrasonicDistance = readUltrasonicDistance();

  if (ultrasonicDistance > USER_DETECTION_DISTANCE_CM || ultrasonicDistance == 0) {
    Serial.println("User left - back to WAITING");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WAITING...");
    lcd.setCursor(0, 1);
    lcd.print("Come closer");
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    lastButtonState = LOW;
    currentState = WAITING;
    return;
  }

  if (isButtonClicked()) {
    Serial.println("Button clicked - starting RFID scan");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID tag");
    lcd.setCursor(0, 1);
    lcd.print("Please wait...");
    currentState = RFID_SCAN;
  }

  Serial.println("Waiting for button...");
}

void handleRFIDScanState() {
  Serial.println("Waiting for RFID scan...");
  unsigned long startTime = millis();

  while (millis() - startTime < RFID_WAIT_TIME) {
    if (checkRFID()) {
      rfidApproved = true;
      Serial.println("RFID approved - moving to laser check");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("RFID OK!");
      lcd.setCursor(0, 1);
      lcd.print("Stand close...");
      delay(1000);
      currentState = LASER_CHECK;
      return;
    }
    delay(200);
  }

  Serial.println("RFID timeout - access denied");
  rfidApproved = false;
  laserDistanceOk = false;
  currentState = ACCESS_DENIED;
}

void handleLaserCheckState() {
  laserDistance = readLaserDistance();

  if (laserDistance == -1) {
    Serial.println("Laser not ready - retrying...");
    delay(100);
    return;
  }

  laserDistanceOk = checkLaserDistance(laserDistance);

  Serial.print("Laser distance: ");
  Serial.print(laserDistance);
  Serial.println(" cm");
  Serial.print("Laser distance OK: ");
  Serial.println(laserDistanceOk);

  if (laserDistanceOk) {
    currentState = ACCESS_GRANTED;
  } else {
    currentState = ACCESS_DENIED;
  }
}

void handleAccessGrantedState() {
  Serial.println("ACCESS GRANTED");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access granted!");
  lcd.setCursor(0, 1);
  lcd.print("Gate opening...");

  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);

  openGate();
  delay(2000);
  closeGate();

  digitalWrite(GREEN_LED_PIN, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WAITING...");
  lcd.setCursor(0, 1);
  lcd.print("Come closer");

  currentState = WAITING;
}

void handleAccessDeniedState() {
  Serial.println("ACCESS DENIED");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access denied!");
  lcd.setCursor(0, 1);

  if (!rfidApproved) {
    lcd.print("Invalid RFID");
    Serial.println("Reason: Invalid RFID");
  } else if (!laserDistanceOk) {
    lcd.print("Wrong position");
    Serial.println("Reason: Wrong position");
  }

  digitalWrite(RED_LED_PIN, HIGH);
  digitalWrite(GREEN_LED_PIN, LOW);

  delay(2500);

  digitalWrite(RED_LED_PIN, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WAITING...");
  lcd.setCursor(0, 1);
  lcd.print("Come closer");

  rfidApproved = false;
  laserDistanceOk = false;
  currentState = WAITING;
}

// ---------- Sensor / Action Functions ----------

int readUltrasonicDistance() {
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  return duration * 0.0343 / 2;
}

int readLaserDistance() {
  int distanceMM = laser.read();
  if (laser.timeoutOccurred()) {
    Serial.println("Laser timeout!");
    return -1;
  }
  int distanceCM = distanceMM / 10;
  Serial.print("Laser raw reading: ");
  Serial.print(distanceMM);
  Serial.println(" mm");
  return distanceCM;
}

bool checkRFID() {
  if (!rfid.PICC_IsNewCardPresent()) {
    rfid.PCD_Reset();
    rfid.PCD_Init();
    return false;
  }
  if (!rfid.PICC_ReadCardSerial()) {
    rfid.PCD_Reset();
    rfid.PCD_Init();
    return false;
  }

  Serial.print("Card UID: ");
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i], HEX);
    Serial.print(" ");
  }
  Serial.println();

  bool authorized = true;
  for (byte i = 0; i < 4; i++) {
    if (rfid.uid.uidByte[i] != authorizedUID[i]) {
      authorized = false;
      break;
    }
  }

  if (authorized) {
    Serial.println("Card recognized: AUTHORIZED");
  } else {
    Serial.println("Card recognized: UNAUTHORIZED");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  rfid.PCD_Reset();
  rfid.PCD_Init();
  return authorized;
}

bool checkLaserDistance(int distanceCm) {
  return (distanceCm >= LASER_MIN_DISTANCE_CM && distanceCm <= LASER_MAX_DISTANCE_CM);
}

void openGate() {
  Serial.println("Opening gate...");
  gateServo.write(0);
  delay(GATE_OPEN_TIME);
  gateServo.write(90);
}

void closeGate() {
  Serial.println("Closing gate...");
  gateServo.write(180);
  delay(GATE_CLOSE_TIME);
  gateServo.write(90);
}

bool isButtonClicked() {
  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    lastButtonState = currentButtonState;
    return true;
  }
  lastButtonState = currentButtonState;
  return false;
}