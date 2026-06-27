#include <LiquidCrystal.h>
#include <Servo.h>
#include <IRremote.h>
#include "HX711.h"

// =====================
// Pin Definitions
// =====================
#define buttonPin 2
#define ledPin 3
#define servoPin 9
#define irPin 11

#define hx711DT 8
#define hx711SCK A1

// =====================
// LCD
// =====================
LiquidCrystal lcd(12, 10, 7, 6, 5, 4);

// =====================
// Servo + Scale
// =====================
Servo gateServo;
HX711 scale;

// =====================
// Weight Thresholds
// =====================
#define OK_LIMIT 600000
#define OVERLOAD_LIMIT 2000000

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);

  lcd.begin(16, 2);

  gateServo.attach(servoPin);

  // Continuous Servo STOP
  gateServo.write(90);

  scale.begin(hx711DT, hx711SCK);

  IrReceiver.begin(irPin);

  showHomeScreen();

  Serial.println("System Ready");
}

void loop() {

  if (digitalRead(buttonPin) == LOW) {

    delay(250);

    bool irOK = showIRScreen();

    if (irOK) {
      checkWeight();
    }

    showHomeScreen();

    delay(500);
  }
}

// =====================
// Home Screen
// =====================
void showHomeScreen() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Red Sea Gate");

  lcd.setCursor(0, 1);
  lcd.print("Press Start");

  digitalWrite(ledPin, LOW);

  gateServo.write(90);
}

// =====================
// IR Check
// =====================
bool showIRScreen() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Scan ID");

  lcd.setCursor(0, 1);
  lcd.print("Use IR Remote");

  Serial.println("Waiting for IR");

  delay(3000);

  if (IrReceiver.decode()) {

    Serial.print("IR Code: ");
    Serial.println(IrReceiver.decodedIRData.command);

    IrReceiver.resume();

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("ID Approved");

    lcd.setCursor(0, 1);
    lcd.print("Access Granted");

    Serial.println("IR Approved");

    delay(1000);

    return true;
  }

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("IR Timeout");

  lcd.setCursor(0, 1);
  lcd.print("Continue...");

  Serial.println("IR Timeout");

  delay(2000);

  return true;
}

// =====================
// Weight Check
// =====================
void checkWeight() {

  long weightValue = scale.read_average(10);

  Serial.print("Weight Value: ");
  Serial.println(weightValue);

  lcd.clear();

  // Overloaded
  if (weightValue >= OVERLOAD_LIMIT) {

    lcd.setCursor(0, 0);
    lcd.print("Overloaded");

    lcd.setCursor(0, 1);
    lcd.print("Remove Cargo");

    digitalWrite(ledPin, LOW);

    gateServo.write(90);

    Serial.println("REJECTED - OVERLOAD");

    delay(3000);
  }

  // Approved
  else if (weightValue >= OK_LIMIT) {

    lcd.setCursor(0, 0);
    lcd.print("Weight OK");

    lcd.setCursor(0, 1);
    lcd.print("Cross The Sea");

    digitalWrite(ledPin, HIGH);

    Serial.println("OPENING GATE");

    // Open Gate
    gateServo.write(120);
    delay(1000);

    gateServo.write(90);

    Serial.println("Gate Open");

    delay(5000);

    Serial.println("CLOSING GATE");

    // Close Gate
    gateServo.write(60);
    delay(1000);

    gateServo.write(90);

    digitalWrite(ledPin, LOW);

    Serial.println("Gate Closed");

    delay(1000);
  }

  // Too Low
  else {

    lcd.setCursor(0, 0);
    lcd.print("Load Too Low");

    lcd.setCursor(0, 1);
    lcd.print("Add Cargo");

    digitalWrite(ledPin, LOW);

    gateServo.write(90);

    Serial.println("REJECTED - LOW LOAD");

    delay(3000);
  }
}