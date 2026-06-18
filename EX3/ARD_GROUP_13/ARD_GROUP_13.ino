/*
 * ============================================================
 *   שומר המָן במדבר  -  Manna Guardian
 *   Industrial Automation - Final Practical Project (Exercise C)
 * ============================================================
 *
 *   המערכת מבקרת איסוף מָן לפי משקל:
 *     - כל משפחה סורקת תג RFID לזיהוי.
 *     - הארדואינו מודד את משקל המָן שנאסף.
 *     - אם המשקל בטווח היעד היומי -> סרוו פותח את האסם.
 *     - אם נאסף יותר מהיעד          -> LED אדום + באזר ("אסור לאגור!").
 *     - ביום שישי היעד כפול (משנה לשבת).
 *     - בשבת המערכת נעולה ואינה מאפשרת איסוף.
 *
 *   שני מצבי הפעלה:
 *     SIMULATION_MODE = 1  ->  לסביבת Tinkercad
 *                              (משקל = פוטנציומטר, RFID = כפתור)
 *     SIMULATION_MODE = 0  ->  לחומרה פיזית (HX711 + RC522)
 * ============================================================
 */

#define SIMULATION_MODE 1   // 1 = Tinkercad, 0 = real hardware

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#if SIMULATION_MODE == 0
  #include <HX711.h>
  #include <SPI.h>
  #include <MFRC522.h>
#endif

// =================== Pin Map ===================
#if SIMULATION_MODE
  const int POT_PIN        = A0;   // מדמה משקל 0..1023 -> 0..500g
  const int BTN_RFID_SIM   = A1;   // לחיצה = "תג זוהה"
#else
  const int HX711_DT       = 2;
  const int HX711_SCK      = 3;
  const int RFID_SS        = 10;
  const int RFID_RST       = A0;
#endif

const int LED_GREEN        = 4;
const int LED_RED          = 5;
const int BUZZER           = 6;
const int BTN_DAY          = 7;   // קידום יום בשבוע
const int BTN_NEXT         = 8;   // סיום איסוף / משפחה הבאה
const int SERVO_PIN        = 9;

// =================== Constants ===================
const float NORMAL_TARGET  = 200.0;   // gram per person on weekday
const float FRIDAY_TARGET  = 400.0;   // double portion (משנה לשבת)
const float TOLERANCE      = 20.0;    // ±20g acceptable window
const long  DEBOUNCE_MS    = 50;
const float CALIB_FACTOR   = -7050.0; // נדרש כיול ב-HX711 הפיזי

const byte SUN = 0, FRI = 5, SAT = 6;
const char* DAY_NAMES[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};

enum State { IDLE, COLLECTING, OK, OVER, SHABBAT };

// =================== Globals ===================
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo silo;

#if SIMULATION_MODE == 0
  HX711 scale;
  MFRC522 rfid(RFID_SS, RFID_RST);
#endif

byte         currentDay     = SUN;
String       currentFamily  = "";
float        lastWeight     = 0.0;
State        state          = IDLE;

unsigned long lastDayBtnT   = 0;
unsigned long lastNextBtnT  = 0;
unsigned long lastRfidBtnT  = 0;
bool          lastDayState  = HIGH;
bool          lastNextState = HIGH;
bool          lastRfidState = HIGH;

float         tareOffset    = 0.0;    // for simulation tare

// =================== Setup ===================
void setup() {
  Serial.begin(9600);
  Serial.println(F("=== Manna Guardian Boot ==="));

  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED,   OUTPUT);
  pinMode(BUZZER,    OUTPUT);
  pinMode(BTN_DAY,   INPUT_PULLUP);
  pinMode(BTN_NEXT,  INPUT_PULLUP);

#if SIMULATION_MODE
  pinMode(POT_PIN,      INPUT);
  pinMode(BTN_RFID_SIM, INPUT_PULLUP);
#else
  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(CALIB_FACTOR);
  scale.tare();

  SPI.begin();
  rfid.PCD_Init();
#endif

  lcd.init();
  lcd.backlight();
  lcd.print("Manna Guardian");
  lcd.setCursor(0, 1);
  lcd.print("Booting...");

  silo.attach(SERVO_PIN);
  closeSilo();

  delay(1200);
  updateLcdIdle();
  Serial.print(F("Day = "));
  Serial.println(DAY_NAMES[currentDay]);
}

// =================== Loop ===================
void loop() {
  handleDayButton();
  handleNextButton();

  if (currentDay == SAT) {
    enterShabbat();
    return;
  }

  switch (state) {
    case IDLE:
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED,   LOW);
      noTone(BUZZER);
      checkRfid();
      break;

    case COLLECTING:
    case OK:
    case OVER:
      readAndJudgeWeight();
      break;

    case SHABBAT:
      break;
  }
}

// =================== RFID / family scan ===================
void checkRfid() {
#if SIMULATION_MODE
  bool s = digitalRead(BTN_RFID_SIM);
  if (s == LOW && lastRfidState == HIGH &&
      millis() - lastRfidBtnT > DEBOUNCE_MS) {
    lastRfidBtnT = millis();
    onFamilyScanned("FAM-SIM");
  }
  lastRfidState = s;
#else
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial())   return;
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) uid += String(rfid.uid.uidByte[i], HEX);
  rfid.PICC_HaltA();
  onFamilyScanned(uid);
#endif
}

void onFamilyScanned(const String& fam) {
  currentFamily = fam;
  Serial.print(F("[SCAN] Family: "));
  Serial.println(currentFamily);
  doTare();
  state = COLLECTING;
  updateLcdCollecting();
}

// =================== Weight reading ===================
float readWeightGrams() {
#if SIMULATION_MODE
  int raw = analogRead(POT_PIN);             // 0..1023
  float g = (raw / 1023.0) * 500.0;          // 0..500g
  return g - tareOffset;
#else
  if (!scale.is_ready()) return lastWeight;
  return scale.get_units(5);
#endif
}

void doTare() {
#if SIMULATION_MODE
  tareOffset = (analogRead(POT_PIN) / 1023.0) * 500.0;
#else
  scale.tare();
#endif
}

void readAndJudgeWeight() {
  float w = readWeightGrams();
  if (w < 0) w = 0;
  lastWeight = w;

  float target = (currentDay == FRI) ? FRIDAY_TARGET : NORMAL_TARGET;
  State newState;

  if (w < target - TOLERANCE) {
    newState = COLLECTING;
  } else if (w <= target + TOLERANCE) {
    newState = OK;
  } else {
    newState = OVER;
  }

  if (newState != state) {
    state = newState;
    Serial.print(F("[STATE] -> "));
    Serial.print(stateName(state));
    Serial.print(F(" | family="));
    Serial.print(currentFamily);
    Serial.print(F(" weight="));
    Serial.print(w, 1);
    Serial.println(F("g"));
  }

  applyOutputs();
  updateLcdWeight(target);
}

void applyOutputs() {
  switch (state) {
    case COLLECTING:
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED,   LOW);
      noTone(BUZZER);
      closeSilo();
      break;
    case OK:
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED,   LOW);
      noTone(BUZZER);
      openSilo();
      break;
    case OVER:
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED,   HIGH);
      tone(BUZZER, 1200);
      closeSilo();
      break;
    default: break;
  }
}

// =================== Buttons ===================
void handleDayButton() {
  bool s = digitalRead(BTN_DAY);
  if (s == LOW && lastDayState == HIGH &&
      millis() - lastDayBtnT > DEBOUNCE_MS) {
    lastDayBtnT = millis();
    currentDay = (currentDay + 1) % 7;
    Serial.print(F("[DAY] -> "));
    Serial.println(DAY_NAMES[currentDay]);
    resetCycle();
  }
  lastDayState = s;
}

void handleNextButton() {
  bool s = digitalRead(BTN_NEXT);
  if (s == LOW && lastNextState == HIGH &&
      millis() - lastNextBtnT > DEBOUNCE_MS) {
    lastNextBtnT = millis();
    Serial.print(F("[NEXT] family="));
    Serial.print(currentFamily);
    Serial.print(F(" final="));
    Serial.print(lastWeight, 1);
    Serial.println(F("g"));
    resetCycle();
  }
  lastNextState = s;
}

void resetCycle() {
  state = IDLE;
  currentFamily = "";
  lastWeight = 0;
  doTare();
  closeSilo();
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED,   LOW);
  noTone(BUZZER);
  updateLcdIdle();
}

// =================== Servo ===================
void openSilo()  { silo.write(90); }
void closeSilo() { silo.write(0);  }

// =================== Shabbat ===================
void enterShabbat() {
  if (state != SHABBAT) {
    state = SHABBAT;
    Serial.println(F("[SHABBAT] System locked"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("** SHABBAT **");
    lcd.setCursor(0, 1);
    lcd.print("No collection");
  }
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED,   HIGH);
  noTone(BUZZER);
  closeSilo();
}

// =================== LCD helpers ===================
void updateLcdIdle() {
  float t = (currentDay == FRI) ? FRIDAY_TARGET : NORMAL_TARGET;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[");
  lcd.print(DAY_NAMES[currentDay]);
  lcd.print("] Scan card");
  lcd.setCursor(0, 1);
  lcd.print("Target: ");
  lcd.print(t, 0);
  lcd.print("g");
}

void updateLcdCollecting() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[");
  lcd.print(DAY_NAMES[currentDay]);
  lcd.print("] ");
  lcd.print(currentFamily.substring(0, 8));
  lcd.setCursor(0, 1);
  lcd.print("Collecting...");
}

void updateLcdWeight(float target) {
  static unsigned long lastT = 0;
  if (millis() - lastT < 200) return;
  lastT = millis();

  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);

  if      (state == OK)   lcd.print("OK!  ");
  else if (state == OVER) lcd.print("STOP ");
  else                    lcd.print("More ");

  lcd.print(lastWeight, 0);
  lcd.print("/");
  lcd.print(target, 0);
  lcd.print("g");
}

const char* stateName(State s) {
  switch (s) {
    case IDLE:       return "IDLE";
    case COLLECTING: return "COLLECTING";
    case OK:         return "OK";
    case OVER:       return "OVER";
    case SHABBAT:    return "SHABBAT";
    default:         return "?";
  }
}
