#include <IRremote.hpp>
#include <LiquidCrystal.h>
#include <Servo.h>

// LCD pins: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(3, A0, A1, A2, A3, A4);

// Servo object used to raise the camp warning flag
Servo alarmServo;

// Hardware connections
const int buttonPin = 2;
const int trigPin = 4;
const int echoPin = 5;
const int irPin = 6;
const int buzzerPin = 7;
const int ledPin = 8;
const int servoPin = 9;

// System status variables
bool systemArmed = false;
bool alarmActive = false;

int lastButtonState = LOW;
const int distanceThreshold = 30;

unsigned long lastDistanceCheck = 0;
unsigned long alarmResetTime = 0;

String lastLine1 = "";
String lastLine2 = "";

void setup()
{
  pinMode(buttonPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(ledPin, LOW);
  noTone(buzzerPin);

  alarmServo.attach(servoPin);
  alarmServo.write(0);

  lcd.begin(16, 2);
  showScreen("Camp Safe", "Press Button");

  Serial.begin(115200);
  IrReceiver.begin(irPin, ENABLE_LED_FEEDBACK);

  Serial.println("SYSTEM STARTED");
  Serial.println("Mode: Camp Safe");
  Serial.println("Waiting for guard button...");
}

void loop()
{
  readIRRemote();
  readButton();

  if (millis() - lastDistanceCheck >= 300)
  {
    lastDistanceCheck = millis();

    if (systemArmed)
    {
      if (millis() - alarmResetTime > 3000)
      {
        int distance = readDistance();

        if (!alarmActive)
        {
          showScreen("Night Guard", "Dist: " + String(distance) + " cm");
        }

        if (distance > 0 && distance < distanceThreshold && !alarmActive)
        {
          alarmActive = true;

          Serial.println("STATUS CHANGE: Intruder detected");
          Serial.print("Measured distance: ");
          Serial.print(distance);
          Serial.println(" cm");
          Serial.println("Alarm activated: LED ON, Buzzer ON, Servo flag UP");
        }
      }
    }
    else
    {
      showScreen("Camp Safe", "Press Button");
    }
  }

  updateAlarm();
}

// Guard button arms or disarms the night security mode
void readButton()
{
  int buttonState = digitalRead(buttonPin);

  if (buttonState == HIGH && lastButtonState == LOW)
  {
    systemArmed = !systemArmed;

    if (!systemArmed)
    {
      alarmActive = false;
      alarmServo.write(0);
      showScreen("Camp Safe", "Press Button");

      Serial.println("STATUS CHANGE: System disarmed");
      Serial.println("Alarm reset and camp returned to safe mode");
    }
    else
    {
      showScreen("Night Guard", "Active");

      Serial.println("STATUS CHANGE: Night guard mode activated");
      Serial.println("The camp perimeter is now being monitored");
    }

    delay(50); // Button debounce delay
  }

  lastButtonState = buttonState;
}

// Authorized guard resets the alarm using the IR remote control
void readIRRemote()
{
  if (IrReceiver.decode())
  {
    int command = IrReceiver.decodedIRData.command;

    Serial.print("IR signal received. Command: ");
    Serial.println(command);

    if (alarmActive)
    {
      alarmActive = false;
      alarmResetTime = millis();

      showScreen("Alarm Reset", "Guard OK");

      Serial.println("STATUS CHANGE: Alarm reset by authorized guard");
      Serial.println("System remains armed and will resume monitoring in 3 seconds");
    }

    IrReceiver.resume();
  }
}

// Activate or deactivate warning devices
void updateAlarm()
{
  if (alarmActive)
  {
    digitalWrite(ledPin, HIGH);
    tone(buzzerPin, 1000);
    alarmServo.write(90);

    showScreen("WARNING!", "Intruder");
  }
  else
  {
    digitalWrite(ledPin, LOW);
    noTone(buzzerPin);
    alarmServo.write(0);
  }
}

// Measure distance from the ultrasonic sensor
int readDistance()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000);

  if (duration == 0)
  {
    return 0;
  }

  int distance = duration * 0.0343 / 2;

  return distance;
}

// Update LCD only when text changes
void showScreen(String line1, String line2)
{
  if (line1 != lastLine1 || line2 != lastLine2)
  {
    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(line1);

    lcd.setCursor(0, 1);
    lcd.print(line2);

    lastLine1 = line1;
    lastLine2 = line2;
  }
}