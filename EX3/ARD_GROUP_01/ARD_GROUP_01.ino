#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include <Servo.h>
#include <HX711.h>

#define LDR_PIN 7
#define LED1_PIN 6
#define LED2_PIN 5
#define TEMP_PIN 4
#define LED_RED_TEMP_PIN 3
#define IR_PIN 11
#define SERVO_PIN 9
#define GREEN_LED_PIN 8
#define RED_LED_PIN 10
#define HX711_DT A0
#define HX711_SCK A1
#define BUTTON_PIN 12

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo gateServo;
HX711 scale;

bool gateOpen = false;
unsigned long lastCheck = 0;
int inputAnswer = 0;

void setup() {
  pinMode(LDR_PIN, INPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED_RED_TEMP_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  sensors.begin();
  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);
  gateServo.attach(SERVO_PIN);
  gateServo.detach();

  scale.begin(HX711_DT, HX711_SCK);
  scale.set_scale(364800);
  scale.tare();

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Our grade is?");
  lcd.setCursor(0, 1);
  lcd.print("Answer: ?");

  Serial.begin(9600);
  Serial.println("המערכת מוכנה");
}

int translateKey(int command) {
  switch (command) {
    case 69: return 1;
    case 25: return 0;
    default: return -1;
  }
}

void shutdownSystem() {
  Serial.println("כיבוי מערכת - האחרון עבר!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Goodbye!");
  lcd.setCursor(0, 1);
  lcd.print("System OFF");
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(LED_RED_TEMP_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  while (true) {}  // עצור לנצח
}

void loop() {

  // --- כפתור כיבוי ---
  if (digitalRead(BUTTON_PIN) == LOW) {
    shutdownSystem();
  }

  if (millis() - lastCheck >= 500) {
    lastCheck = millis();

    // --- חיישן אור ---
    int lightStatus = digitalRead(LDR_PIN);
    if (lightStatus == HIGH) {
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);
      Serial.println("חשוך - בני ישראל מדליקים לפידים!");
    } else {
      digitalWrite(LED1_PIN, LOW);
      digitalWrite(LED2_PIN, LOW);
      Serial.println("מואר - הלפידים כבויים");
    }

    // --- חיישן טמפרטורה ---
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);
    Serial.print("טמפרטורה: ");
    Serial.print(temperature);
    Serial.println(" מעלות");

    if (temperature > 30) {
      digitalWrite(LED_RED_TEMP_PIN, HIGH);
      Serial.println("חם מדי - כסו את המצות!");
    } else {
      digitalWrite(LED_RED_TEMP_PIN, LOW);
      Serial.println("הטמפרטורה תקינה");
    }

    // --- משקל ---
    if (scale.is_ready()) {
      float weight = scale.get_units(5);
      Serial.print("משקל: ");
      Serial.print(weight);
      Serial.println(" קג");

      if (weight < 0.01) {
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
        Serial.println("אין משקל על החיישן");
      } else if (weight > 5.0) {
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
        Serial.println("משקל כבד מדי - הורד משקל!");
      } else {
        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);
        Serial.println("משקל תקין - אפשר לצאת!");
      }
    }
  }

  // --- שלט IR ושאלה ---
  if (IrReceiver.decode()) {
    int command = IrReceiver.decodedIRData.command;

    if (command != 0) {
      Serial.print("קיבלנו מהשלט: ");
      Serial.println(command);

      if (command == 28) {
        if (inputAnswer == 100) {
          gateOpen = true;
          gateServo.attach(SERVO_PIN);
          gateServo.write(0);
          delay(600);
          gateServo.write(90);
          Serial.println("השער פתוח - מחכה 5 שניות");
          delay(5000);
          gateServo.write(180);
          delay(600);
          gateServo.write(90);
          delay(200);
          gateServo.detach();
          Serial.println("השער נסגר");
          gateOpen = false;
          inputAnswer = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Our grade is?");
          lcd.setCursor(0, 1);
          lcd.print("Answer: ?");
          Serial.println("מוכן לבא בתור!");
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Wrong answer!");
          lcd.setCursor(0, 1);
          lcd.print("Try again...");
          Serial.println("תשובה שגויה - נסה שוב");
          delay(2000);
          inputAnswer = 0;
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Our grade is?");
          lcd.setCursor(0, 1);
          lcd.print("Answer: ?");
        }
      } else {
        int digit = translateKey(command);
        if (digit != -1) {
          inputAnswer = inputAnswer * 10 + digit;
          lcd.setCursor(0, 1);
          lcd.print("Answer: ");
          lcd.print(inputAnswer);
          Serial.print("ספרה שנלחצה: ");
          Serial.println(digit);
        }
      }
    }
    IrReceiver.resume();
  }

  delay(100);
}