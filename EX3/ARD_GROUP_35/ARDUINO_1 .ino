#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
#include <VL53L1X.h>
#include <Servo.h>

// --- הגדרת פינים לחומרה ---
#define RST_PIN         9          // פין Reset ל-RFID
#define SS_PIN          10         // פין SDA/SS ל-RFID
const int emergencyBtn = 2;        // כפתור חירום (מטה משה לסגירה סופית)
const int touchSensor = 4;         // חיישן מגע (לפתיחת הים בתחילת הנס)
const int servoPin = 3;            // מנוע סרוו 360 (פרופלור)
const int ledPin = 8;              // נורת אזהרה

// --- הגדרת הרכיבים ---
LiquidCrystal_I2C lcd(0x27, 16, 2); 
MFRC522 mfrc522(SS_PIN, RST_PIN);   
VL53L1X laserSensor;                
Servo barrierServo;

// --- משתנים גלובליים ---
int savedCount = 0;           // מונה ניצולים (בני ישראל)
bool systemLocked = false;    // מצב חירום
bool barrierOpen = false;     // הים מתחיל סגור
int distance = 0;             // מרחק המצרים
bool miracleStarted = false;  // האם הנס כבר התחיל?
bool isDangerState = false;   // מונע "הספמה" במוניטור בזמן סכנה

void setup() {
  barrierServo.attach(servoPin);
  barrierServo.write(90); 

  Serial.begin(9600);
  Wire.begin();               
  SPI.begin();                
  
  // תחילת הדפסות מסודרות למוניטור
  Serial.println("--- System Booting ---");
  
  lcd.init();
  lcd.backlight(); 
  lcd.setCursor(0, 0);
  lcd.print("System Ready..."); 
  
  mfrc522.PCD_Init();
  
  laserSensor.setTimeout(500);
  if (!laserSensor.init()) {
    Serial.println("ERROR: Laser sensor not detected!");
    lcd.clear();
    lcd.print("Laser Error!");
    while (1); 
  }
  laserSensor.setDistanceMode(VL53L1X::Long);
  laserSensor.setMeasurementTimingBudget(50000);
  laserSensor.startContinuous(50);
  
  pinMode(emergencyBtn, INPUT); 
  pinMode(touchSensor, INPUT); 
  pinMode(ledPin, OUTPUT);
  
  Serial.println("System Ready. Waiting for touch sensor to part the sea...");
}

void loop() {
  // --- 1. המתנה לנס פתיחת הים ---
  if (!miracleStarted) {
    if (digitalRead(touchSensor) == HIGH) { 
      miracleStarted = true;
      Serial.println("STATE CHANGE: Miracle initiated - Sea is parting!");
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sea is Parting!");
      
      // פתיחת המחסום הראשונית כלפי מעלה
      barrierServo.write(110); 
      delay(750);              
      barrierServo.write(90);  
      barrierOpen = true;
      
      delay(2000); 
      lcd.clear();
      Serial.println("INFO: Sea is now open. Scanning for RFID and distance...");
    }
    return; 
  }

  // --- 2. קריאת חיישן לייזר (מצרים מתקרבים) ---
  distance = laserSensor.readRangeContinuousMillimeters() / 10; 
  if (distance >= 400 || distance <= 0) distance = 400; 

  // --- 3. בדיקת RFID (בני ישראל עוברים) ---
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    savedCount++;
    Serial.print("EVENT: Pass Authorized! Total saved: ");
    Serial.println(savedCount);
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pass Authorized!");
    delay(1500);
    lcd.clear();
    mfrc522.PCD_StopCrypto1(); 
  }

  // --- 4+5. תצוגה ואזהרה ---
  lcd.setCursor(0, 1);
  lcd.print("Saved: ");
  lcd.print(savedCount);

  if (distance > 0 && distance < 50) {
    // הדפסה למוניטור פעם אחת בלבד כשהסכנה מתחילה
    if (!isDangerState) {
      Serial.print("WARNING: Egyptians approach! Distance: ");
      Serial.print(distance);
      Serial.println(" cm");
      isDangerState = true;
    }
    
    // מצב סכנה: הבהוב מסך ונורה
    lcd.setCursor(0, 0);
    lcd.print("! HURRY UP !    "); 
    digitalWrite(ledPin, HIGH);
    delay(150);
    
    lcd.setCursor(0, 0);
    lcd.print("                "); 
    digitalWrite(ledPin, LOW);
    delay(150);
  } 
  else {
    // הדפסה למוניטור פעם אחת בלבד כשהסכנה חולפת
    if (isDangerState) {
      Serial.println("INFO: Area is clear again.");
      isDangerState = false;
    }
    
    // שגרה: תצוגת מרחק
    lcd.setCursor(0, 0);
    lcd.print("Dist: ");
    lcd.print(distance);
    lcd.print("cm      "); 
    digitalWrite(ledPin, LOW);
  }

  // --- 6. סגירת חירום סופית (כפתור מטה משה) ---
  if (digitalRead(emergencyBtn) == HIGH) {
    systemLocked = true;
    
    Serial.println("=====================================");
    Serial.println("STATE CHANGE: SEA CLOSED PERMANENTLY!");
    Serial.print("Final Saved Count: ");
    Serial.println(savedCount);
    Serial.println("=====================================");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("SEA CLOSED!");
    
    lcd.setCursor(0, 1);
    lcd.print("Total Saved: ");
    lcd.print(savedCount);
    
    if (barrierOpen) {
      barrierServo.write(70);  
      delay(750);              
      barrierServo.write(90);  
      barrierOpen = false;
    }
    
    digitalWrite(ledPin, HIGH); 
    
    while(systemLocked) {
      delay(100);
    }
  }
}