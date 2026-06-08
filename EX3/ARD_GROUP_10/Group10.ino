#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <VL53L1X.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

// Pin definitions for system components
const int ledPin = 3;       // LED
const int buttonPin = 4;    // Push Button - Start
const int servoPin = 9;     // Servo Motor
const int ldrPin = A0;      // Day/Light

// RFID module pin configuration
#define SS_PIN 10
#define RST_PIN 5
MFRC522 rfid(SS_PIN, RST_PIN);

// Component object initializations
LiquidCrystal_I2C lcd(0x27, 16, 2);
VL53L1X laser;
Servo myServo;

// --- Light threshold setting ---
// Night (covered) = high values, Day (light) = low values
const int LIGHT_THRESHOLD = 300; 

// System state variables for flow management
bool systemRunning = false;   
bool mosesIdentified = false; 
int lastButtonState = LOW;

// Physical state tracking for the sea barrier
enum SeaState { SEA_CLOSED, SEA_OPENED };
SeaState currentSeaState = SEA_CLOSED;

// Distance threshold for detection (in cm)
const float X_LIMIT = 30.0;   

// Precise timing values to ensure full 90-degree motor rotation
const int OPEN_ROTATION_TIME = 1200;   
const int CLOSE_ROTATION_TIME = 1500; 

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(400000);
  SPI.begin();
  rfid.PCD_Init(); // Initialize the RFID reader component

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(ldrPin, INPUT);

  digitalWrite(ledPin, LOW); // Turn the indicator LED off at startup

  myServo.attach(servoPin);
  myServo.write(90); // (sea starts closed)

  lcd.init();
  lcd.backlight(); // Turn on the LCD screen backlight
  
 // Display initial message when the system is off
  lcd.setCursor(0, 0);
  lcd.print("System: OFF     ");
  lcd.setCursor(0, 1);
  lcd.print("Press Start...  ");
  
 // Initialize the laser distance sensor
  laser.setTimeout(500);
  if (!laser.init()) {
    Serial.println("Laser Error!");
    while (1); 
  }
  laser.setDistanceMode(VL53L1X::Long);
  laser.setMeasurementTimingBudget(50000);
  laser.startContinuous(50);

  Serial.println("--- System Fully Operational ---");
}

void loop() {
  // 1. Read the Start button state with state-change detection
  int currentButtonState = digitalRead(buttonPin);
  if (currentButtonState == HIGH && lastButtonState == LOW) {
    systemRunning = !systemRunning; // system running state on/off
    
    if (!systemRunning) {
      // Reset variables and turn off LED when system stops
      mosesIdentified = false;
      digitalWrite(ledPin, LOW); 
      
      // Safety: Automatically close the sea barrier if it was left open
      if (currentSeaState == SEA_OPENED) {
        myServo.write(0); 
        delay(CLOSE_ROTATION_TIME);
        myServo.write(92);  // Stop the continuous servo motor
        currentSeaState = SEA_CLOSED;
      }
      // Update screen to show system is off
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("System: OFF");
    }
    delay(300); 
  }
  lastButtonState = currentButtonState;
// If the system is toggled off, exit the loop here and do nothing else
  if (!systemRunning) return;

 // 2. Read the physical RFID reader (Identify Moses)
  if (!mosesIdentified && rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    mosesIdentified = true; 
    Serial.println("Real RFID: Moses Identified!");
    rfid.PICC_HaltA(); // Stop reading the current RFID card
    lcd.clear();
  }

  // 3. System Flow Management
  if (!mosesIdentified) {
    // Step A: Moses is not identified yet (Access Denied)
    myServo.write(90);         
    digitalWrite(ledPin, LOW); 
    
    lcd.setCursor(0, 0);
    lcd.print("WaitingForMoses"); 
    lcd.setCursor(0, 1);
    lcd.print("AccessDenied    "); 
  } 
  else {
    // Moses is identified! Read sensor data
    int lightValue = analogRead(ldrPin); // Read light sensor value
    int distanceMM = laser.read();       // Read distance from laser sensor in mm
    float distanceCM = distanceMM / 10.0; // Convert distance to cm

    // Print values to the Serial Monitor for control and debugging    Serial.print("LDR Value: ");
    Serial.println(lightValue);

      // --- Laser safety override (If Egyptians approach - close immediately!) ---
    if (distanceCM < X_LIMIT) {
      lcd.setCursor(0, 0);
      lcd.print("Egyptians Near  "); 
      lcd.setCursor(0, 1);
      lcd.print("Sea Closing...  "); 
      
      // Close the sea barrier if it was currently open
      if (currentSeaState == SEA_OPENED) {
        myServo.write(0);         
        delay(CLOSE_ROTATION_TIME); 
        myServo.write(92);          
        currentSeaState = SEA_CLOSED;
      }
      
      digitalWrite(ledPin, LOW); // Turn off the indicator LED since the sea is closed
      
      // System lockout: Reset Moses status so a card swipe is required again
      mosesIdentified = false; 
      delay(1000); 
    } 
    else {
      // Egyptians are far away. Check light levels to see if opening is allowed (only at night!)
      
      if (lightValue >= LIGHT_THRESHOLD) {
        // [DARK / NIGHT] -> Conditions met! The Red Sea parts
        lcd.setCursor(0, 0);
        lcd.print("Sea Is          ");
        lcd.setCursor(0, 1);
        lcd.print("Opened          "); 

        // Open the sea barrier if it is currently closed
        if (currentSeaState == SEA_CLOSED) {
          myServo.write(180);          
          delay(OPEN_ROTATION_TIME); 
          myServo.write(88);         
          currentSeaState = SEA_OPENED;
        }
        
        digitalWrite(ledPin, HIGH);  // Turn on the indicator LED while the sea is open
      } 
      else {
        // [LIGHT / DAY] -> Moses is identified, but the sea cannot open during daylight!
        lcd.setCursor(0, 0);
        lcd.print("Moses Identified"); 
        lcd.setCursor(0, 1);
        lcd.print("WaitingForNight");

        // Safety: If it suddenly becomes day while the sea is open, close it safely
        if (currentSeaState == SEA_OPENED) {
          myServo.write(0);  
          delay(CLOSE_ROTATION_TIME);
          myServo.write(92);  
          currentSeaState = SEA_CLOSED;
        }

        digitalWrite(ledPin, LOW); // Turn off the indicator LED since the sea is closed
      }
    }
  }
  delay(50); 
}