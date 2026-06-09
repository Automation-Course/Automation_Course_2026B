#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo seaServo;

// UID של משה
byte authorizedCardUID[4] = {0x5A, 0xA8, 0x96, 0x15};

// Pins
const int startButton = A2;
const int soundSensor = A3;
const int servoPin = A1;

const int SERVO_STOP = 90;
const int SERVO_RUN = 120;
const int LED_DELAY = 500;
const int REQUIRED_CLAPS = 2;
const int CLAP_INTERVAL = 500;

// State variables
bool seaOpen = false;
bool startPressed = false;
bool clapEnabled = false;

// Clap detection
int clapCount = 0;
unsigned long lastClapTime = 0;

/*
 * Initializes all hardware components
 * and displays the welcome screen.
 */

void setup()
{
  Serial.begin(9600);

  SPI.begin();
  mfrc522.PCD_Init();

  lcd.init();
  lcd.backlight();

  // LEDs
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(A0, OUTPUT);

  // Start button
  pinMode(startButton, INPUT_PULLUP);

  // Sound sensor
  pinMode(soundSensor, INPUT);

  // Servo
  seaServo.attach(servoPin);
  seaServo.write(SERVO_STOP);  // עצור

  
 
  allLedsOff();

  lcd.setCursor(0, 0);
  lcd.print("Red Sea System");
  lcd.setCursor(0, 1);
  lcd.print("Press Start");

  Serial.println("System Ready");
}

/*
 * Main program loop.
 * Handles start button, RFID scanning
 * and clap detection.
 */

void loop()
{
  // START button
  if (!startPressed && digitalRead(startButton) == LOW)
  {
    startPressed = true;

    Serial.println("START pressed");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Scan RFID");
    lcd.setCursor(0, 1);
    lcd.print("Moshe Only");

    delay(500);
  }

  // RFID check
  if (startPressed && !seaOpen)
  {
    if (mfrc522.PICC_IsNewCardPresent() &&
        mfrc522.PICC_ReadCardSerial())
    {
      if (isAuthorizedCard())
      {

     
        Serial.println("Welcome Moshe");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Welcome Moshe");
        lcd.setCursor(0, 1);
        lcd.print("Opening Sea");


        openSea();

        seaOpen = true;
        // עצירת הסרבו מיד אחרי הפתיחה
        seaServo.write(SERVO_STOP);



        // עכשיו אפשר להקשיב למחיאות
        clapEnabled = true;

        clapCount = 0;
        lastClapTime = millis();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sea Open");
        lcd.setCursor(0, 1);
        lcd.print("Clap x2");
      }
      else
      {
        Serial.println("Access denied");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Denied");
        lcd.setCursor(0, 1);
        lcd.print("Not Moshe");

        delay(2000);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Scan RFID");
        lcd.setCursor(0, 1);
        lcd.print("Moshe Only");
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
  }

  // Clap detection
  if (seaOpen && clapEnabled && digitalRead(soundSensor) == HIGH)  {
    unsigned long currentTime = millis();

    if (currentTime - lastClapTime > CLAP_INTERVAL)
    {
      clapCount++;
      lastClapTime = currentTime;

      Serial.print("Clap Count: ");
      Serial.println(clapCount);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sea Open");
      lcd.setCursor(0, 1);
      lcd.print("Clap ");
      lcd.print(clapCount);
      lcd.print("/2");

      if (clapCount >= REQUIRED_CLAPS)
      {
        Serial.println("Closing sea");

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Closing Sea");
        lcd.setCursor(0, 1);
        lcd.print("Please Wait");

        
        
        closeSea();

        seaOpen = false;
        startPressed = false;
        clapEnabled = false;
        clapCount = 0;
        
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Sea Closed");
        lcd.setCursor(0, 1);
        lcd.print("Press Start");

        Serial.println("Sea closed");

        delay(1000);
      }
    }
  }
}

/*
 * Opens the sea by rotating the servo
 * and lighting LEDs sequentially.
 */
void openSea()
{
  // Start spinning
  seaServo.write(SERVO_RUN);
  
  digitalWrite(8, HIGH);
digitalWrite(A0, HIGH);
delay(LED_DELAY);

digitalWrite(6, HIGH);
digitalWrite(7, HIGH);
delay(LED_DELAY);

digitalWrite(4, HIGH);
digitalWrite(5, HIGH);
delay(LED_DELAY);

digitalWrite(2, HIGH);
digitalWrite(3, HIGH);
delay(LED_DELAY);

  Serial.println("Sea opened");
}

/*
 * Closes the sea by turning off LEDs
 * and stopping the servo.
 */
void closeSea()
{
  digitalWrite(2, LOW);
digitalWrite(3, LOW);
delay(LED_DELAY);
  
digitalWrite(4, LOW);
digitalWrite(5, LOW);
delay(LED_DELAY);

digitalWrite(6, LOW);
digitalWrite(7, LOW);
delay(LED_DELAY);

digitalWrite(8, LOW);
digitalWrite(A0, LOW);
delay(LED_DELAY);
  // Stop spinning
  seaServo.write(SERVO_STOP);

  Serial.println("Staff lowered");
}

 /*
 * Turns off all LEDs.
 */
void allLedsOff()
{
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(7, LOW);
  digitalWrite(8, LOW);
  digitalWrite(A0, LOW);
}

/*
 * Checks whether the scanned RFID card
 * matches the authorized card.
 */
bool isAuthorizedCard()
{
  for (byte i = 0; i < 4; i++)
  {
    if (mfrc522.uid.uidByte[i] != authorizedCardUID[i])
    {
      return false;
    }
  }

  return true;
}