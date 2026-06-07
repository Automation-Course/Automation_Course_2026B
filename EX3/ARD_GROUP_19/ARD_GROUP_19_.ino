// ==========================================
// פרויקט עמדת חלוקת מן חכמה 
// ==========================================

// --- ייבוא ספריות ---
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include <Servo.h>
#include "IRremote.hpp" 
#include "HX711.h"    

// --- הגדרת פינים ---
#define SERVO_PIN 9
#define BUTTON_PIN 2   
#define LED_PIN 12     
#define LDR_PIN A0    
#define IR_PIN 7       
#define LOADCELL_DOUT_PIN 4
#define LOADCELL_SCK_PIN 5

// --- יצירת אובייקטים ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo indicatorServo;
HX711 scale; 

// --- משתנים גלובליים ---
int lightValue;
long rawWeightValue; 
float weightGrams; 
bool isIdentified = false;

// --- פונקציית האתחול (Setup) ---
void setup() {
  Serial.begin(9600); 

  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  pinMode(LED_PIN, OUTPUT);

  indicatorServo.attach(SERVO_PIN);
  indicatorServo.write(90); // מצב מנוחה לסרוו

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Manna Station");
  lcd.setCursor(0, 1);
  lcd.print("Please Identify");

  Serial.println("Initializing the scale...");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.tare(); // איפוס משקל התחלתי

  IrReceiver.begin(IR_PIN, ENABLE_LED_FEEDBACK); 
  Serial.println("System Ready - Waiting for IR Remote");
}

// --- הלולאה המרכזית (Loop) ---
void loop() {

  // שלב 1: זיהוי השלט (בקרת הרשאות)
  if (isIdentified == false) {
    indicatorServo.write(90);
    digitalWrite(LED_PIN, LOW); 
    
    if (IrReceiver.decode()) {
      if (IrReceiver.decodedIRData.protocol != UNKNOWN && IrReceiver.decodedIRData.decodedRawData != 0) {
        
        Serial.print("Real IR Signal Detected! Protocol: ");
        Serial.println(IrReceiver.decodedIRData.protocol);
        Serial.print("Code: ");
        Serial.println(IrReceiver.decodedIRData.decodedRawData, HEX);
        
        isIdentified = true; 
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("User Identified");
        lcd.setCursor(0, 1);
        lcd.print("Press Button");
        Serial.println("User identified successfully. Locked on target.");
        
        delay(1500); 
      }
      IrReceiver.resume(); // הכנת המקלט לאות הבא
    }
  }

  // שלב 2: שקילה והחלטות בקרה
  else {
    indicatorServo.write(90);
    
    if (digitalRead(BUTTON_PIN) == LOW) {
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Checking...");
      Serial.println("Button Pressed! Starting measurements...");
      delay(500);

      // דגימת נתוני החיישנים
      lightValue = analogRead(LDR_PIN);
      rawWeightValue = scale.read(); 
      weightGrams = (rawWeightValue + 392832) * 0.0025316; // נוסחת כיול משקל

      Serial.print("Weight in Grams: ");
      Serial.print(weightGrams);
      Serial.println(" g");
      Serial.print("LDR value: ");
      Serial.println(lightValue);

      // בקרת תנאי סביבה - בדיקת אור
      if (lightValue > 410) {
        digitalWrite(LED_PIN, LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Too dark");
        lcd.setCursor(0, 1);
        lcd.print("Wait morning");
        Serial.println("Status: Too dark");
        
        indicatorServo.write(90); 
        delay(3000);
      }
      
      // בקרת כמות - בדיקת משקל
      else {
        if (weightGrams < 300) { 
          digitalWrite(LED_PIN, LOW); 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Too little");
          lcd.setCursor(0, 1);
          lcd.print("Add manna");
          Serial.println("Status: Too little manna -> Rotating CW");
          
          indicatorServo.write(0); 
          delay(5000); 
          indicatorServo.write(90); 
          delay(1000); 
        }
        
        else if (weightGrams >= 300 && weightGrams <= 700) { 
          // --- מנה מאושרת! לד אדום נדלק ---
          digitalWrite(LED_PIN, HIGH); 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Approved");
          lcd.setCursor(0, 1);
          lcd.print("Manna OK");
          Serial.println("Status: Approved portion -> STOP");
          
          indicatorServo.write(90); 
          delay(5000); 
        }
        
        else {
          digitalWrite(LED_PIN, LOW); 
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Too much");
          lcd.setCursor(0, 1);
          lcd.print("Remove manna");
          Serial.println("Status: Too much manna -> Rotating CCW");
          
          indicatorServo.write(180); 
          delay(5000); 
          indicatorServo.write(90); 
          delay(1000); 
        }
      }

      // קריאה לפונקציה מודולרית לאיפוס המערכת
      resetSystem(); 
    }
  }
}

// ==========================================
// פונקציות עזר מודולריות
// ==========================================

/*
 * פונקציית resetSystem:
 */
void resetSystem() {
  indicatorServo.write(90); 
  digitalWrite(LED_PIN, LOW); 
  isIdentified = false;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Manna Station");
  lcd.setCursor(0, 1);
  lcd.print("Please Identify");
  
  Serial.println("System reset. Waiting for next user...");
  delay(1000);
}