#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.h>

const int buttonPin   = 8;
const int greenLedPin = 6;
const int redLedPin   = 10;
const int irPin       = 7;
const int ldrPin      = A0;
const int soundPin    = A1;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo myServo;
Servo testServo;

// --- Her Principle 1: Explicit Direction & Calibration Commands ---
// If the servo slowly drifts or creeps during the stops, change 90 to 89 here.
#define SERVO_STOP  89   
#define SERVO_OPEN  180  // Full speed forward
#define SERVO_CLOSE 0    // Full speed backward

// --- Her Principle 2: Asymmetric Timing ---
// Time in milliseconds to simulate distance. She found closing takes 100ms longer!
const int OPEN_TIME = 280;   
const int CLOSE_TIME = 280;  


int state = 0;
unsigned long bakingDuration  = 0;
unsigned long timeRemaining   = 0;
unsigned long lastMillis      = 0;
unsigned long hideStartTime   = 0;
unsigned long successStartTime = 0;

const int LDR_PIN = A0;          // Where your light sensor is plugged in
const int LIGHT_THRESHOLD = 500; // Calibrate this number!
bool isAt90 = false;             // This is the memory trick! It tracks position.

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin, OUTPUT);
  IrReceiver.begin(irPin, ENABLE_LED_FEEDBACK);
  lcd.init();
  lcd.backlight();
  
   // Attach the servo to Pin 9
  testServo.attach(9);

  // Initialize: Ensure the motor starts completely stationary
  testServo.write(SERVO_STOP);
  delay(1000); // Pause 1 second before doing anything

  resetSystem();
}

void resetSystem() {
  state = 0;
  successStartTime = 0;
  digitalWrite(greenLedPin, LOW);
  digitalWrite(redLedPin, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Insert Dough");
  Serial.println("[STATUS] Ready");
}

void loop() {
  // Read the sensor ONCE at the top of the loop so all cases can use it
  int lightLevel = analogRead(LDR_PIN);

  switch (state) {
    case 0:
      if (digitalRead(buttonPin) == LOW) {
        delay(200);
        state = 1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Select Program:");
        lcd.setCursor(0, 1);
        lcd.print("1:2min | 2:3min");
        Serial.println("[STATUS] Select program");
      }
      break;

    case 1:
      if (IrReceiver.decode()) {
        int cmd = IrReceiver.decodedIRData.command;
        if (cmd == 69) {
          bakingDuration = 120000;
          lcd.clear();
          lcd.print("Selected: 2 min");
          Serial.println("[STATUS] Program selected: 2 min");
        } else if (cmd == 70) {
          bakingDuration = 180000;
          lcd.clear();
          lcd.print("Selected: 3 min");
          Serial.println("[STATUS] Program selected: 3 min");
        } else if (cmd == 28) {
          if (bakingDuration > 0) {
            timeRemaining = bakingDuration;
            state = 2;
            lastMillis = millis();
            lcd.clear();
            Serial.println("[STATUS] Baking started");
          }
        }
        IrReceiver.resume();
      }
      break;

    case 2:
      if (lightLevel > LIGHT_THRESHOLD && isAt90 == false) {
        testServo.attach(9);          // WAKE UP the servo!
        testServo.write(SERVO_OPEN);  // Spin forward
        delay(OPEN_TIME);             // Run for the time it takes to reach ~90 deg
        testServo.write(SERVO_STOP);  // Hard Stop
        delay(50);                    // Tiny pause to let the motor settle
        testServo.detach();           // PUT TO SLEEP! (Stops the electrical noise)
        
        isAt90 = true;                // Update memory
      }
      
      // --- IF DARK (and we are currently at 90) ---
      if (lightLevel <= LIGHT_THRESHOLD && isAt90 == true) {
        testServo.attach(9);          // WAKE UP the servo!
        testServo.write(SERVO_CLOSE); // Spin backward
        delay(CLOSE_TIME);            // Run for the time it takes to close
        testServo.write(SERVO_STOP);  // Hard Stop
        delay(50);                    // Tiny pause to let the motor settle
        testServo.detach();           // PUT TO SLEEP! (Stops the electrical noise)
        
        isAt90 = false;               // Update memory
      }
      
      if (millis() - lastMillis >= 1000) {
        lastMillis = millis();
        timeRemaining -= 1000;
        updateLCDTimer();
      }

      if (analogRead(soundPin) > 1000) {
        state = 3;
        hideStartTime = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("WARNING: ENEMY!");
        Serial.println("[ALERT] Enemy detected - hiding!");
      }

      if (timeRemaining <= 0 && state == 2) {
        state = 4;
        successStartTime = millis();
        digitalWrite(greenLedPin, HIGH);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kosher Success!");
        Serial.println("[STATUS] Baking complete!");
      }
      break;

    case 3:
      // Turn the red light SOLID on (no more weird blinking math!)
      if ((millis() / 250) % 2 == 0) {
              digitalWrite(redLedPin, HIGH);
            } else {
              digitalWrite(redLedPin, LOW);
            }      
      // Stay hidden for 3 seconds
      if (millis() - hideStartTime >= 3000) {
        state = 2;
        digitalWrite(redLedPin, LOW); // Turn off the red light
        lastMillis = millis();
        lcd.clear();
        updateLCDTimer();
        Serial.println("[STATUS] Enemy gone - resuming baking");
      }
      break;

    case 4:
      if (millis() - successStartTime >= 15000) {
        resetSystem();
      }
      if (digitalRead(buttonPin) == LOW) {
        delay(200);
        resetSystem();
      }
      break;
  }
  delay(50);
}

void updateLCDTimer() {
  int mins = (timeRemaining / 1000) / 60;
  int secs = (timeRemaining / 1000) % 60;
  lcd.setCursor(0, 0);
  lcd.print("Baking...");
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  if (mins < 10) lcd.print("0");
  lcd.print(mins);
  lcd.print(":");
  if (secs < 10) lcd.print("0");
  lcd.print(secs);
}