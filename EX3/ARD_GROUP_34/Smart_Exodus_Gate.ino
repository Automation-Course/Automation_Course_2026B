/**
 * Project: Biometric Border Gate to Israel (Exodus 3) - PHYSICAL HARDWARE VERSION
 * Course: Automation & Computer Integrated Manufacturing, BGU
 * 
 * Description:
 * This code is optimized and cleaned specifically for the PHYSICAL hardware.
 * All simulation code has been removed.
 * Memory optimizations have been applied to fit on Arduino Uno (ATmega328P).
 * 
 * WIRING INSTRUCTIONS:
 *  - AS608 Fingerprint Sensor:
 *    * VCC -> 5V (or 3.3V depending on AS608 module specs, check datasheet!)
 *    * GND -> GND
 *    * TX of sensor -> Pin 8 (Arduino RX)
 *    * RX of sensor -> Pin 11 (Arduino TX via a voltage divider if 3.3V logic is required)
 *  - HC-SR04 Ultrasonic Sensor:
 *    * VCC -> 5V
 *    * GND -> GND
 *    * Trig -> Pin 2
 *    * Echo -> Pin 3
 *  - IR Receiver:
 *    * VCC -> 5V
 *    * GND -> GND
 *    * OUT -> Pin 4
 *  - Emergency Pushbutton:
 *    * One terminal -> Pin 5
 *    * Other terminal -> GND (Internal pullup is enabled in code)
 *  - LEDs:
 *    * Green LED -> Pin 6 (via 220 ohm resistor to GND)
 *    * Red LED -> Pin 7 (via 220 ohm resistor to GND)
 *  - Servo Gate Motor:
 *    * Signal (Orange/Yellow) -> Pin 9
 *    * VCC (Red) -> 5V
 *    * GND (Brown/Black) -> GND
 *  - LCD I2C Screen:
 *    * SDA -> Pin A4
 *    * SCL -> Pin A5
 *    * VCC -> 5V
 *    * GND -> GND
 */

// ==========================================
// MEMORY OPTIMIZATION FOR IRREMOTE
// ==========================================
// Defining DECODE_NEC before including IRremote.h disables all other 
// protocols (Sony, Panasonic, etc.) saving up to 8KB of Flash and lots of SRAM.
#define DECODE_NEC           

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <IRremote.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

// ==========================================
// PIN DEFINITIONS
// ==========================================
const int TRIG_PIN       = 2;   // Ultrasonic Trigger
const int ECHO_PIN       = 3;   // Ultrasonic Echo
const int IR_RECV_PIN    = 4;   // IR Receiver
const int BUTTON_PIN     = 5;   // Emergency Button (Internal pullup)
const int LED_GREEN      = 6;   // Access Granted LED
const int LED_RED        = 7;   // Access Denied LED
const int SERVO_PIN      = 9;   // Servo Gate Motor

// Fingerprint Serial Pins (Software Serial)
const int FINGER_RX_PIN  = 8;   // Connect to TX of AS608 sensor (Arduino RX)
const int FINGER_TX_PIN  = 11;  // Connect to RX of AS608 sensor (Arduino TX)

// ==========================================
// CONSTANTS & OBJECTS
// ==========================================
const long DISTANCE_THRESHOLD = 50;     // Detection distance in cm
const unsigned long TIMEOUT_SCAN   = 10000; // 10 seconds scan timeout
const unsigned long TIMEOUT_OPEN   = 5000;  // 5 seconds gate open duration
const unsigned long TIMEOUT_DENIED = 3000;  // 3 seconds denied lock duration
const unsigned long TIMEOUT_ADMIN  = 10000; // 10 seconds admin open duration

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo gateServo;
SoftwareSerial fingerSerial(FINGER_RX_PIN, FINGER_TX_PIN);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

// Auto-detect if using new IRremote library (v3.x+) or legacy (v2.x)
#if defined(VERSION_MAJOR) || defined(VERSION_IRREMOTE) || defined(IrReceiver)
  #define NEW_IRREMOTE
#endif

#ifndef NEW_IRREMOTE
  IRrecv irrecv(IR_RECV_PIN);
  decode_results irResults;
#endif

// ==========================================
// STATE MACHINE DEFINITIONS
// ==========================================
enum SystemState {
  STATE_IDLE,
  STATE_WAITING_FOR_SCAN,
  STATE_ACCESS_GRANTED,
  STATE_ACCESS_DENIED,
  STATE_ADMIN_OVERRIDE,
  STATE_EMERGENCY
};

SystemState currentState = STATE_IDLE;
unsigned long stateStartTime = 0;
bool isGateOpen = false; // Tracks if the 360-degree servo gate is currently open

// Non-blocking timer variables
unsigned long lastDistanceCheck = 0;
const unsigned long DISTANCE_CHECK_INTERVAL = 250; // Check distance 4 times a second
long currentDistance = 999;

// Button state variables (for debouncing)
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long DEBOUNCE_DELAY = 50;

// ==========================================
// FUNCTION PROTOTYPES
// ==========================================
long readUltrasonicDistance();
bool checkIROverride();
bool checkButtonPress();
int getFingerprintStatus();
void updateActuators(int state);

// ==========================================
// ARDUINO SETUP
// ==========================================
void setup() {
  Serial.begin(9600);
  Serial.println(F("[SYSTEM START] Biometric Border Gate Initializing (Physical Hardware)..."));

  // Configure standard I/O pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  // Initialize Servo
  gateServo.attach(SERVO_PIN);
  gateServo.write(90); // Stop 360 servo initially on boot

  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize IR Receiver
#ifdef NEW_IRREMOTE
  IrReceiver.begin(IR_RECV_PIN, ENABLE_LED_FEEDBACK);
#else
  irrecv.enableIRIn();
#endif

  // Initialize AS608 Fingerprint Reader
  finger.begin(57600);
  delay(500);
  if (finger.verifyPassword()) {
    Serial.println(F("[BIOMETRIC] AS608 Fingerprint Sensor Detected successfully!"));
  } else {
    Serial.println(F("[BIOMETRIC ERROR] AS608 Fingerprint Sensor NOT found. Check wiring."));
  }

  // Set initial state
  updateActuators(STATE_IDLE);
  Serial.println(F("=== SYSTEM READY ==="));
}

// ==========================================
// ARDUINO MAIN LOOP
// ==========================================
void loop() {
  unsigned long currentTime = millis();

  // 1. Non-blocking Distance Measurement (Ultrasonic)
  if (currentTime - lastDistanceCheck >= DISTANCE_CHECK_INTERVAL) {
    lastDistanceCheck = currentTime;
    currentDistance = readUltrasonicDistance();
    
    // Print distance to Serial Monitor for debugging (every 1 second)
    static unsigned long lastDistancePrint = 0;
    if (currentTime - lastDistancePrint >= 1000) {
      lastDistancePrint = currentTime;
      Serial.print(F("[DEBUG] Distance: "));
      Serial.print(currentDistance);
      Serial.println(F(" cm"));
    }
  }

  // 2. Check Emergency Button Press
  if (checkButtonPress()) {
    if (currentState == STATE_EMERGENCY) {
      Serial.println(F("[BUTTON] Resetting from emergency mode..."));
      currentState = STATE_IDLE;
      updateActuators(currentState);
    } else {
      Serial.println(F("[BUTTON] Emergency button pressed!"));
      currentState = STATE_EMERGENCY;
      updateActuators(currentState);
    }
  }

  // 3. Check Admin IR Override (Bypass) - disabled during emergency
  if (currentState != STATE_EMERGENCY && checkIROverride()) {
    currentState = STATE_ADMIN_OVERRIDE;
    updateActuators(currentState);
  }

  // 4. State Machine transitions
  switch (currentState) {
    
    case STATE_IDLE:
      // Distance monitoring during testing
      if (currentDistance <= DISTANCE_THRESHOLD) {
        Serial.print(F("[SYSTEM] Traveler detected at: "));
        Serial.print(currentDistance);
        Serial.println(F(" cm."));
        currentState = STATE_WAITING_FOR_SCAN;
        updateActuators(currentState);
      }
      break;

    case STATE_WAITING_FOR_SCAN:
      // Check if traveler walked away (hysteresis of +20cm)
      if (currentDistance > DISTANCE_THRESHOLD + 20) {
        if (currentTime - stateStartTime >= 5000) { // Walked away for >5s
          Serial.println(F("[SYSTEM] Traveler walked away. Resetting to IDLE."));
          currentState = STATE_IDLE;
          updateActuators(currentState);
          break;
        }
      } else {
        // Reset timeout while traveler remains close
        stateStartTime = currentTime;
      }

      // Check for scan timeout
      if (currentTime - stateStartTime >= TIMEOUT_SCAN) {
        Serial.println(F("[SYSTEM] Scanning timeout reached."));
        currentState = STATE_IDLE;
        updateActuators(currentState);
        break;
      }

      // Read physical fingerprint sensor
      int scanStatus;
      scanStatus = getFingerprintStatus();

      if (scanStatus >= 0) {
        // Authorized traveler (ID matched)
        currentState = STATE_ACCESS_GRANTED;
        updateActuators(currentState);
      } 
      else if (scanStatus == -2) {
        // Unauthorized traveler (Stranger)
        currentState = STATE_ACCESS_DENIED;
        updateActuators(currentState);
      }
      break;

    case STATE_ACCESS_GRANTED:
      // Keep gate open for TIMEOUT_OPEN, then close
      if (currentTime - stateStartTime >= TIMEOUT_OPEN) {
        Serial.println(F("[SYSTEM] Welcome timeout completed. Closing gate."));
        currentState = STATE_IDLE;
        updateActuators(currentState);
      }
      break;

    case STATE_ACCESS_DENIED:
      // Show denied screen for TIMEOUT_DENIED, then reset
      if (currentTime - stateStartTime >= TIMEOUT_DENIED) {
        Serial.println(F("[SYSTEM] Denied lock timeout completed. Resetting."));
        currentState = STATE_IDLE;
        updateActuators(currentState);
      }
      break;

    case STATE_ADMIN_OVERRIDE:
      // Keep gate open for TIMEOUT_ADMIN, then close
      if (currentTime - stateStartTime >= TIMEOUT_ADMIN) {
        Serial.println(F("[SYSTEM] Admin override timeout completed. Closing gate."));
        currentState = STATE_IDLE;
        updateActuators(currentState);
      }
      break;

    case STATE_EMERGENCY:
      // Flash Red LED at 2Hz (non-blocking)
      if ((currentTime / 250) % 2 == 0) {
        digitalWrite(LED_RED, HIGH);
      } else {
        digitalWrite(LED_RED, LOW);
      }
      break;
  }
}

// ==========================================
// CORE HARDWARE FUNCTIONS
// ==========================================

long readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Measure echo pulse (30ms timeout = ~5 meters maximum distance)
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); 
  if (duration == 0) {
    return 999; 
  }
  return duration / 58; // Convert pulse duration to cm
}

bool checkIROverride() {
#ifdef NEW_IRREMOTE
  if (IrReceiver.decode()) {
    unsigned long irCode = IrReceiver.decodedIRData.decodedRawData;
    if (irCode == 0) {
      irCode = IrReceiver.decodedIRData.command;
    }
    if (irCode != 0 && irCode != 0xFFFFFFFF) {
      Serial.print(F("[IR] Code Received: 0x"));
      Serial.println(irCode, HEX);
      Serial.println(F("[ADMIN] Moses master key activated."));
      IrReceiver.resume(); // Ready to receive next value
      return true;
    }
    IrReceiver.resume(); 
  }
#else
  if (irrecv.decode(&irResults)) {
    unsigned long irCode = irResults.value;
    if (irCode != 0 && irCode != 0xFFFFFFFF) {
      Serial.print(F("[IR] Code Received: 0x"));
      Serial.println(irCode, HEX);
      Serial.println(F("[ADMIN] Moses master key detected."));
      irrecv.resume(); // Ready to receive next value
      return true;
    }
    irrecv.resume(); 
  }
#endif
  return false;
}

bool checkButtonPress() {
  static bool lastState = HIGH;
  bool currentState = digitalRead(BUTTON_PIN);
  bool pressed = false;

  if (currentState == LOW && lastState == HIGH) {
    pressed = true;
  }
  lastState = currentState;
  return pressed;
}

int getFingerprintStatus() {
  uint8_t p = finger.getImage();
  if (p == FINGERPRINT_NOFINGER) return -1;
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print(F("[BIOMETRIC] Match found! ID #"));
    Serial.print(finger.fingerID);
    Serial.print(F(" | Confidence: "));
    Serial.println(finger.confidence);
    Serial.println(F("[ACCESS] Authorized traveler approved."));
    return finger.fingerID;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println(F("[BIOMETRIC] Stranger detected! Access Denied."));
    return -2;
  }
  return -1;
}

/**
 * Controls the 360-degree continuous rotation servo by timing its spin.
 * @param open: true to open the gate, false to close it.
 */
void setGatePosition(bool open) {
  if (open == isGateOpen) return; // Already in target position
  
  if (open) {
    Serial.println(F("[SERVO] Opening gate (360)..."));
    // 105 is slightly above 90 (slow clockwise rotation)
    // You can adjust '105' (speed) and '1000' (time in ms) to match your physical gate limits!
    gateServo.write(105); 
    delay(1000);          
    gateServo.write(90);  // Stop
    isGateOpen = true;
  } else {
    Serial.println(F("[SERVO] Closing gate (360)..."));
    // 75 is slightly below 90 (slow counter-clockwise rotation)
    // You can adjust '75' (speed) and '1000' (time in ms) to match your physical gate limits!
    gateServo.write(75);  
    delay(1000);          
    gateServo.write(90);  // Stop
    isGateOpen = false;
  }
}

void updateActuators(int state) {
  stateStartTime = millis();
  
  Serial.print(F("[STATE CHANGE] "));
  switch(state) {
    case STATE_IDLE:
      Serial.println(F("IDLE - Gate Locked"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("  Gate Locked   "));
      lcd.setCursor(0, 1);
      lcd.print(F("Bnei Yisrael Only"));
      
      setGatePosition(false); // Close gate
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      break;
      
    case STATE_WAITING_FOR_SCAN:
      Serial.println(F("WAITING_FOR_SCAN - Traveler Detected"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Traveler Detected"));
      lcd.setCursor(0, 1);
      lcd.print(F("Scan Fingerprint"));
      
      setGatePosition(false); // Keep gate closed
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, LOW);
      break;
      
    case STATE_ACCESS_GRANTED:
      Serial.println(F("ACCESS_GRANTED - Welcome to Israel!"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Welcome Home!"));
      lcd.setCursor(0, 1);
      lcd.print(F("Bnei Yisrael :)"));
      
      setGatePosition(true); // Open gate
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      break;
      
    case STATE_ACCESS_DENIED:
      Serial.println(F("ACCESS_DENIED - Intruder Blocked!"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Access Denied!"));
      lcd.setCursor(0, 1);
      lcd.print(F("Stranger Alert!"));
      
      setGatePosition(false); // Keep gate locked
      digitalWrite(LED_GREEN, LOW);
      digitalWrite(LED_RED, HIGH);
      break;
      
    case STATE_ADMIN_OVERRIDE:
      Serial.println(F("ADMIN_OVERRIDE - Moses Master Key"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Admin Override!"));
      lcd.setCursor(0, 1);
      lcd.print(F("Moses Key Active"));
      
      setGatePosition(true); // Open gate
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      break;
      
    case STATE_EMERGENCY:
      Serial.println(F("EMERGENCY - Manual Alarm Tripped!"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("EMERGENCY ALARM "));
      lcd.setCursor(0, 1);
      lcd.print(F("Gate Failsafe OP"));
      
      setGatePosition(true); // Open gate for safety
      digitalWrite(LED_GREEN, LOW);
      break;
  }
}
