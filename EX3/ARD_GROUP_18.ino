#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <VL53L1X.h>

// ===================== הגדרת פינים =====================
#define LED_PIN       7
#define BUTTON_PIN    8
#define SERVO_PIN     6
#define RFID_SS_PIN   2
#define RFID_RST_PIN  9

// ===================== קבועים =====================
#define HAND_THRESHOLD    200   // סף מרחק בממ להרמת יד (20 סמ)
#define DEBOUNCE_DELAY    50    // השהיה למניעת רעש בכפתור (מש)
#define HAND_COOLDOWN     1500  // זמן המתנה בין הרמות יד (מש)
#define TOTAL_PLAGUES     10    // מספר המכות

// ===================== אתחול רכיבים =====================
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
VL53L1X laser;

// ===================== משתני מצב =====================
int  plagueIndex      = 0;
bool systemActive     = false;//  RFID
bool handWasClose     = false;
bool seaOpen          = false;
unsigned long lastHandTime   = 0;
unsigned long lastButtonTime = 0;

// ===================== רשימת המכות =====================
const char* plagueNames[TOTAL_PLAGUES] = {
  "Dam",        // דם
  "Tzfardea",   // צפרדע
  "Kinim",      // כינים
  "Arov",       // ערוב
  "Dever",      // דבר
  "Shchin",     // שחין
  "Barad",      // ברד
  "Arbeh",      // ארבה
  "Choshech",   // חושך
  "Bechorot"    // מכת בכורות
};

// ===================== פונקציות עזר =====================

void displayLCD(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void blinkLED(int times, int delayMs) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(delayMs);
    digitalWrite(LED_PIN, LOW);
    delay(delayMs);
  }
}

void resetSystem() {
  plagueIndex  = 0;
  systemActive = false;// RFID
  seaOpen      = false;
  handWasClose = false;

  gateServo.write(90);/////שינוי SERVO ל 90 על מנת שלא יסתובב ברצף
  digitalWrite(LED_PIN, LOW);

  displayLCD("Show Moses card", "to activate");

  Serial.println("=== System Reset ===");
  Serial.println("Waiting for RFID card...");
}

void activatePlague(int index) {
  Serial.print("--- Plague #");
  Serial.print(index + 1);
  Serial.print(": ");
  Serial.println(plagueNames[index]);

  char line1[17];
  snprintf(line1, sizeof(line1), "Plague %d/%d:", index + 1, TOTAL_PLAGUES);

  displayLCD(line1, plagueNames[index]);
  blinkLED(3, 300);
  digitalWrite(LED_PIN, HIGH);
}

void openRedSea() {
  Serial.println("=== Red Sea opens! Israel is free! ===");

  displayLCD("Bnei Israel -", "Cross! Sea parts!");
  gateServo.write(180);/////פתיחת סיבוב SERVO
  delay(10000); /// 10 שניות סיבוב
  gateServo.write(90);  // עצור את המנוע כדי שהשער יישאר פתוח!
  // ----------------------------------------
  seaOpen = true;

  blinkLED(5, 150);
  digitalWrite(LED_PIN, HIGH);
}

// ===================== SETUP =====================
void setup() {
  Serial.begin(9600);
  Serial.println("=== Exodus System Starting ===");

  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();

  gateServo.attach(SERVO_PIN);
  gateServo.write(90);/////////שינוי SERVO ל 90 על מנת שלא יסתובב ברצף

  SPI.begin();
  rfid.PCD_Init();
  Serial.println("RFID ready");

  Wire.begin();
  laser.setTimeout(500);
  if (!laser.init()) {
    Serial.println("ERROR: VL53L1X not found!");
    displayLCD("ERROR:", "Laser not found");
    while (1);
  }
  laser.setDistanceMode(VL53L1X::Long);
  laser.setMeasurementTimingBudget(50000);
  laser.startContinuous(50);
  Serial.println("Laser VL53L1X ready");

  resetSystem();
}

// ===================== LOOP =====================
void loop() {

  // --- בדיקת כפתור ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    unsigned long now = millis();
    if (now - lastButtonTime > DEBOUNCE_DELAY) {
      lastButtonTime = now;
      Serial.println(">>> Button pressed - Pharaoh's heart hardened! Reset <<<");
      displayLCD("Heart hardened!", "Resetting...");
      delay(1500);
      resetSystem();
      return;
    }
  }

  // --- בדיקת RFID ---
  if (!systemActive) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      Serial.println(">>> Moses card detected! System activated <<<");
      systemActive = true;
      plagueIndex  = 0;

      displayLCD("Welcome Moses!", "Raise hand 2 start");
      blinkLED(2, 200);

      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
    return;
  }

  // --- אם הים פתוח ---
  if (seaOpen) return;

  // --- קריאת לייזר ---
  int distance   = laser.read();
  int distanceCm = distance / 10;

  bool handClose = (distanceCm > 0 && distanceCm < HAND_THRESHOLD);

  // --- זיהוי הרמת יד ---
  if (handClose && !handWasClose) {
    unsigned long now = millis();
    if (now - lastHandTime > HAND_COOLDOWN) {
      lastHandTime = now;

      if (plagueIndex < TOTAL_PLAGUES) {
        activatePlague(plagueIndex);
        plagueIndex++;
      } else {
        openRedSea();
      }
    }
  }

  handWasClose = handClose;

  // --- הדפסה ל Serial Monitor ---
  static bool prevHandClose = false;
  if (handClose != prevHandClose) {
    Serial.print("Distance: ");
    Serial.print(distanceCm);
    Serial.print(" cm | Hand: ");
    Serial.println(handClose ? "CLOSE" : "FAR");
    prevHandClose = handClose;
  }
}
