#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo seaServo;

const int trigPin = 7;        
const int echoPin = 6;        
const int soundPin = A0;      
const int ledPin = 13;        
const int buttonPin = 2;      
const int irReceiverPin = 11; 

// Tuning parameter: Time in milliseconds to complete one full 360-degree rotation at max speed
const int fullRotationTime = 2200; 

enum SystemState {
  WAITING_FOR_USER,
  PENDING_AUTHENTICATION,
  AUTHORIZED_SEA_CLOSED,
  SEA_OPEN
};

SystemState currentState = WAITING_FOR_USER;

long duration;
int distance;
int soundLevel;
bool actionTriggered = false;

bool checkIRInput(int pin) {
  if (digitalRead(pin) == LOW) {
    return true; 
  }
  return false;
}

void setup() {
  Serial.begin(9600);
  
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(irReceiverPin, INPUT); 
  
  seaServo.attach(9);
  seaServo.write(90); // Hard stop baseline signal
  
  Wire.begin();
  lcd.init();
  lcd.clear();
  lcd.backlight();
  
  Serial.println("[SYSTEM] Moses Red Sea System Hardware Ready."); 
}

void loop() {
  // 1. Only read the distance sensor if we are actively waiting for a user to approach the shore
  if (currentState == WAITING_FOR_USER) {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    duration = pulseIn(echoPin, HIGH, 25000); 
    
    if (duration == 0) {
      distance = 999; 
    } else {
      distance = duration * 0.034 / 2; 
    }
  }

  // 2. Read environmental noise levels and button states continuously
  soundLevel = analogRead(soundPin);
  actionTriggered = (digitalRead(buttonPin) == LOW || soundLevel > 600);

  // 3. State Engine Execution Controller
  switch (currentState) {
    
    case WAITING_FOR_USER:
      if (distance >= 30) {
        lcd.setCursor(0, 0);
        lcd.print("Red Sea: CLOSED "); 
        lcd.setCursor(0, 1);
        lcd.print("Scanning Shore.."); 
      } 
      else if (distance > 0 && distance < 30) { 
        currentState = PENDING_AUTHENTICATION;
        lcd.clear();
        Serial.println("[STATE] Shore Boundary Breached. Halting distance sensor tracking."); 
      }
      break;

    case PENDING_AUTHENTICATION:
      lcd.setCursor(0, 0);
      lcd.print("Identify To Pass"); 
      lcd.setCursor(0, 1);
      lcd.print("The Red Sea     "); 
      
      if (checkIRInput(irReceiverPin)) {
        currentState = AUTHORIZED_SEA_CLOSED;
        lcd.clear();
        Serial.println("[AUTH] Token Accepted."); 
        delay(300); 
      }
      break;

    case AUTHORIZED_SEA_CLOSED:
      lcd.setCursor(0, 0);
      lcd.print("Click Or Shout  "); 
      lcd.setCursor(0, 1);
      lcd.print("To Open Red Sea "); 
      
      if (actionTriggered) {
        // --- OPENING MECHANICAL SEQUENCE ---
        seaServo.write(0);           // Fast counter-clockwise rotation (matching the original close direction)
        delay(fullRotationTime);     // Run long enough to complete a full 360 loop
        seaServo.write(90);          // Hard brake to stop instantly
        
        digitalWrite(ledPin, HIGH); 
        currentState = SEA_OPEN;
        
        Serial.println("[SEA] RED SEA IS NOW OPEN!"); 
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Red Sea is Open!");
        
        for (int i = 4; i > 0; i--) {
          lcd.setCursor(0, 1);
          lcd.print("Safe Gap: ");
          lcd.print(i);
          lcd.print("s    ");
          delay(1000); 
        }
        
        lcd.clear(); 
      }
      break;

    case SEA_OPEN:
      lcd.setCursor(0, 0);
      lcd.print("Click Or Shout  "); 
      lcd.setCursor(0, 1);
      lcd.print("To Close Sea    "); 
      
      if (actionTriggered) {
        // --- CLOSING MECHANICAL SEQUENCE ---
        seaServo.write(0);           // Fast counter-clockwise rotation
        delay(fullRotationTime);     // Run for the exact same duration to complete another loop
        seaServo.write(90);          // Hard brake to stop instantly
        
        digitalWrite(ledPin, LOW);  
        currentState = WAITING_FOR_USER; 
        lcd.clear();
        Serial.println("[SEA] Red Sea is now closed. Safe crossing completed."); 
        
        delay(2000); 
      }
      break;
  }

  delay(10); 
}