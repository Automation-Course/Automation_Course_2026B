#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include <Servo.h>
#include <SPI.h>         // Infrastructure library for the SPI protocol (for the physical RFID)
#include <MFRC522.h>     // Library for the physical RC522 RFID module
#include "HX711.h"       // Library for the physical HX711 weight module


// Pin definitions (pins + hardware additions)

#define BUTTON_PIN      2   // Moses button: open/close sea (press = HIGH)
#define LED_PIN         3   // Enough food LED
#define TRIG_PIN        4   // Ultrasonic TRIG
#define ECHO_PIN        5   // Ultrasonic ECHO
#define SERVO_PIN       9   // Servo fan / sea simulation

// Pins for the physical RFID reader (RC522)
#define RFID_SS_PIN     10  // Component select pin (SDA/SS on the RFID)
#define RFID_RST_PIN    -1  // The RFID RST pin is permanently connected to 3.3V on the breadboard 

// Pins for the physical weight transducer (HX711)
#define HX711_DOUT_PIN  A2  // Data Out pin of the weight sensor
#define HX711_SCK_PIN   A3  // Serial Clock pin of the weight sensor


// Components & Constants

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Servo seaServo;

MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN); // Real RFID reader object
HX711 scale;                                 // Real weight sensor object

const String AUTHORIZED_UID = "F1 95 99 8A";   // UID code of your authorized cooks' card
const float SCALE_CALIBRATION_FACTOR = 453.16; // Calibration factor of your scale 


// Global variables

bool lastButtonState = LOW;
bool lastRFIDState = LOW;

bool calculationDone = false;
bool seaOpened = false;
bool personDetected = false;
bool systemFinished = false;

int peopleCount = 0;
float requiredFoodWeight = 0;

int servoFanAngle = 0;
bool fanDirection = true;

unsigned long lastLCDUpdateTime = 0;

// Helper functions

void updateLCDCount() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("People Count:");
  lcd.setCursor(0, 1);
  lcd.print(peopleCount);
}

void openSea() {
  seaServo.attach(SERVO_PIN);
  seaServo.write(0);
  Serial.println("Event: Button pressed. Opening Sea.");
  seaOpened = true;

  lcd.clear();
  lcd.print("Sea is Open!");
  delay(1000);

  updateLCDCount();
}

void closeSea() {
  Serial.println("Event: Crossing ended. Closing Sea.");
  seaOpened = false;
  seaServo.write(0);
  seaServo.detach();
  requiredFoodWeight = peopleCount * 100.0;
  calculationDone = true;

  lcd.clear();
  lcd.print("Need food:");
  lcd.setCursor(0, 1);
  lcd.print((int)requiredFoodWeight);
  lcd.print(" g");

  Serial.print("Final count: ");
  Serial.print(peopleCount);
  Serial.print(". Required food: ");
  Serial.print(requiredFoodWeight);
  Serial.println(" grams.");
}

void moveFan() {
  if (fanDirection) {
    servoFanAngle += 5;
  } else {
    servoFanAngle -= 5;
  }

  seaServo.write(servoFanAngle);

  if (servoFanAngle >= 180) { fanDirection = false; }
  if (servoFanAngle <= 0) { fanDirection = true; }

  delay(15);
}

long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) { return 999; }

  long distance = duration * 0.034 / 2;
  return distance;
}

float readSimulatedWeight() {
  float weight = 0.0;
  
  // Check whether the weight transducer is ready to work
  if (scale.is_ready()) {
    // Quick sample (1) from the sensor, using abs (absolute value) to prevent negative readings caused by vibrations
    weight = abs(scale.get_units(1)); 
  }

  // Informative print to the serial monitor (the repeated print was removed to avoid flooding the screen)
  return weight;
}

void checkRFIDSimulation() {
  // Check: is a new physical RFID card placed near the reader, and were we able to read its data?
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    
    // Built-in loop that converts the UID byte array into a readable text String
    String scannedUID = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      scannedUID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      scannedUID.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    scannedUID.toUpperCase();
    scannedUID.trim(); // Remove unnecessary spaces

    Serial.print("Scanned RFID UID: ");
    Serial.println(scannedUID);

    // Condition: is the scanned card the authorized cooks' card?
    if (scannedUID == AUTHORIZED_UID) {
      Serial.println("Auth: RFID Verified! Cook confirmed food is ready.");

      lcd.clear();
      lcd.print("Food is Ready!");
      lcd.setCursor(0, 1);
      lcd.print("Enjoy the meal!");

      systemFinished = true; // Activate the system lock flag (end of one iteration)
    } 
    else {
      // If an unknown card is scanned, the system does not lock and displays a temporary warning message
      Serial.println("Unknown Card. Access Denied.");
      lcd.setCursor(0, 1);
      lcd.clear();
      lcd.print("Wrong RFID!     ");
      delay(1500); 
    }
    
    mfrc522.PICC_HaltA(); // Command telling the current card to stop transmitting (prevents duplicate readings in the loop)
  }
}

// Arduino core functions

void setup() {
  Serial.begin(9600);
  
  SPI.begin();           // Initialize the SPI communication line (required for the RFID module)
  mfrc522.PCD_Init();    // Physical initialization of the RFID card reader

  // Initial setup and calibration of the HX711 weight module
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(SCALE_CALIBRATION_FACTOR);
  scale.tare();          // Reset the scale to 0 grams (automatically subtracts the weight of the empty manna container at startup)

  pinMode(BUTTON_PIN, INPUT); // Remains regular INPUT exactly like in your original code
  pinMode(LED_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Servo initialization was moved to the openSea/closeSea functions as you arranged, protected against noise!

  // Proper physical screen initialization
  lcd.init();          
  lcd.backlight();     
  
  lcd.clear();
  lcd.print("Welcome to Sinai");
  delay(2000);

  lcd.clear();
  lcd.print("Press to cross");

  Serial.println("System started. Waiting for Moses button.");
}

void loop() {
  if (systemFinished) {
    return;
  }

  // Button: open / close sea

  int currentButtonState = digitalRead(BUTTON_PIN);

  if (currentButtonState == HIGH && lastButtonState == LOW) {
    if (!seaOpened && !calculationDone) {
      openSea();
    } 
    else if (seaOpened) {
      closeSea();
    }
    delay(200); 
  }
  lastButtonState = currentButtonState;

  
  // Sea open: fan movement + counting people

  if (seaOpened) {
    moveFan();

    long distance = getDistance();

    if (distance < 15 && !personDetected) {
      peopleCount++;
      personDetected = true;

      Serial.print("Event: Person crossed. Total: ");
      Serial.println(peopleCount);

      updateLCDCount();
    } 
    else if (distance > 20) {
      personDetected = false;
    }
  }

  // Food checking stage
  
  if (calculationDone) {
    if (millis() - lastLCDUpdateTime > 300) {
      lastLCDUpdateTime = millis();
      
      float currentWeight = readSimulatedWeight();

      lcd.setCursor(0, 0);
      lcd.print("Need:");
      lcd.print((int)requiredFoodWeight);
      lcd.print("g    "); 

      lcd.setCursor(0, 1);
      if (currentWeight < requiredFoodWeight) {
        digitalWrite(LED_PIN, LOW);
        lcd.print("Now:");
        lcd.print((int)currentWeight);
        lcd.print("g Add ");
      } 
      else {
        digitalWrite(LED_PIN, HIGH);
        lcd.print("Manna Enough!  ");
        checkRFIDSimulation();
      }
    }
  }
}