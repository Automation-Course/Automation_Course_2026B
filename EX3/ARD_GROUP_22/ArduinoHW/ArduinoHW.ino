#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "HX711.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// --- PIN DEFINITIONS ---
#define RFID_SS_PIN       10
#define RFID_RST_PIN      -1 
const int BUTTON_PIN = 8;
const int TRIG_PIN = 7;
const int ECHO_PIN = 6;
const int SERVO_PIN = 9;

const int HX711_DOUT_PIN = A2; 
const int HX711_SCK_PIN  = A3;

// LED Pins
const int GREEN_LED_PIN = 2;
const int RED_LED_PIN = 3;

// --- CONFIGURATION CONSTANTS ---
const String AUTHORIZED_UID = "63 31 1A AA";     // white keycard uuid
const float SCALE_CALIBRATION_FACTOR = 317.21;   // scale calibration factor
const float WEIGHT_LIMIT = 300.0;                // Weight limit threshold in grams
const int DISTANCE_THRESHOLD = 1;                // Stop arm when closer than this
const unsigned long SERVO_SAFETY_TIMEOUT = 5000; // Force stop servo after 5 seconds max
const int POST_STOP_DELAY = 1000;                // Dynamic hold time (1 sec) after arm stops

// --- INSTANCES & VARIABLES ---
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);
HX711 scale;
Servo myServo;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change 0x27 to 0x3F if LCD doesn't respond

bool isVerified = false; 
float savedWeight = 0.0;

// Function prototype
int getDistance();

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(SCALE_CALIBRATION_FACTOR);
  scale.tare(); // Zero out scale on boot

  pinMode(BUTTON_PIN, INPUT_PULLUP); // Active LOW: Pressed = LOW
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");

  myServo.attach(SERVO_PIN);
  myServo.write(90);
  Serial.println(F("===================================="));
  Serial.println(F("    STREAMLINED AUTOMATION SYSTEM  "));
  Serial.println(F("===================================="));
  Serial.println(F("Ready. Scan RFID card to start..."));
}

void loop() {
  
  // STATE 1: Waiting for a valid RFID scan
  if (!isVerified) {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      String scannedUID = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        scannedUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        scannedUID.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      scannedUID.toUpperCase();
      scannedUID.trim();

      lcd.clear();
      lcd.setCursor(0, 0);
      if (scannedUID == AUTHORIZED_UID) {
        Serial.println(F("🔓 RFID Verified! Press button to capture weight..."));
        lcd.print("jewish");
        isVerified = true;
      } else {
        Serial.println(F("❌ Unknown Card. Access Denied."));
        lcd.print("not jewish");
        digitalWrite(RED_LED_PIN, HIGH);
        delay(2000);
        digitalWrite(RED_LED_PIN, LOW);
        lcd.clear();
        lcd.print("Scan RFID...");
      }
      mfrc522.PICC_HaltA();
      delay(1000);
    }
  } 
  
  // STATE 2: Verified! Waiting for button press to weigh and run servo
  else {
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(50);
      while(digitalRead(BUTTON_PIN) == LOW); // Wait for release

      Serial.println(F("Measuring payload..."));
      lcd.setCursor(0, 1);
      lcd.print("Weighing...     ");
      
      if (scale.is_ready()) {
        savedWeight = abs(scale.get_units(10)); 
        Serial.print(F("Weight Captured: "));
        Serial.print(savedWeight, 1);
        Serial.println(F(" g"));

        // STATE 3: Weight evaluation & Servo routine execution
        if (savedWeight < WEIGHT_LIMIT) {
          Serial.println(F("Weight condition met! Executing servo routine..."));
          
          // Clear screen to apply your layout cleanly
          lcd.clear();
          
          // Row 1 layout: "jwish wt: XX.Xg"
          lcd.setCursor(0, 0);
          lcd.print("jwish wt:");
          lcd.print(savedWeight, 1);
          lcd.print("g");
          
          // Row 2 layout: "You shall pass!!"
          lcd.setCursor(0, 1);
          lcd.print("You shall pass!!");
          
          // --- LED INDICATOR: VALID WEIGHT ---
          digitalWrite(GREEN_LED_PIN, HIGH);
          digitalWrite(RED_LED_PIN, LOW);

          // 1. Spin Counterclockwise for 1 second
          myServo.write(120); 
          delay(900);

          // 2. Stop for 2 seconds
          myServo.write(90); 
          delay(4000);

          // 3. Rotate Clockwise until the distance sensor detects the arm is close
          Serial.println(F("Returning arm home..."));
          myServo.write(80); 
          
          unsigned long servoStartTime = millis();
          
          while (true) {
            int currentDistance = getDistance();
            
            if (currentDistance != 999) {
              Serial.print(F("Arm Distance: ")); Serial.print(currentDistance); Serial.println(F(" cm"));
            } else {
              Serial.println(F("⚠️ Sensor timeout... retrying"));
            }
            
            if (currentDistance > 0 && currentDistance <= DISTANCE_THRESHOLD) {
              myServo.write(90); // Stop the servo motor
              Serial.println(F("🎯 Arm home position reached."));
              break; 
            }

            if (millis() - servoStartTime > SERVO_SAFETY_TIMEOUT) {
              myServo.write(90);
              Serial.println(F("🚨 SAFETY ERROR: Distance sensor unresponsive. Servo forced to stop."));
              break;
            }
            
            delay(60); 
          }
          
          // DYNAMIC HOLD: Wait exactly 1 second AFTER the arm has stopped tracking
          delay(POST_STOP_DELAY);

        } else {
          Serial.println(F("⚠️ OVERWEIGHT! Servo routine bypassed."));
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("jewish wt:");
          lcd.print(savedWeight, 1);
          lcd.print("g");
          
          lcd.setCursor(0, 1);
          lcd.print("OVERWEIGHT!     ");
          
          // --- LED INDICATOR: OVERWEIGHT ---
          digitalWrite(RED_LED_PIN, HIGH);
          digitalWrite(GREEN_LED_PIN, LOW);
          
          delay(4000); 
        }

        // Reset the system state back to the beginning for the next scan cycle
        Serial.println(F("\nCycle complete. Resetting state for next user...\n"));
        
        // Hold the final LED state and LCD message for 3 seconds before wiping
        delay(3000); 
        
        // Turn everything off upon returning to "System Ready"
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
        
        lcd.clear();
        lcd.print("System Ready");
        isVerified = false;
        savedWeight = 0;
        delay(1000);
      } else {
        Serial.println(F("Error: Scale hardware not responsive."));
        lcd.setCursor(0, 1);
        lcd.print("Scale Error     ");
      }
    }
  }
}

// Helper function for ultrasonic readings
int getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5); 
  
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 25000); 
  
  if (duration == 0) {
    return 999; 
  }
  
  return duration * 0.0343 / 2;
}