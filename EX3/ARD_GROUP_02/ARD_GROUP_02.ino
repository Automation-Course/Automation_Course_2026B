#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HX711.h>
#include <IRremote.hpp>

// ---------- מסך ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------- מנוע ----------
Servo servoMotor;

// ---------- חיבורי הפינים במטריצה ----------
const int tempPin = 9;          // חיישן טמפרטורה
const int hxDoutPin = 8;        // חיישן משקל נתונים
const int hxSckPin = 7;         // חיישן משקל  

const int ledPin = 12;          // נורה
const int buttonPin = 2;        // כפתור הפעלה
const int irReceiverPin = 6;    // חיישן שלט
const int servoPin = 11;        // מנוע

// ---------- הגדרת חיישן טמפרטורה ----------
OneWire oneWire(tempPin);
DallasTemperature tempSensor(&oneWire);

// ---------- הגדרת חיישן משקל ----------
HX711 scale;

// פקטור הכיול שהתאמנו למשקל בוחן 200 גרם ששקלנו
float calibrationFactor = 410.7;

// ---------- משתני המערכת ----------
int targetWeight = 0;           // משקל היעד המבוקש (100 או 200) תלוי בסוג המצה
int currentWeight = 0;          // המשקל שיש כרגע על החיישן
bool targetSelected = false;    // האם נבחר כבר סוג מצה?

// טווחי טמפרטורה מותרים
const float minTemp = 20.0;
const float maxTemp = 27.0;

// סטיית משקל מותרת (10 גרם )
const int tolerance = 10; 

// ==========================================
// פונקציית ההגדרות - רצה פעם אחת כשהלוח נדלק
// ==========================================
void setup()
{
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  servoMotor.attach(servoPin);
  servoMotor.write(90); // עצירת המנוע (לסרוו 360, 90 זה עצירה)

  tempSensor.begin();

  scale.begin(hxDoutPin, hxSckPin);
  scale.set_scale(calibrationFactor);
  scale.tare(); // איפוס המשקל ל-0

  // הפעלת השלט וביטול ההבהוב כדי שלא יפריע לנורה שלנו בפין 12
  IrReceiver.begin(irReceiverPin, DISABLE_LED_FEEDBACK);

  digitalWrite(ledPin, LOW); // מוודאים שהנורה כבויה בהתחלה

  // -- הדפסה לסריאל --
  Serial.println(F("------------------------------------"));
  Serial.println(F("--- Matzah Control System Online ---"));
  Serial.println(F("------------------------------------"));
  Serial.println(F("Initializing System Components..."));
  
  lcd.setCursor(0, 0);
  lcd.print(F("Matzah System"));
  lcd.setCursor(0, 1);
  lcd.print(F("Starting..."));
  delay(1500);
  lcd.clear();
}

// ==========================================
// הלולאה הראשית - רצה ללא הפסקה
// ==========================================
void loop()
{
  float temperature = readTemperature();

  // שלב א' - בדיקה שהטמפרטורה בטווח התקין
  if (!isTemperatureValid(temperature)) {
    stopSystemTemperatureWarning(temperature);
    return; // עוצר את הלולאה ומתחיל מחדש
  }

  // שלב ב' - המתנה לבחירת סוג מצה על ידי לחיצה על כפתור בשלט
  if (!targetSelected) {
    showWaitForTargetScreen(temperature);
    readTargetFromIR();
    return;
  }

  // שלב ג' - קריאת המשקל הנוכחי
  currentWeight = readWeight();

  // שלב ד' - בדיקה האם המשקל מתאים ליעד
  bool weightOK = isWeightValid(currentWeight, targetWeight);

  showWorkingScreen(currentWeight, targetWeight, weightOK);
  
  // מדליק או מכבה את הנורה לפי תקינות המשקל
  updateLed(weightOK);

  // שלב ה' - אם המשקל תקין ולחצו על הכפתור -> מפעילים את המנוע
  if (weightOK && digitalRead(buttonPin) == LOW) {
    approveDough();       
    askAnotherMatzah();   
  }

  delay(300); // השהיה קטנה כדי לא להעמיס על הלוח
}

// ---------- פונקציות טמפרטורה ----------
// ---------- קרא את הטמפרטורה בחדר ----------
float readTemperature()
{
  tempSensor.requestTemperatures();
  return tempSensor.getTempCByIndex(0);
}


// ---------- בדוק האם הטמפרטורה תקינה ----------
bool isTemperatureValid(float temperature)
{
  if (temperature == DEVICE_DISCONNECTED_C) return false;
  return (temperature >= minTemp && temperature <= maxTemp);
}


// ---------- עצור מערכת כשהטמפרטורה לא תקינה ----------
void stopSystemTemperatureWarning(float temperature)
{
  digitalWrite(ledPin, LOW);
  targetSelected = false;
  targetWeight = 0;
  servoMotor.write(90); 

  // -- הדפסה לסריאל --
  Serial.print(F("[ERROR] Temperature Alert! Current Temp: "));
  Serial.println(temperature);

  lcd.setCursor(0, 0);
  lcd.print(F("Temp Alert!     "));
  lcd.setCursor(0, 1);

  if (temperature == DEVICE_DISCONNECTED_C) {
    lcd.print(F("Sensor Error    "));
  } else {
    lcd.print(F("Temp:"));
    lcd.print(temperature, 1);
    lcd.print(F("C     "));
  }
  delay(700);
}

// ---------- פונקציות מסך ושלט ----------
void showWaitForTargetScreen(float temperature)
{
  static unsigned long lastSerialPrint = 0;
  
  digitalWrite(ledPin, LOW);
  lcd.setCursor(0, 0);
  lcd.print(F("Temp OK: "));
  lcd.print(temperature, 1);
  lcd.print(F("C "));
  lcd.setCursor(0, 1);
  lcd.print(F("Press IR remote "));

  // -- הדפסה לסריאל (רק פעם ב-2 שניות כדי לא חהעמיס בהדפסות) --
  if (millis() - lastSerialPrint > 2000) {
    Serial.print(F("[INFO] System Ready. Temp OK ("));
    Serial.print(temperature, 1);
    Serial.println(F("C). Waiting for IR command..."));
    lastSerialPrint = millis();
  }
}


// ---------- קרא לחיצה מהשלט ----------
void readTargetFromIR()
{
  static unsigned long lastPressTime = 0; // מניעת לחיצות כפולות
  
  if (IrReceiver.decode()) {
    // מתעלם מלחיצה ארוכה מדי
    if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
      IrReceiver.resume();
      return;
    }
    
    // מוודא שעברה חצי שנייה לפחות מהלחיצה הקודמת
    if (millis() - lastPressTime > 700) {
      lastPressTime = millis();
      int command = IrReceiver.decodedIRData.command;
      
      // כפתור 1 - מצה דקה (100 גרם)
      if (command == 69) {
        targetWeight = 100;
        
        // -- הדפסה לסריאל --
        Serial.println(F("[INPUT] IR Button 1 Pressed -> Selected: Thin Matzah (100g)"));
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Thin Matzah"));
        lcd.setCursor(0, 1);
        lcd.print(F("Target: 100GR"));
        targetSelected = true;
        delay(1500);
        lcd.clear();
      } 
      // כפתור 2 - מצה עבה (200 גרם)
      else if (command == 70) {
        targetWeight = 200;
        
        // -- הדפסה לסריאל --
        Serial.println(F("[INPUT] IR Button 2 Pressed -> Selected: Rich Matzah (200g)"));
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Rich Matzah"));
        lcd.setCursor(0, 1);
        lcd.print(F("Target: 200GR"));
        targetSelected = true;
        delay(1500);
        lcd.clear();
      }
    }
    IrReceiver.resume(); // מאפס את השלט לקליטה הבאה
  }
}

// ---------- פונקציות משקל ----------
int readWeight()
{
  float weight = scale.get_units(10); // ממוצע של 10 קריאות לדיוק
  if (weight < 0) weight = 0;         // מונע הצגת מינוס אם החיישן רועד
  return (int)(weight + 0.5);         // מעגל את המספר
}


// ---------- בדוק משקל תקין ----------
bool isWeightValid(int currentWeight, int targetWeight)
{
  // בודק אם המשקל נמצא בתוך טווח הסטייה שהגדרנו למעלה (פלוס/מינוס 10)
  return (currentWeight >= targetWeight - tolerance &&
          currentWeight <= targetWeight + tolerance);
}

// ---------- הצג משקל על המסך ותקינותו ----------
void showWorkingScreen(int weight, int target, bool weightOK)
{
  // מציג את המשקל הנוכחי מול היעד במסך
  lcd.setCursor(0, 0);
  lcd.print(F("W:"));
  lcd.print(weight);
  lcd.print(F(" T:"));
  lcd.print(target);
  lcd.print(F("GR   "));

  lcd.setCursor(0, 1);
  if (weightOK) {
    lcd.print(F("W:OK Press Btn  ")); 
  } else {
    lcd.print(F("Adjust weight   "));
  }

  // -- הדפסה לסריאל של השקילה בזמן אמת --
  Serial.print(F("[SCALE] Current: "));
  Serial.print(weight);
  Serial.print(F("g | Target: "));
  Serial.print(target);
  if (weightOK) {
    Serial.println(F("g  => STATUS: OK! (Waiting for Button)"));
  } else {
    Serial.println(F("g  => STATUS: Adjusting..."));
  }
}


// ---------- פונקציות נורה ----------
void updateLed(bool weightOK)
{
  // מדליק את הנורה אם המשקל תקין
  if (weightOK) digitalWrite(ledPin, HIGH);
  else digitalWrite(ledPin, LOW);
}

// ---------- פונקציות פעולה וסיום ----------
void approveDough()
{
  // -- הדפסה לסריאל --
  Serial.println(F("[ACTION] Button Pressed! Dough Approved."));
  Serial.println(F("[ACTION] Motor is moving..."));

  // שומר על הנורה דלוקה בזמן הפעולה
  digitalWrite(ledPin, HIGH); 

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Dough Approved"));
  lcd.setCursor(0, 1);
  lcd.print(F("Servo Moving..."));

  // סיבוב המנוע קדימה למשך 1.2 שניות, ואז עצירה
  servoMotor.write(180); 
  delay(1200);           
  servoMotor.write(90);  
  
  // כיבוי הנורה עם סיום תנועת המנוע
  digitalWrite(ledPin, LOW); 
  delay(700);

  // -- הדפסה לסריאל --
  Serial.println(F("[ACTION] Motor stopped. Matzah is Ready!"));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Matzah Ready!"));
  delay(1500);
}


// ---------- שאל ליצירת מצה נוספת ----------
void askAnotherMatzah()
{
  // -- הדפסה לסריאל --
  Serial.println(F("[PROMPT] Waiting for User: Another Matzah? (1=Yes, 2=No)"));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Another Matzah?"));
  lcd.setCursor(0, 1);
  lcd.print(F("1:Yes      2:No"));

  bool decisionMade = false;
  unsigned long lastPressTime = millis();
  
  IrReceiver.resume();

  // מחכה להחלטה מהשלט
  while (!decisionMade) {
    if (IrReceiver.decode()) {
      if (!(IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)) {
        if (millis() - lastPressTime > 500) {
          lastPressTime = millis();
          int command = IrReceiver.decodedIRData.command;
          
          if (command == 69) { // כפתור 1 - מתחילים מצה חדשה
            // -- הדפסה לסריאל --
            Serial.println(F("[INPUT] User chose YES. Starting new cycle."));
            decisionMade = true;
            resetProcess();
          } 
          else if (command == 70) { // כפתור 2 - מסיימים ונועלים מערכת
            // -- הדפסה לסריאל --
            Serial.println(F("[INPUT] User chose NO."));
            Serial.println(F("------------------------------------"));
            Serial.println(F("[SYSTEM] Halting System. Happy Passover!"));
            Serial.println(F("------------------------------------"));

            decisionMade = true;
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(F("Happy Passover!"));

            // תוקע את המערכת בלולאה אינסופית כסיום
            while(true) {
              delay(1000);
            }
          }
        }
      }
      IrReceiver.resume();
    }
  }
}


// ---------- אתחול התהליך ----------
void resetProcess()
{
  // -- הדפסה לסריאל --
  Serial.println(F("[SYSTEM] Resetting Process. Zeroing scale..."));

  // מנקה את כל הנתונים לקראת הסבב הבא
  targetWeight = 0;
  currentWeight = 0;
  targetSelected = false;

  digitalWrite(ledPin, LOW);
  servoMotor.write(90); 

  //  הודעת אזהרה לא לגעת כדי לאפשר למתכת של החיישן להירגע לכיול תקין של המשקל
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Zeroing Scale..."));
  lcd.setCursor(0, 1);
  lcd.print(F("Hands off!      "));

  // המתנה של שנייה וחצי להתייצבות, ורק אז איפוס מוחלט
  delay(1500); 
  scale.tare(); 

  // -- הדפסה לסריאל --
  Serial.println(F("[SYSTEM] Tare complete. Ready for new input."));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("New Matzah...   "));
  delay(1200);
  lcd.clear();
}

