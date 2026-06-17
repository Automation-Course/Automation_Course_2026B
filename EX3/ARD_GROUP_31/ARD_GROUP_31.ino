#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.hpp>

/**
 * --- Finite State Machine States ---
 * These define the logical steps the system goes through.
 * Waiting for IR -> Waiting for Voice Input -> Checking Safety -> Water Flowing
 */
enum SystemState {
  STATE_WAIT_FOR_IR,    // System is locked, waiting for IR remote activation
  STATE_WAIT_INPUT,      // Unlocked, waiting for voice/sound input
  STATE_CHECK_SAFETY,    // Voice received, checking if jug is in place
  STATE_WATER_FLOWING    // All conditions met, water is actively flowing
};

// Function prototype for the state transition handler
void transitionTo(SystemState newState);

// Global variable to hold the current active state, starts at locked
SystemState currentState = STATE_WAIT_FOR_IR;

/**
 * --- Pin mapping ---
 * Defines which hardware component is connected to which Arduino pin.
 */
const int PIN_IR_RECEIVER   = 9; // IR receiver signal pin
const int PIN_MIC_BUTTON    = 2; // Sound/Voice sensor input (digital)
const int PIN_HIT_BUTTON    = 3; // "Hit the rock" button input
const int PIN_TRIG          = 6; // Ultrasonic Trig pin
const int PIN_ECHO          = 7; // Ultrasonic Echo pin
const int PIN_SERVO         = 5; // Continuous servo control pin
const int PIN_LED           = 4; // Valve status LED

/**
 * --- System and Timing Constants ---
 */
const int  SERVO_STOP         = 90;   // Stop point signal for continuous servo
const int  SERVO_MAX_SPEED    = 180;  // Signal for max rotation speed!
const long JUG_DISTANCE_CM    = 5;    // Max distance in cm to detect jug
const int  WATER_FLOW_TIME_MS = 5000; // Duration for water flow
const int  ERROR_DISPLAY_MS   = 2000; // Time to show error messages

// Initialize LCD (Address 0x27, 16 chars per line, 2 lines)
LiquidCrystal_I2C lcd(0x27, 16, 2);
// Initialize Servo object for valve control
Servo waterValve;

/**
 * --- Input State Variables ---
 * Used to save previous states of inputs to detect changes (falling edges).
 */
bool lastMicState  = HIGH;
bool lastHitState  = HIGH;

/**
 * --- measureDistanceCM() ---
 * Triggers the ultrasonic sensor to send a pulse and measures the echo time
 * to calculate the distance to an object in centimeters.
 * @return Distance in cm.
 */
long measureDistanceCM() {
  // Clear Trig pin
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  // Set Trig high for 10 microseconds to send pulse
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  // Measure how long Echo pin stays high (in microseconds)
  long duration = pulseIn(PIN_ECHO, HIGH, 30000); // 30ms timeout
  // Calculate distance: Time * Speed of Sound (0.034 cm/us) / 2 (round-trip)
  long distance = duration * 0.034 / 2;
  return distance;
}

/**
 * --- displayMessage(line1, line2) ---
 * Helper function to clear the LCD and print new messages on both lines.
 * @param line1 String for top line.
 * @param line2 String for bottom line.
 * (Hebrew strings are shown in code but displayed on physical LCD)
 */
void displayMessage(String line1, String line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

/**
 * --- openValve() ---
 * Activates the valve system by starting the continuous servo,
 * lighting the LED, and notifying via Serial.
 */
void openValve() {
  waterValve.attach(PIN_SERVO);
  waterValve.write(SERVO_MAX_SPEED);  // Start high-speed continuous rotation
  
  digitalWrite(PIN_LED, HIGH);        // Turn on status LED
  Serial.println("[VALVE] Valve OPENED - water flowing continuously");
}

/**
 * --- closeValve() ---
 * Deactivates the valve system by stopping the continuous servo,
 * turning off the LED, and detaching the servo to save power/reduce noise.
 */
void closeValve() {
  waterValve.write(SERVO_STOP);       // Send stop signal to servo
  delay(50);                          // Wait for servo to react
  waterValve.detach();                // Disconnect servo pin
  
  digitalWrite(PIN_LED, LOW);         // Turn off status LED
  Serial.println("[VALVE] Valve CLOSED");
}

/**
 * --- transitionTo(newState) ---
 * Handles state transitions by updating the global currentState,
 * printing debug info to Serial, and updating the LCD message.
 * @param newState The state to switch the system to.
 */
void transitionTo(SystemState newState) {
  currentState = newState;
  switch (newState) {
    case STATE_WAIT_FOR_IR:
      displayMessage("Welcome Moses!", "Activate System");
      Serial.println("[STATE] --> WAIT_FOR_IR (Waiting for IR Remote)");
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

/**
 * --- setup() ---
 * Runs once at startup. Initializes pins, communication, and hardware.
 */
void setup() {
  Serial.begin(9600); // Start Serial Monitor for debugging
  Serial.println("=== Smart Rock System Initializing ===");

  // Initialize IR receiver on Pin 9
  IrReceiver.begin(PIN_IR_RECEIVER, ENABLE_LED_FEEDBACK);

  // Set up inputs (using internal pullup resistors)
  pinMode(PIN_MIC_BUTTON,  INPUT_PULLUP);
  pinMode(PIN_HIT_BUTTON,  INPUT_PULLUP);
  
  // Set up ultrasonic pins
  pinMode(PIN_TRIG,        OUTPUT);
  pinMode(PIN_ECHO,        INPUT);
  
  // Set up output indicators
  pinMode(PIN_LED,         OUTPUT);
  digitalWrite(PIN_LED,    LOW); // Ensure LED starts off

  // Initialize and ensure valve starts closed
  waterValve.attach(PIN_SERVO);
  waterValve.write(SERVO_STOP);
  delay(100);
  waterValve.detach();

  // Initialize LCD and turn on backlight
  lcd.init();
  lcd.backlight();

  // Begin system in the initial locked state
  transitionTo(STATE_WAIT_FOR_IR);
  Serial.println("=== System Ready ===");
}

/**
 * --- loop() ---
 * Main continuous execution loop. Runs the state machine and checks inputs.
 */
void loop() {
  // Read current digital states of microphone and hit button
  bool currentMicState  = digitalRead(PIN_MIC_BUTTON);
  bool currentHitState  = digitalRead(PIN_HIT_BUTTON);

  /**
   * --- Global Check: Invalid Action (Hitting the rock) ---
   * This check is always active UNLESS the system is locked (STATE_WAIT_FOR_IR).
   * It detects a new press (Falling Edge: High to Low) of the hit button.
   */
  if (currentHitState == LOW && lastHitState == HIGH && currentState != STATE_WAIT_FOR_IR) {
    delay(50); // Small debounce delay
    if (digitalRead(PIN_HIT_BUTTON) == LOW) { // Verify press
      Serial.println("[ERROR] Hit detected globally! Invalid action");
      
      // Notify user on LCD
      displayMessage("Wrong Action!", "Speak, Dont Hit!");
      delay(ERROR_DISPLAY_MS); // Show error for specified time
      
      // Refresh LCD with the message of the CURRENT state after error
      transitionTo(currentState); 
    }
  } 
  else {
    /**
     * --- State Machine Logic ---
     * Perform actions only relevant to the current system state.
     */
    switch (currentState) {

      case STATE_WAIT_FOR_IR: {
        /**
         * System is locked. Waiting for a valid IR remote command to activate.
         */
        if (IrReceiver.decode()) {
          Serial.print("[IR] Button pressed! Command: ");
          Serial.println(IrReceiver.decodedIRData.command); // Debug info
          
          // Activation confirmed, transition to input state
          transitionTo(STATE_WAIT_INPUT);
          
          delay(500); // Delay before next state and to prevent double triggers
          IrReceiver.resume(); // Ready to receive next IR command
        }
        break;
      }

      case STATE_WAIT_INPUT: {
        /**
         * System unlocked. Waiting for valid voice/sound input.
         * Detects a Falling Edge (new sound triggered) on the microphone module.
         */
        if (currentMicState == LOW && lastMicState == HIGH) {
          delay(50); // Debounce voice sensor
          if (digitalRead(PIN_MIC_BUTTON) == LOW) { // Verify input
            Serial.println("[INPUT] Valid voice input detected");
            // Input confirmed, move to safety check state
            transitionTo(STATE_CHECK_SAFETY);
          }
        }
        break;
      }

      case STATE_CHECK_SAFETY: {
        /**
         * Voice received, but valve won't open unless jug is detected below the "rock".
         * Constantly measure distance with ultrasonic sensor.
         */
        long distance = measureDistanceCM();
        
        // Check if object (jug) is in front AND within JUG_DISTANCE_CM
        bool jugDetected = (distance > 0 && distance <= JUG_DISTANCE_CM);
        
        if (jugDetected) {
          Serial.println("[SAFETY] Jug detected - opening valve");
          // Conditions met, start water flow
          transitionTo(STATE_WATER_FLOWING);
          
          openValve();              // Physically open valve
          delay(WATER_FLOW_TIME_MS); // Wait for miracle duration
          closeValve();             // Physically close valve
          
          Serial.println("[SYSTEM] Cycle complete - resetting");
          
          // Clear any IR commands that might have come while water flowed
          while (IrReceiver.decode()) {
            IrReceiver.resume();
          }
          // Read current input states to prevent immediate re-triggering upon reset
          currentMicState = digitalRead(PIN_MIC_BUTTON);
          currentHitState = digitalRead(PIN_HIT_BUTTON);
          
          // Automatically reset system back to locked state
          transitionTo(STATE_WAIT_FOR_IR);
        } else {
          // If jug is not detected, stay in this state until it is.
          delay(50); // Short delay before next measurement
        }
        break;
      }

      case STATE_WATER_FLOWING: {
        /**
         * Active water flow state. The code handles the flow time
         * within the STATE_CHECK_SAFETY block before transitioning here,
         * so this block remains mostly for logical consistency.
         */
        break;
      }
    }
  }

  /**
   * --- Store Input States for Next Loop ---
   * Crucial for edge detection (e.g., detecting WHEN a button is PRESSED).
   */
  lastMicState  = currentMicState;
  lastHitState  = currentHitState;
}