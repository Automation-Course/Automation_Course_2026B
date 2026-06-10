#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.hpp>

// --- States for the finite state machine ---
enum SystemState {
  STATE_WAIT_RFID,
  STATE_WAIT_INPUT,
  STATE_CHECK_SAFETY,
  STATE_WATER_FLOWING
};

void transitionTo(SystemState newState);
SystemState currentState = STATE_WAIT_RFID;

// --- Pin mapping ---
const int PIN_IR_RECEIVER   = 9; 
const int PIN_MIC_BUTTON    = 2;
const int PIN_HIT_BUTTON    = 3; 
const int PIN_TRIG          = 6;
const int PIN_ECHO          = 7;
const int PIN_SERVO         = 5;
const int PIN_LED           = 4;

// --- Servo and timing constants ---
const int  SERVO_STOP         = 90;   // Stop point for continuous servo
const int  SERVO_MAX_SPEED    = 180;  // Maximum rotation speed!
const long JUG_DISTANCE_CM    = 5;
const int  WATER_FLOW_TIME_MS = 5000;
const int  ERROR_DISPLAY_MS   = 2000;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo waterValve;

// --- Variables to save previous button states ---
bool lastMicState  = HIGH;
bool lastHitState  = HIGH;

// --- Calculates distance in cm using the ultrasonic sensor ---
long measureDistanceCM() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout
  long distance = duration * 0.034 / 2;
  return distance;
}

// --- Helper to clear LCD and print 2 lines quickly ---
void displayMessage(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

// --- Starts the continuous servo spinning ---
void openValve() {
  waterValve.attach(PIN_SERVO);
  waterValve.write(SERVO_MAX_SPEED);  // Spins fast and continuously!
  
  digitalWrite(PIN_LED, HIGH);
  Serial.println("[VALVE] Valve OPENED - water flowing continuously");
}

// --- Stops the continuous servo ---
void closeValve() {
  waterValve.write(SERVO_STOP); // Stops the rotation
  delay(50);
  waterValve.detach();            
  
  digitalWrite(PIN_LED, LOW);
  Serial.println("[VALVE] Valve CLOSED");
}

// --- Handles state switching, prints to serial and updates LCD ---
void transitionTo(SystemState newState) {
  currentState = newState;
  switch (newState) {
    case STATE_WAIT_RFID:
      displayMessage("Welcome Moses!", "Activate System");
      Serial.println("[STATE] --> WAIT_RFID (Waiting for IR Remote)");
      break;
    case STATE_WAIT_INPUT:
      displayMessage("Speak to Rock!", "");
      Serial.println("[STATE] --> WAIT_INPUT");
      break;
    case STATE_CHECK_SAFETY:
      displayMessage("Place the Jug", "Under the Rock");
      Serial.println("[STATE] --> CHECK_SAFETY");
      break;
    case STATE_WATER_FLOWING:
      displayMessage("Water Flowing!", "Miracle Done :)");
      Serial.println("[STATE] --> WATER_FLOWING");
      break;
  }
}

// --- Hardware setup ---
void setup() {
  Serial.begin(9600);
  Serial.println("=== Smart Rock System Initializing ===");

  IrReceiver.begin(PIN_IR_RECEIVER, ENABLE_LED_FEEDBACK);

  pinMode(PIN_MIC_BUTTON,  INPUT_PULLUP);
  pinMode(PIN_HIT_BUTTON,  INPUT_PULLUP);
  pinMode(PIN_TRIG,        OUTPUT);
  pinMode(PIN_ECHO,        INPUT);
  pinMode(PIN_LED,         OUTPUT);
  digitalWrite(PIN_LED,    LOW);

  waterValve.attach(PIN_SERVO);
  waterValve.write(SERVO_STOP);
  delay(100);
  waterValve.detach();

  lcd.init();
  lcd.backlight();

  transitionTo(STATE_WAIT_RFID);
  Serial.println("=== System Ready ===");
}

// --- Main loop running the state machine ---
void loop() {
  bool currentMicState  = digitalRead(PIN_MIC_BUTTON);
  bool currentHitState  = digitalRead(PIN_HIT_BUTTON);

  // --- Global check for hitting the rock (Only active after system activation) ---
  if (currentHitState == LOW && lastHitState == HIGH && currentState != STATE_WAIT_RFID) {
    delay(50); // Debounce
    if (digitalRead(PIN_HIT_BUTTON) == LOW) {
      Serial.println("[ERROR] Hit detected globally! Invalid action");
      
      displayMessage("Wrong Action!", "Speak, Dont Hit!");
      delay(ERROR_DISPLAY_MS); 
      
      transitionTo(currentState); 
    }
  } 
  else {
    // --- Continue normal operation ---
    switch (currentState) {

      case STATE_WAIT_RFID: {
        if (IrReceiver.decode()) {
          Serial.print("[IR] Button pressed! Command: ");
          Serial.println(IrReceiver.decodedIRData.command); 
          
          transitionTo(STATE_WAIT_INPUT);
          
          delay(500); 
          IrReceiver.resume(); 
        }
        break;
      }

      case STATE_WAIT_INPUT: {
        if (currentMicState == LOW && lastMicState == HIGH) {
          delay(50); 
          if (digitalRead(PIN_MIC_BUTTON) == LOW) {
            Serial.println("[INPUT] Valid voice input detected");
            transitionTo(STATE_CHECK_SAFETY);
          }
        }
        break;
      }

      case STATE_CHECK_SAFETY: {
        long distance = measureDistanceCM();
        
        bool jugDetected = (distance > 0 && distance <= JUG_DISTANCE_CM);
        if (jugDetected) {
          Serial.println("[SAFETY] Jug detected - opening valve");
          transitionTo(STATE_WATER_FLOWING);
          
          openValve(); 
          delay(WATER_FLOW_TIME_MS); 
          closeValve(); 
          
          Serial.println("[SYSTEM] Cycle complete - resetting");
          
          // Clear IR receiver buffer before resetting
          while (IrReceiver.decode()) {
            IrReceiver.resume();
          }
          currentMicState = digitalRead(PIN_MIC_BUTTON);
          currentHitState = digitalRead(PIN_HIT_BUTTON);
          
          transitionTo(STATE_WAIT_RFID);
        } else {
          delay(50); 
        }
        break;
      }

      case STATE_WATER_FLOWING: {
        break;
      }
    }
  }

  // Save current states for the next loop
  lastMicState  = currentMicState;
  lastHitState  = currentHitState;
}
