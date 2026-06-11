#include <Wire.h>                  
#include <LiquidCrystal_I2C.h> 
#include "Adafruit_VL53L1X.h" 
#include <Servo.h>            
#include <SPI.h>
#include <MFRC522.h>

// הגדרות פינים - אנלוגי A0 לחיישן האור
const int nightLedPin = 2;    // נורת הלילה הצהובה מחוברת ל-D2
const int ldrAnalogPin = A0;  // פין האות האנלוגי (AO) מחובר לפין A0!
const int buttonPin = 4;      // הכפתור מחובר לפין D4
const int ledPin = 6;         // הלד הירוק של השער מחובר לפין D6
const int servoPin = 7;       // המנוע מחובר לפין D7
#define RST_PIN         9     // פין RST של ה-RFID מחובר ל-D9
#define SS_PIN          10    // פין SDA/SS של ה-RFID מחובר ל-D10

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X();
Servo myServo;                
MFRC522 mfrc522(SS_PIN, RST_PIN); 

int lastState = -1; 

// --- 1. פונקציה עצמאית לחלוטין שבודקת את האור ומגיבה מיד ---
void checkLightSensor() {
  int ldrValue = analogRead(ldrAnalogPin); 
  if (ldrValue > 500) {
    digitalWrite(nightLedPin, HIGH); // חושך -> נורה צהובה נדלקת
  } else {
    digitalWrite(nightLedPin, LOW);  // אור -> נורה צהובה נכבית
  }
}

// --- 2. פונקציית המתנה חכמה שמחליפה את ה-delay הרגיל ---
void smartDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    checkLightSensor(); // בזמן שהשער ממתין, אנחנו קוראים את החיישן האנלוגי בלי הפסקה!
    delay(1);           
  }
}

void setup() {
  Wire.begin();
  SPI.begin();           
  mfrc522.PCD_Init();    
  
  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(ledPin, OUTPUT);
  pinMode(nightLedPin, OUTPUT);    
  
  digitalWrite(ledPin, LOW);        
  digitalWrite(nightLedPin, LOW);   
  
  myServo.attach(servoPin);
  myServo.write(90);                
  delay(100);
  myServo.detach();                 
  
  lcd.init();          
  lcd.backlight();    
  
  if (!vl53.begin(0x29, &Wire)) { 
    lcd.setCursor(0, 0);
    lcd.print("Laser Error!   ");
    while (1); 
  }
  vl53.startRanging();
}

void loop() {
  // בדיקה רציפה של האור בכל תחילת סיבוב
  checkLightSensor();

  // === קוד השער והלייזר (נפרד לחלוטין בזכות ה-smartDelay) ===
  if (vl53.dataReady()) {
    int distanceMm = vl53.distance();
    vl53.clearInterrupt(); 
    
    if (distanceMm == -1) {
      distanceMm = 9999; 
    }

    if (distanceMm > 30 && distanceMm <= 500) {
      if (lastState != 1) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("To open the sea ");
        lcd.setCursor(0, 1);
        lcd.print("PressBtn or Card"); 
        lastState = 1;
      }
      
      bool rfidDetected = false;
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
        rfidDetected = true;
        mfrc522.PICC_HaltA(); 
      }

      if (digitalRead(buttonPin) == LOW || rfidDetected == true) {
        digitalWrite(ledPin, HIGH); 
        myServo.attach(servoPin);
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Opening the sea ");
        myServo.write(180);     
        smartDelay(3000); // שינוי לדיליי חכם: השער ימתין, אבל האור ימשיך לעבוד במקביל!
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("The sea is open "); 
        lcd.setCursor(0, 1);
        lcd.print("Go through      "); 
        myServo.write(90);      
        smartDelay(3000); // שינוי לדיליי חכם!
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Closing the sea ");
        myServo.write(0);       
        smartDelay(3000); // שינוי לדיליי חכם!
        
        myServo.write(90);      
        delay(100);
        myServo.detach();       
        digitalWrite(ledPin, LOW); 
        
        lastState = -1; 
      }
    } 
    else {
      if (lastState != 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Exodus          "); 
        lcd.setCursor(0, 1);
        lcd.print("From Egypt      "); 
        lastState = 0;
      }
    }
  }
  smartDelay(20); // גם כאן בסוף הלולאה
}