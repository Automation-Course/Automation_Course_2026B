#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
#include <Servo.h>
#include <SPI.h>        
#include <MFRC522.h>    

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define redLed 8
#define buttonPin 2
#define trigPin 6
#define echoPin 7
#define servoPin 9
#define greenLed 3     

#define RST_PIN   A1    
#define SS_PIN    10    

SoftwareSerial fingerSerial(4, 5); 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fingerSerial);

MFRC522 mfrc522(SS_PIN, RST_PIN); 
Servo gateServo;

bool processStarted = false;
bool seaClosed = false; 

void setup()
{
  Serial.begin(9600);

  pinMode(redLed, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(greenLed, OUTPUT);
  digitalWrite(greenLed, LOW);

  gateServo.attach(servoPin);
  gateServo.write(90); 

  lcd.init();
  lcd.backlight();

  SPI.begin();
  mfrc522.PCD_Init(); 

  finger.begin(57600);

  Serial.println(F("System started"));

  lcd.setCursor(0, 0);
  lcd.print("Yetziat");
  lcd.setCursor(0, 1);
  lcd.print("Mitzrayim");
  delay(3000);

  if(finger.verifyPassword())
  {
    Serial.println(F("Fingerprint sensor found"));
  }
  else
  {
    Serial.println(F("Fingerprint sensor NOT found"));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Finger Error");
    lcd.setCursor(0, 1);
    lcd.print("Check Sensor");
    delay(4000);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press Button");
  lcd.setCursor(0, 1);
  lcd.print("for 10 Makot");

  Serial.println(F("Waiting for button press"));
}

void loop()
{
  if(digitalRead(buttonPin) == HIGH && processStarted == false && seaClosed == false)
  {
    delay(50); 

    if(digitalRead(buttonPin) == HIGH)
    {
      processStarted = true;
      Serial.println(F("[Mstye State] Button pressed - Starting 10 Makot"));

      tenMakot();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Makot Finished");
      lcd.setCursor(0, 1);
      lcd.print("Come Closer");

      Serial.println(F("10 Makot finished. Waiting for Bnei Israel."));
      delay(3000);

      checkDistanceLoop();
    }
  }
}

void tenMakot()
{
  for(int i = 1; i <= 10; i++)
  {
    lcd.clear();
    digitalWrite(redLed, HIGH);

    lcd.setCursor(0, 0);
    lcd.print("10 Makot:");
    lcd.setCursor(0, 1);

    Serial.print(F("Maka number "));
    Serial.print(i);
    Serial.print(F(": "));

    if(i == 1) { lcd.print("1 Dam"); Serial.println(F("Dam")); }
    if(i == 2) { lcd.print("2 Tzfardea"); Serial.println(F("Tzfardea")); }
    if(i == 3) { lcd.print("3 Kinim"); Serial.println(F("Kinim")); }
    if(i == 4) { lcd.print("4 Arov"); Serial.println(F("Arov")); }
    if(i == 5) { lcd.print("5 Dever"); Serial.println(F("Dever")); }
    if(i == 6) { lcd.print("6 Shchin"); Serial.println(F("Shchin")); }
    if(i == 7) { lcd.print("7 Barad"); Serial.println(F("Barad")); }
    if(i == 8) { lcd.print("8 Arbeh"); Serial.println(F("Arbeh")); }
    if(i == 9) { lcd.print("9 Hoshech"); Serial.println(F("Hoshech")); }
    if(i == 10) { lcd.print("10 Bechorot"); Serial.println(F("Bechorot")); }

    delay(500);
    digitalWrite(redLed, LOW);
    delay(500);
  }
}

void checkDistanceLoop()
{
  while(true)
  {
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) 
    {
      Serial.println(F("[State Change] RFID Card Detected! Closing the Sea."));
      seaClosed = true; 
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Sea is Closed!");
      lcd.setCursor(0, 1);
      lcd.print("Yetzat Mitzrayim");
      
      Serial.println(F("========================================="));
      Serial.println(F("SUCCESS: YETZIAT MITZRAYIM COMPLETED!"));
      Serial.println(F("The sea is locked. No further access allowed."));
      Serial.println(F("========================================="));
      
      mfrc522.PICC_HaltA(); 
      break; 
    }

    int distance = readDistance();
    Serial.println(F("-------------------"));

    if(distance == -1)
    {
      Serial.println(F("ERROR: No Echo Received"));
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("No Echo");
      lcd.setCursor(0, 1);
      lcd.print("Check Sensor");
    }
    else
    {
      Serial.print(F("Distance: "));
      Serial.print(distance);
      Serial.println(F(" cm"));

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Distance:");
      lcd.setCursor(0, 1);
      lcd.print(distance);
      lcd.print(" cm");

      if(distance > 0 && distance <= 20)
      {
        Serial.println(F("Bnei Israel detected within 20 cm. Requesting Fingerprint."));

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Bnei Israel");
        lcd.setCursor(0, 1);
        lcd.print("Place Finger");
        delay(3000);

        bool approved = waitForFingerprint(10);

        if(approved)
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Approved");
          lcd.setCursor(0, 1);
          lcd.print("Sea Opens");
          Serial.println(F("Fingerprint approved - opening sea gate"));
          delay(3000);

          openGate360();

          lcd.clear();
        
        }
        else
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Access Denied");
          lcd.setCursor(0, 1);
          lcd.print("Try Again");
          Serial.println(F("Fingerprint denied - gate stays closed"));
          delay(4000);
        }

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Move Away");
        lcd.setCursor(0, 1);
        lcd.print("Then Return");
        Serial.println(F("Waiting for person to move away"));
        delay(3000);

        while(true)
        {
          if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
             break; 
          }
          
          int currentDistance = readDistance();
          if(currentDistance == -1 || currentDistance > 25)
          {
            Serial.println(F("Person moved away"));
            break;
          }
          delay(500);
        }
      }
      else
      {
        Serial.println(F("Waiting for person to come closer... (Or RFID closure)"));
      }
    }
    delay(1000);
  }
}

int readDistance()
{
  long duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH, 30000);
  if(duration == 0)
  {
    return -1;
  }
  return (duration * 0.034 / 2);
}

bool waitForFingerprint(int secondsToWait)
{
  unsigned long startTime = millis();
  while(millis() - startTime < secondsToWait * 1000UL)
  {
    int fingerID = getFingerprintID();
    if(fingerID >= 0)
    {
      Serial.print(F("Authorized ID: "));
      Serial.println(fingerID);
      return true;
    }
    Serial.println(F("Waiting for valid fingerprint..."));
    delay(500);
  }
  Serial.println(F("Fingerprint timeout"));
  return false;
}

int getFingerprintID()
{
  uint8_t p = finger.getImage();
  if(p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if(p != FINGERPRINT_OK) return -1;

  p = finger.fingerSearch();
  if(p != FINGERPRINT_OK) return -1;

  return finger.fingerID;
}

void openGate360()
{
  Serial.println(F("Opening gate"));
  gateServo.write(0);
  delay(680);
  gateServo.write(90);

  digitalWrite(greenLed, HIGH); 
  Serial.println(F("Gate opened - LED Green ON"));

  delay(5000); 

  Serial.println(F("Closing gate"));
  gateServo.write(180);
  delay(3000);
  gateServo.write(90);

  digitalWrite(greenLed, LOW);
  Serial.println(F("Gate closed - LED Green OFF"));
}