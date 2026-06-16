#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>
#include "HX711.h"

#include <SPI.h>
#include <MFRC522.h>

#define BUTTON_PIN 2
#define HX711_DT 3
#define TEMP_PIN 4
#define HX711_SCK 5
#define SERVO_PIN 6
#define LED_PIN 8

#define RFID_RST_PIN 9
#define RFID_SS_PIN 10

#define READY_TEMP 28
#define RESET_TEMP 26
#define THICK_LIMIT 1000

LiquidCrystal_I2C lcd(0x27, 16, 2);

OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);

Servo matzahServo;
HX711 scale;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

int lastButtonState = LOW;
bool cycleComplete = false;

long zeroValue = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);

  lcd.init();
  lcd.backlight();

  sensors.begin();

  matzahServo.attach(SERVO_PIN);
  matzahServo.write(0);

  scale.begin(HX711_DT, HX711_SCK);

  // start RFID reader
  SPI.begin();
  rfid.PCD_Init();

  lcd.setCursor(0, 0);
  lcd.print("Matzah System");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");
  delay(2000);

  // baker identification before system continues
  waitForBakerRFID();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weight setup");
  lcd.setCursor(0, 1);
  lcd.print("Do not touch");
  delay(2000);

  if (scale.is_ready()) {
    zeroValue = scale.read_average(10);

    Serial.print("Zero value: ");
    Serial.println(zeroValue);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Zero saved");
    lcd.setCursor(0, 1);
    lcd.print(zeroValue);
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HX711 error");
    lcd.setCursor(0, 1);
    lcd.print("Check wires");
    delay(2000);
  }

  lcd.clear();
}

void loop() {
  sensors.requestTemperatures();
  float temperature = sensors.getTempCByIndex(0);

  int buttonState = digitalRead(BUTTON_PIN);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  if (cycleComplete == true) {
    showCycleCompleteScreen(temperature);

    if (temperature < RESET_TEMP) {
      cycleComplete = false;
      lcd.clear();
    }

    lastButtonState = buttonState;
    delay(300);
    return;
  }

  if (temperature >= READY_TEMP) {
    showReadyScreen(temperature);

    if (buttonState == HIGH && lastButtonState == LOW) {
      startBakingProcess();
      checkMatzahWeight();
      cycleComplete = true;
    }
  } else {
    showHeatingScreen(temperature);
  }

  lastButtonState = buttonState;

  delay(300);
}

void showHeatingScreen(float temperature) {
  digitalWrite(LED_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C   ");

  lcd.setCursor(0, 1);
  lcd.print("Heating Oven   ");
}

void showReadyScreen(float temperature) {
  digitalWrite(LED_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C   ");

  lcd.setCursor(0, 1);
  lcd.print("Ready to Bake! ");
}

void showCycleCompleteScreen(float temperature) {
  digitalWrite(LED_PIN, LOW);

  lcd.setCursor(0, 0);
  lcd.print("Cycle Complete ");

  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(temperature);
  lcd.print(" C ");
}

void startBakingProcess() {
  Serial.println("Baking process started");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Matzah entered ");
  lcd.setCursor(0, 1);
  lcd.print("Servo moving   ");

  matzahServo.write(90);
  delay(1000);

  digitalWrite(LED_PIN, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Oven working   ");
  lcd.setCursor(0, 1);
  lcd.print("Baking...      ");

  delay(3000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Matzah out     ");
  lcd.setCursor(0, 1);
  lcd.print("Servo moving   ");

  matzahServo.write(0);
  delay(1000);

  digitalWrite(LED_PIN, LOW);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Baking done!   ");
  lcd.setCursor(0, 1);
  lcd.print("Check weight   ");

  delay(2000);

  lcd.clear();
}

void checkMatzahWeight() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weighing...");
  lcd.setCursor(0, 1);
  lcd.print("Please wait");
  delay(1500);

  if (scale.is_ready()) {
    long rawValue = scale.read_average(10);
    long weightChange = rawValue - zeroValue;

    if (weightChange < 0) {
      weightChange = weightChange * -1;
    }

    Serial.print("Raw: ");
    Serial.print(rawValue);
    Serial.print(" | Change: ");
    Serial.println(weightChange);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight: ");
    lcd.print(weightChange);

    lcd.setCursor(0, 1);

    if (weightChange < THICK_LIMIT) {
      lcd.print("Too Thick!     ");
    } else {
      lcd.print("Perfect Matzah!");
    }

    delay(3000);
  } else {
    Serial.println("HX711 not ready");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HX711 error");
    lcd.setCursor(0, 1);
    lcd.print("Check wires");
    delay(3000);
  }

  lcd.clear();
}

// waits until the baker scans an RFID card
void waitForBakerRFID() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scan Baker RFID");
  lcd.setCursor(0, 1);
  lcd.print("Waiting...     ");

  while (true) {
    if (!rfid.PICC_IsNewCardPresent()) {
      delay(100);
      continue;
    }

    if (!rfid.PICC_ReadCardSerial()) {
      delay(100);
      continue;
    }

    Serial.print("Baker RFID UID: ");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Baker Approved");
    lcd.setCursor(0, 1);

    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i], HEX);
      Serial.print(" ");

      if (rfid.uid.uidByte[i] < 0x10) {
        lcd.print("0");
      }

      lcd.print(rfid.uid.uidByte[i], HEX);
    }

    Serial.println();

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    delay(2500);
    lcd.clear();
    break;
  }
}


