#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

// הגדרת פינים
#define LED_RED 5
#define LED_GREEN 7
#define SOUND_PIN 3
#define SERVO_PIN 6
#define RESET_BUTTON 2
#define LIGHT_PIN 4

// הגדרת פינים ל-RFID
#define SS_PIN 10
#define RST_PIN 9

// אתחול הרכיבים
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo seaServo;

// משתנים
bool isReadyToOpen = false;
bool isSeaOpen = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(SOUND_PIN, INPUT);
  pinMode(RESET_BUTTON, INPUT_PULLUP); 
  pinMode(LIGHT_PIN, INPUT);
  
  lcd.init();
  lcd.backlight();
  
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, LOW);
  
  lcd.setCursor(0, 0);
  lcd.print("The sea is");
  lcd.setCursor(0, 1);
  lcd.print("closed");
}

void loop() {
  
  // --- בדיקת תנאי סגירת הים ---
  
  bool isButtonPressed = (digitalRead(RESET_BUTTON) == LOW);
  
  // החזרנו ל-LOW! החיישן מזהה אור ושולח LOW לארדואינו.
  bool isMorning = (digitalRead(LIGHT_PIN) == LOW); 

  // 1. סגירה בגלל זריחה (חיישן אור)
  if (isSeaOpen == true && isMorning == true) {
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sunrise!");
    lcd.setCursor(0, 1);
    lcd.print("Closing sea...");
    delay(3000); 
    
    seaServo.attach(SERVO_PIN);
    seaServo.write(0);  
    delay(1000);        
    seaServo.detach();  
    
    isReadyToOpen = false;
    isSeaOpen = false;
    
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("The sea is");
    lcd.setCursor(0, 1);
    lcd.print("closed");
    
    delay(500); 
    return;     
  }
  
  // 2. סגירה בגלל לחיצה על כפתור 
  if (isButtonPressed) {
    if (isSeaOpen == true) {
      seaServo.attach(SERVO_PIN);
      seaServo.write(0);  
      delay(1000);        
      seaServo.detach();  
    }
    
    isReadyToOpen = false;
    isSeaOpen = false;
    
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_GREEN, LOW);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("The sea is");
    lcd.setCursor(0, 1);
    lcd.print("closed");
    
    delay(500); 
    return;
  }


  // --- שלב ראשון: RFID ---
  if (!isReadyToOpen && !isSeaOpen) {
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    if ( ! mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    
    isReadyToOpen = true;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ready to open");
  }

  // --- שלב שני: המתנה לצעקה ---
  if (isReadyToOpen && !isSeaOpen) {
    
    if (digitalRead(SOUND_PIN) == HIGH) {
      isSeaOpen = true;
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("The sea is");
      lcd.setCursor(0, 1);
      lcd.print("open!");
      
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
      
      seaServo.attach(SERVO_PIN);
      seaServo.write(180); 
      
      return; 
    }

    digitalWrite(LED_RED, HIGH);
    for(int i=0; i<30; i++) { 
      delay(10); 
      if(digitalRead(SOUND_PIN) == HIGH) break; 
      if(digitalRead(RESET_BUTTON) == LOW) return; 
    }
    
    digitalWrite(LED_RED, LOW);
    for(int i=0; i<30; i++) { 
      delay(10); 
      if(digitalRead(SOUND_PIN) == HIGH) break; 
      if(digitalRead(RESET_BUTTON) == LOW) return;
    }
  }
}