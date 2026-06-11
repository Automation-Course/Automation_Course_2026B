#include <Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>

// הגדרת פינים
const int irPin = 9;
const int micPin = 2; 
const int resetBtn = 4;
const int servoPin = 6;
const int yellowLedPin = 7; // נורה צהובה מציינת ים פתוח
const int redLedPin = 8;    // נורה אדומה עבור אזהרה או חירום
const int ldrPin = 3;       // החיישן הדיגיטלי

byte lightIcon[8] = {
  0b00110,
  0b00100,
  0b01100,
  0b11111,
  0b00110,
  0b00100,
  0b01000,
  0b00000
};

// אובייקטים
LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo seaServo;

const unsigned long MOSES_CODE = 0xE31CFF00;

// תיאור מצבים
enum State { WAITING, MOSES_MSG, WAITING_FOR_LIGHT, LISTENING, OPEN, COUNTDOWN, END_MSG };
State currentState = WAITING;

// משתנים לשמירת זמנים וניהול הלוגיקה
unsigned long stateStartTime = 0;
unsigned long previousBlinkTime = 0;
int countdownValue = 3;
bool isManualClose = false;
bool ledState = LOW;

void changeState(State newState);

// פונקציית ההגדרות שרצה פעם אחת בעת הדלקת המערכת
void setup() {
  Serial.begin(9600);
  Serial.println("System starting...");
  
  lcd.init();
  lcd.backlight();
  
  // רישום האייקון למסך
  lcd.createChar(0, lightIcon);
  
  pinMode(resetBtn, INPUT_PULLUP);
  pinMode(micPin, INPUT_PULLUP); 
  pinMode(yellowLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  pinMode(ldrPin, INPUT); // החיישן הדיגיטלי מוגדר כקלט
  
  IrReceiver.begin(irPin);
  
  seaServo.attach(servoPin);
  seaServo.write(90); 
  
  changeState(WAITING);
}

// הלולאה המרכזית שרצה באופן רציף 
void loop() {
  runStateMachine();
}

// הפונקציה שמנהלת את כל הלוגיקה של המערכת
void runStateMachine() {
  switch (currentState) {
    case WAITING:
      // מצב המתנה לפקודה ראשונית מהשלט
      seaServo.write(90); 
      digitalWrite(yellowLedPin, LOW);
      digitalWrite(redLedPin, LOW);
      
      if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.decodedRawData == MOSES_CODE) {
          changeState(MOSES_MSG);
        }
        IrReceiver.resume();
      }
      break;

    case MOSES_MSG:
      // המתנה של שתי שניות וחצי לפני שמחכים לאות
      if (millis() - stateStartTime >= 2500) changeState(WAITING_FOR_LIGHT);
      break;

    case WAITING_FOR_LIGHT:
      // מחכים שהמשתמש יאיר עם פנס (LOW = מואר)
      if (digitalRead(ldrPin) == LOW) {
        changeState(LISTENING);
      }
      break;

    case LISTENING:
      // האזנה למיקרופון מופעל רק אם עברה חצי שניה כדי למנוע קריאות כפולות
      if (millis() - stateStartTime > 500 && digitalRead(micPin) == LOW) {
        changeState(OPEN);
      }
      break;

    case OPEN:
      // פתיחת הים על ידי סיבוב מנוע רציף קדימה לחצי שניה ואז עצירה שלו
      if (millis() - stateStartTime < 500) seaServo.write(180);
      else seaServo.write(90);

      digitalWrite(yellowLedPin, HIGH); // נורה צהובה דולקת קבוע
      digitalWrite(redLedPin, LOW);
      
      // בדיקה אם נלחץ כפתור חירום או שעבר הזמן
      if (digitalRead(resetBtn) == LOW) {
        Serial.println("Emergency button pressed in OPEN state!");
        isManualClose = true;
        changeState(END_MSG);
      } else if (millis() - stateStartTime >= 7000) {
        changeState(COUNTDOWN);
      }
      break;

    case COUNTDOWN:
      seaServo.write(90); // מוודאים שהים נשאר פתוח
      
      // הבהוב סירנה לסירוגין מבוסס זמן ולא לולאה חוסמת
      if (millis() - previousBlinkTime >= 250) {
        previousBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(redLedPin, ledState);
        digitalWrite(yellowLedPin, !ledState);
      }
      
      // עדכון הספירה לאחור בכל שניה שחולפת או עצירת חירום
      if (digitalRead(resetBtn) == LOW) {
        Serial.println("Emergency button pressed in COUNTDOWN state!");
        isManualClose = true;
        changeState(END_MSG);
      } else if (millis() - stateStartTime >= 1000) {
        stateStartTime = millis();
        countdownValue--;
        if (countdownValue > 0) {
          lcd.setCursor(12, 0);
          lcd.print(countdownValue);
        } else {
          isManualClose = false;
          changeState(END_MSG);
        }
      }
      break;

    case END_MSG:
      // סגירת הים על ידי סיבוב המנוע לאחור לחצי שניה ואז עצירה
      if (millis() - stateStartTime < 500) seaServo.write(0);
      else seaServo.write(90);
      
      // הבהוב נורות מהיר במצב סיום
      if (millis() - previousBlinkTime >= 100) {
        previousBlinkTime = millis();
        ledState = !ledState;
        digitalWrite(redLedPin, ledState);
        digitalWrite(yellowLedPin, !ledState);
      }
      
      // חזרה למצב המתנה אחרי שלוש שניות
      if (millis() - stateStartTime >= 3000) {
        changeState(WAITING);
      }
      break;
  }
}

// פונקציית עזר למעבר מסודר בין המצבים השונים
void changeState(State newState) {
  currentState = newState;
  stateStartTime = millis();
  countdownValue = 3;
  lcd.clear();
  
  Serial.print("State changed to: ");
  switch (currentState) {
    case WAITING:
      Serial.println("WAITING (Idle)");
      lcd.print("Waiting for");
      lcd.setCursor(0, 1); lcd.print("Moses...");
      break;
      
    case MOSES_MSG:
      Serial.println("MOSES_MSG (IR Detected)");
      lcd.print("Moses Detected!");
      break;
      
    case WAITING_FOR_LIGHT:
      Serial.println("WAITING_FOR_LIGHT (Signal needed)");
      lcd.print("Signal needed!");
      lcd.setCursor(0, 1);
      lcd.write(byte(0));   // סמל לפני הטקסט
      lcd.print(" Use Flashlight");
      break;
      
    case LISTENING:
      Serial.println("LISTENING (Ready for noise)");
      lcd.print("Signal Arrived!");
      lcd.setCursor(0, 1); lcd.print("Make Noise!");
      break;
      
    case OPEN:
      Serial.println("OPEN (Sea is open)");
      lcd.print("Sea is OPEN!");
      lcd.setCursor(0, 1); lcd.print("Cross Safely!");
      break;
      
    case COUNTDOWN:
      Serial.println("COUNTDOWN (Warning sequence)");
      lcd.print("Closing in: 3");
      break;
      
    case END_MSG:
      Serial.println("END_MSG (Process complete)");
      if (isManualClose) {
        lcd.print("Sea Closed!");
        lcd.setCursor(0, 1); lcd.print("Egyptians drown!");
      } else {
        lcd.print("Time is UP!");
        lcd.setCursor(0, 1); lcd.print("GAME OVER!");
      }
      break;
  }
}