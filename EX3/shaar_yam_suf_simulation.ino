#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#define TRIG_PIN 2
#define ECHO_PIN 3
#define BUTTON_PIN 4
#define SERVO_PIN 6
#define LED_PIN 8
#define BUZZER_PIN 9
#define TEMP_PIN A0

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;

bool gateOpen = false;
bool alarmActive = false;
unsigned long detectionTime = 0;
bool personDetected = false;

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  gateServo.attach(SERVO_PIN);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  gateServo.write(90);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  lcd.setCursor(0, 0);
  lcd.print("Shaar Yam Suf");
  lcd.setCursor(0, 1);
  lcd.print("Hatzeg Kartis");
  Serial.println("המערכת מוכנה");
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

float getTemperature() {
  int val = analogRead(TEMP_PIN);
  float voltage = val * (5.0 / 1023.0);
  return (voltage - 0.5) * 100;
}

void openGate() {
  float temp = getTemperature();
  gateServo.write(0);
  digitalWrite(LED_PIN, HIGH);
  gateOpen = true;
  detectionTime = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Chag Cherut!");
  lcd.setCursor(0, 1);
  lcd.print("Meushar ");
  lcd.print(temp, 1);
  lcd.print("C");
  Serial.println("כרטיס מורשה - השער נפתח");
  Serial.print("טמפרטורה: ");
  Serial.print(temp);
  Serial.println("C");
  delay(4000);
  gateServo.write(90);
  digitalWrite(LED_PIN, LOW);
  gateOpen = false;
  personDetected = false;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Shaar Yam Suf");
  lcd.setCursor(0, 1);
  lcd.print("Hatzeg Kartis");
  Serial.println("השער נסגר - מצב המתנה");
}

void alarmMode() {
  alarmActive = true;
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("EVED MUZAR!");
  lcd.setCursor(0, 1);
  lcd.print("Lo Rashai!");
  Serial.println("אזעקה - אדם לא מורשה");
  delay(2000);
  digitalWrite(BUZZER_PIN, LOW);
}

void resetGate() {
  gateServo.write(90);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  gateOpen = false;
  alarmActive = false;
  personDetected = false;
  detectionTime = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Shaar Yam Suf");
  lcd.setCursor(0, 1);
  lcd.print("Hatzeg Kartis");
  Serial.println("המערכת אופסה");
}

void loop() {
  long distance = getDistance();

  if (!alarmActive) {
    if (distance < 30 && distance > 0 && !personDetected && !gateOpen) {
      personDetected = true;
      detectionTime = millis();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hatzeg Kartis!");
      lcd.setCursor(0, 1);
      lcd.print("5 Shniot...");
      Serial.println("אדם זוהה - ממתין לכרטיס");
    }

    // בסימולציה: לחיצה על כפתור = כרטיס מורשה
    if (personDetected && !gateOpen) {
      if (digitalRead(BUTTON_PIN) == LOW) {
        openGate();
        personDetected = false;
      }
      if (millis() - detectionTime > 5000) {
        alarmMode();
        personDetected = false;
      }
    }
  }

  // כפתור כשיש אזעקה = איפוס
  if (digitalRead(BUTTON_PIN) == LOW && alarmActive) {
    resetGate();
    delay(500);
  }

  delay(100);
}
