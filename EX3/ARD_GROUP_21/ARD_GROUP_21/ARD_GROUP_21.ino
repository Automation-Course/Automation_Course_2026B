#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.hpp>
#include <HX711.h>

#define TRIG_PIN       7
#define ECHO_PIN       6
#define GREEN_LED_PIN  5
#define RED_LED_PIN    3
#define SERVO_PIN      9
#define BUTTON_PIN     A1
#define IR_PIN         2
#define HX711_DOUT     12
#define HX711_SCK      13

#define DISTANCE_THRESHOLD  50
#define GATE_OPEN_ANGLE     0
#define GATE_CLOSED_ANGLE   90
#define AUTO_CLOSE_DELAY    10000
#define WEIGHT_THRESHOLD    85.0
#define SCALE_FACTOR        409.4

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;
HX711 scale;

bool gateOpen = false;
unsigned long gateOpenTime = 0;
bool irApproved = false;
int fiveCount = 0;

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  return (duration * 0.034) / 2.0;
}

float measureWeight() {
  if (scale.is_ready()) {
    return scale.get_units(5);
  }
  return 0;
}

void openGate() {
  gateServo.write(GATE_OPEN_ANGLE);
  gateOpen = true;
  gateOpenTime = millis();
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(RED_LED_PIN, LOW);
  Serial.println("[GATE] שער נפתח");
}

void closeGate() {
  gateServo.write(GATE_CLOSED_ANGLE);
  gateOpen = false;
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  Serial.println("[GATE] שער נסגר");
}

void showLCD(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  showLCD("Yam Suf Gate", "Initializing...");

  gateServo.attach(SERVO_PIN);
  closeGate();

  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);

  scale.begin(HX711_DOUT, HX711_SCK);
  scale.set_scale(SCALE_FACTOR);
  scale.tare();
  Serial.println("[SCALE] חיישן משקל מוכן");

  showLCD("System Ready", "Approach gate...");
  Serial.println("[SYSTEM] המערכת מוכנה");
}

void loop() {

  // --- כפתור חירום ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (gateOpen) {
      Serial.println("[EMERGENCY] חירום — סוגר שער");
      closeGate();
      showLCD("STOP! Mitzri!", "No entry!");
      digitalWrite(RED_LED_PIN, HIGH);
      delay(3000);
      digitalWrite(RED_LED_PIN, LOW);
      showLCD("System Ready", "Approach gate...");
    }
    return;
  }

  // --- סגירה אוטומטית אחרי 20 שניות ---
  if (gateOpen && (millis() - gateOpenTime >= AUTO_CLOSE_DELAY)) {
    Serial.println("[GATE] סוגר אוטומטית");
    closeGate();
    showLCD("Gate closed", "Approach again");
    delay(2000);
    showLCD("System Ready", "Approach gate...");
    return;
  }

  if (gateOpen) return;

  // --- בדיקת קירבה ---
  float distance = measureDistance();

  if (distance > 0 && distance < DISTANCE_THRESHOLD) {
    Serial.print("[ULTRASONIC] אדם זוהה: ");
    Serial.println(distance);

    // --- שלב 1: בקש מצות ---
    showLCD("Place matzot", "on the scale!");
    delay(4000);

    float weight = measureWeight();
    Serial.print("[SCALE] משקל: ");
    Serial.println(weight);
    bool weightOK = (weight >= WEIGHT_THRESHOLD);

    // --- שלב 2: בקש קוד ---
    showLCD("Enter code:", "");

    irApproved = false;
    fiveCount = 0;

    unsigned long irStart = millis();
    while (millis() - irStart < 10000) {
      if (IrReceiver.decode()) {
        uint8_t cmd = IrReceiver.decodedIRData.command;
        IrReceiver.resume();
        if (cmd == 0x40) {
          fiveCount++;
        } else if (cmd == 0x1C && fiveCount >= 3) {
          irApproved = true;
          fiveCount = 0;
          break;
        } else {
          fiveCount = 0;
        }
      }
      if (digitalRead(BUTTON_PIN) == LOW) break;
    }

    // --- שלב 3: החלטה ---
    Serial.print("[CHECK] משקל תקין: "); Serial.println(weightOK ? "כן" : "לא");
    Serial.print("[CHECK] קוד תקין: "); Serial.println(irApproved ? "כן" : "לא");

    if (irApproved && weightOK) {
      Serial.println("[ACCESS] מאושר — בן ישראל!");
      showLCD("Derech Tzlacha!", "Go in peace :)");
      openGate();
    } else {
      Serial.println("[ACCESS] נדחה — מצרי!");
      digitalWrite(RED_LED_PIN, HIGH);
      showLCD("STOP! Mitzri!", "No entry!");
      delay(3000);
      digitalWrite(RED_LED_PIN, LOW);
      showLCD("System Ready", "Approach gate...");
    }
  }

  delay(200);
}