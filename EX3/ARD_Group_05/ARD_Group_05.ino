#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <IRremote.hpp>          // v3 only — avoids timer conflict with Servo
#include <VL53L1X.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pin assignments
const int BtnStartPin = 2;    // Start button
const int GreenLedPin = 3;    // oven is active
const int RedLedPin = 4;    // enemy / fire alert
const int YellowLedPin = 5;    // matzahs ready
const int BuzzerPin = 6;
const int SmokeRelayPin = 13;    // relay controlling MQ2 power
const int LidarXshutPin = 8;
const int ServoPin = 9;
const int IrRecvPin = 10;
const int TempSensorPin = 12;
const int SmokeSensorPin = A0;

// Detection thresholds
const int EnemyThresholdMm = 400;   // outer warning boundary
const int CriticalThresholdMm = 200;   // auto transfer point
const float BakeTempThreshold = 52.0;  // raised via body temp
const float SmokeMultiplier = 1.2;  // trigger when reading is 25% above baseline

// MQ2 warmup after the relay powers it on at baking start
// Relay module is active-LOW: LOW = coil on = sensor powered, HIGH = sensor off
const unsigned long Mq2BakeWarmupMs = 20000;

// IR remote button codes
const uint8_t IrCmdReset    = 0x19;  // 0 reset
const uint8_t IrCmdTransfer = 0x45;  // 1 transfer dough to oven
const uint8_t IrCmdBake     = 0x46;  // 2 turn on oven
const uint8_t IrCmdBurnt    = 0x47;  // 3 dismiss fire alert

const int LcdI2cAddr = 0x27;

// State machine
enum SystemState {
  StateIdle,          // system ready, waiting for start button
  StateDoughRising,   // servo scans border + LIDAR monitors for enemies
  StateEnemyAlert,    // enemy in warning zone, red LED + buzzer active
  StateTransfer,      // dough moving to oven, waiting for user to start baking
  StateBaking,        // oven on, monitoring temp and smoke
  StateMatzahReady,   // target temp reached, process ends
  StateFireAlert      // smoke detected, process ends
};
SystemState systemState  = StateIdle;
SystemState prevSystemState = StateIdle;

LiquidCrystal_I2C lcd(LcdI2cAddr, 16, 2);
Servo scannerServo;
VL53L1X lidar;
OneWire oneWire(TempSensorPin);
DallasTemperature tempSensor(&oneWire);

unsigned long matzahReadyEntryTime = 0;

const int ServoCwUs   = 1645;   // clockwise speed
const int ServoCcwUs  = 1335;   // counter clockwise speed
const int ServoStopUs = 1490;   // stop
const unsigned long SweepDurationMs = 1100;   // ms per sweep direction
const unsigned long SweepSettleMs   = 300;   // stop pause between direction 

int calibD0Mm = 0;

// Live sensor readings
int   enemyDistMm  = 0;
float currentTempC = 0.0;
int   smokeLevel   = 0;

// Non blocking temperature conversion
bool tempConvPending = false;
unsigned long tempRequestTime = 0;
const unsigned long TempConvMs = 200;

// Millis read intervals
unsigned long lastDistReadTime  = 0;
unsigned long lastTempReadTime  = 0;
unsigned long lastSmokeReadTime = 0;
unsigned long lastLcdUpdateTime = 0;
const unsigned long DistReadIntervalMs  = 100;
const unsigned long TempReadIntervalMs  = 2000;
const unsigned long SmokeReadIntervalMs = 1000;
const unsigned long LcdUpdateIntervalMs = 500;

// Button debounce
bool prevBtnState = HIGH;
unsigned long lastBtnPressTime = 0;
const unsigned long DebounceDurationMs = 300;

// Non-blocking buzzer
bool buzzerEnabled = false;
bool buzzerPinOn   = false;
unsigned long lastBuzzerToggle = 0;
unsigned long buzzerOnDurMs  = 500;
unsigned long buzzerOffDurMs = 500;

bool lidarReady = false;

// Baking timeout
unsigned long bakingEntryTime = 0;
const unsigned long BakingTimeoutMs = 50000;

// Smoke baseline — sampled once after warmup, reset each bake cycle
int  smokeBaseline        = 0;
bool smokeBaselineSampled = false;
int  smokeBaselineSum     = 0;
int  smokeBaselineSamples = 0;
unsigned long lastBaselineSampleTime = 0;
const int  BaselineSampleCount       = 5;
const unsigned long BaselineSampleIntervalMs = 400;

// warmup only needed on first bake after power-on; stays true across resets
bool firstBakeDone = false;

// Transfer state timing & routing
unsigned long transferEntryTime   = 0;
bool transferFromCritical  = false;
bool transferMsgShown      = false;
bool transferLcdDistPhase  = false;
const unsigned long TransferDisplayMs = 5000;

// Critical alert display timer
bool criticalAlertPending = false;
unsigned long criticalAlertTime = 0;
const unsigned long CriticalDisplayMs = 1500;

bool fireAlertAcknowledged = false;


void setup() {
  Serial.begin(9600);
  Serial.println(F("=== The Field Bakery ==="));
  Serial.println(F("initializing..."));

  pinMode(BtnStartPin,    INPUT_PULLUP);
  pinMode(GreenLedPin,    OUTPUT);
  pinMode(RedLedPin,      OUTPUT);
  pinMode(YellowLedPin,   OUTPUT);
  pinMode(BuzzerPin,      OUTPUT);
  pinMode(SmokeRelayPin,  OUTPUT);
  pinMode(LidarXshutPin,  OUTPUT);

  setAllOutputsOff();
  digitalWrite(SmokeRelayPin, HIGH);  // MQ2 off at boot (relay turns it on later)

  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcdPrint("Matzah Bakery", "Initializing...");

  digitalWrite(LidarXshutPin, LOW);
  delay(10);
  digitalWrite(LidarXshutPin, HIGH);
  delay(10);

  lidar.setTimeout(500);
  if (!lidar.init()) {
    Serial.println(F("Lidar not found"));
    lcdPrint("ERROR: LIDAR", "Check wiring!");
    lidarReady = false;
  } else {
    lidar.setDistanceMode(VL53L1X::Long);
    lidar.setMeasurementTimingBudget(50000);
    lidar.startContinuous(50);
    lidarReady = true;
  }

  tempSensor.begin();
  tempSensor.setResolution(10);
  tempSensor.setWaitForConversion(false);
  if (tempSensor.getDeviceCount() == 0) {
    Serial.println(F("temp sensor not responding"));
  }

  scannerServo.attach(ServoPin);
  scannerServo.writeMicroseconds(ServoStopUs);

  IrReceiver.begin(IrRecvPin, DISABLE_LED_FEEDBACK);

  transitionTo(StateIdle);
}

void loop() {
  checkTempConversion();
  updateBuzzer();

  if (IrReceiver.decode()) {
    handleIR(IrReceiver.decodedIRData.command);
    IrReceiver.resume();
  }

  switch (systemState) {
    case StateIdle:        stateIdle();        break;
    case StateDoughRising: stateDoughRising(); break;
    case StateEnemyAlert:  stateEnemyAlert();  break;
    case StateTransfer:    stateTransfer();    break;
    case StateBaking:      stateBaking();      break;
    case StateMatzahReady: stateMatzahReady(); break;
    case StateFireAlert:   stateFireAlert();   break;
  }
}

// idle: waiting for start button
void stateIdle() {
  bool btnNow = digitalRead(BtnStartPin);
  if (btnNow == LOW && prevBtnState == HIGH) {
    unsigned long now = millis();
    if (now - lastBtnPressTime >= DebounceDurationMs) {
      lastBtnPressTime = now;
      prevBtnState     = LOW;
      transitionTo(StateDoughRising);
    }
  }
  if (btnNow == HIGH) prevBtnState = HIGH;
}

// dough rising: servo sweeps CW then CCW with a settle pause at each reversal
void stateDoughRising() {
  scannerServo.writeMicroseconds(ServoCwUs);
  if (scanWait(SweepDurationMs)) return;
  scannerServo.writeMicroseconds(ServoStopUs);
  if (scanWait(SweepSettleMs)) return;
  scannerServo.writeMicroseconds(ServoCcwUs);
  if (scanWait(SweepDurationMs)) return;
  scannerServo.writeMicroseconds(ServoStopUs);
  scanWait(SweepSettleMs);
}

// enemy alert: red LED and buzzer, live distance on LCD
void stateEnemyAlert() {
  if (criticalAlertPending) {
    if (millis() - criticalAlertTime >= CriticalDisplayMs) {
      criticalAlertPending = false;
      transferFromCritical = true;
      transitionTo(StateTransfer);
    }
    return;
  }

  unsigned long t = millis();

  if (t - lastDistReadTime >= DistReadIntervalMs && lidarReady) {
    lastDistReadTime = t;
    enemyDistMm = lidar.read(false);
    if (lidar.timeoutOccurred()) enemyDistMm = 0;

    if (enemyDistMm > 0 && enemyDistMm < CriticalThresholdMm) {
      Serial.println(F("enemy too close, baking now"));
      setAllLedsOff();
      setBuzzer(false);
      lcdPrint("No time rising!", "Bake NOW!!");
      criticalAlertPending = true;
      criticalAlertTime    = t;
      return;
    }
  }

  if (t - lastLcdUpdateTime >= LcdUpdateIntervalMs) {
    lastLcdUpdateTime = t;
    int distCm = enemyDistMm / 10;
    lcd.setCursor(0, 0);
    lcd.print("!! ENEMY !!     ");
    lcd.setCursor(0, 1);
    lcd.print("Dist:");
    lcd.print(distCm);
    lcd.print("cm          ");
  }
}

// transfer: dough on its way to oven, waiting for button 2
void stateTransfer() {
  unsigned long t = millis();

  if (t - lastDistReadTime >= DistReadIntervalMs && lidarReady) {
    lastDistReadTime = t;
    enemyDistMm = lidar.read(false);
    if (lidar.timeoutOccurred()) enemyDistMm = 0;
  }

  if (!transferFromCritical && !transferMsgShown) {
    if (t - transferEntryTime >= TransferDisplayMs) {
      transferMsgShown     = true;
      transferLcdDistPhase = false;
      lastLcdUpdateTime    = 0;
      Serial.println(F("dough in oven, press 2 to bake"));
    }
    return;
  }

  if (t - lastLcdUpdateTime >= 2000) {
    lastLcdUpdateTime    = t;
    transferLcdDistPhase = !transferLcdDistPhase;

    if (transferLcdDistPhase) {
      int distCm = enemyDistMm / 10;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Incoming: ");
      if (distCm > 0) { lcd.print(distCm); lcd.print("cm"); }
      else             { lcd.print("None"); }
      lcd.setCursor(0, 1);
      lcd.print("Press 2:Bake    ");
    } else {
      lcdPrint("Press 2 to", "turn on Oven");
    }
  }
}

// baking: monitor temp and smoke
void stateBaking() {
  unsigned long t = millis();

  if (t - lastTempReadTime >= TempReadIntervalMs && !tempConvPending) {
    lastTempReadTime = t;
    tempSensor.requestTemperatures();
    tempConvPending = true;
    tempRequestTime = t;
  }

  if (currentTempC >= BakeTempThreshold && currentTempC > 0) {
    setLEDs(false, false, true);
    transitionTo(StateMatzahReady);
    return;
  }

  if (t - bakingEntryTime >= BakingTimeoutMs) {
    Serial.println(F("baking timeout, matzahs done"));
    transitionTo(StateMatzahReady);
    return;
  }

  // phase 1: warmup, only on first bake 
  if (!firstBakeDone && t - bakingEntryTime < Mq2BakeWarmupMs) {
    if (t - lastSmokeReadTime >= SmokeReadIntervalMs) {
      lastSmokeReadTime = t;
      smokeLevel = analogRead(SmokeSensorPin);
    }
    if (t - lastLcdUpdateTime >= LcdUpdateIntervalMs) {
      lastLcdUpdateTime = t;
      unsigned long secsLeft = (Mq2BakeWarmupMs - (t - bakingEntryTime)) / 1000 + 1;
      lcd.setCursor(0, 0); lcd.print("Baking          ");
      lcd.setCursor(0, 1);
      lcd.print("Warm:");
      lcd.print(secsLeft);
      lcd.print("s Smk:");
      lcd.print(smokeLevel);
      lcd.print("  ");
    }
    return;
  }

  // phase 2: sample baseline (runs immediately on bakes 2+)
  if (!smokeBaselineSampled) {
    if (t - lastBaselineSampleTime >= BaselineSampleIntervalMs) {
      lastBaselineSampleTime = t;
      smokeBaselineSum += analogRead(SmokeSensorPin);
      smokeBaselineSamples++;
      if (smokeBaselineSamples >= BaselineSampleCount) {
        smokeBaseline        = smokeBaselineSum / BaselineSampleCount;
        smokeBaselineSampled = true;
        firstBakeDone        = true;
        Serial.print(F("smoke baseline: "));
        Serial.println(smokeBaseline);
      }
    }
    if (t - lastLcdUpdateTime >= LcdUpdateIntervalMs) {
      lastLcdUpdateTime = t;
      lcd.setCursor(0, 0); lcd.print("Baking Matzahs  ");
      lcd.setCursor(0, 1); lcd.print("Calibrating...  ");
    }
    return;
  }

  // phase 3: active monitoring, fire if reading exceeds baseline by delta
  if (t - lastSmokeReadTime >= SmokeReadIntervalMs) {
    lastSmokeReadTime = t;
    smokeLevel = analogRead(SmokeSensorPin);
    if (smokeLevel > (int)(smokeBaseline * SmokeMultiplier)) {
      setLEDs(false, false, true);
      transitionTo(StateFireAlert);
      return;
    }
  }

  if (t - lastLcdUpdateTime >= LcdUpdateIntervalMs) {
    lastLcdUpdateTime = t;
    lcd.setCursor(0, 0); lcd.print("Baking Matzahs  ");
    lcd.setCursor(0, 1);
    lcd.print("Temp:");
    lcd.print((int)currentTempC);
    lcd.print("C Smk:");
    lcd.print(smokeLevel);
    lcd.print("  ");
  }
}

// matzah ready: show ready message then Moses
void stateMatzahReady() {
  unsigned long elapsed = millis() - matzahReadyEntryTime;
  if (elapsed < 2500) {
    if (millis() - lastLcdUpdateTime >= 500) {
      lastLcdUpdateTime = millis();
      lcdPrint("Matzahs Ready!", "");
    }
  } else {
    if (millis() - lastLcdUpdateTime >= 1000) {
      lastLcdUpdateTime = millis();
      lcdPrint("Moses is waiting", "time to run!    ");
    }
  }
}

// fire alert: smoke detected, wait for acknowledgement then reset
void stateFireAlert() {
  if (millis() - lastLcdUpdateTime >= 1000) {
    lastLcdUpdateTime = millis();
    lcd.setCursor(0, 0); lcd.print("Matzahs Burnt!  ");
    if (fireAlertAcknowledged) {
      lcd.setCursor(0, 1); lcd.print("it happens :(   ");
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Smoke:");
      lcd.print(smokeLevel);
      lcd.print(" Prs3   ");
    }
  }
}

// handles incoming IR commands
void handleIR(uint8_t cmd) {
  if (cmd == 0xFF) return;  // repeat frame, ignore

  if (cmd == IrCmdReset) {
    if (systemState == StateMatzahReady) {
      transitionTo(StateDoughRising);
    } else {
      resetSystem();
    }

  } else if (cmd == IrCmdTransfer) {
    if (systemState == StateDoughRising || systemState == StateEnemyAlert) {
      setAllLedsOff();
      setBuzzer(false);
      transferFromCritical = false;
      Serial.println(F("dough sent to oven"));
      transitionTo(StateTransfer);
    } else {
      Serial.println(F("cant do that now"));
    }

  } else if (cmd == IrCmdBake) {
    if (systemState == StateTransfer) {
      setLEDs(true, false, false);
      Serial.println(F("oven on"));
      transitionTo(StateBaking);
    } else {
      Serial.println(F("transfer dough first"));
    }

  } else if (cmd == IrCmdBurnt) {
    if (systemState == StateFireAlert) {
      fireAlertAcknowledged = true;
      Serial.println(F("fire alert acknowledged, press 0 to reset"));
      lcdPrint("Matzahs Burnt!", "it happens :(");
    } else {
      Serial.println(F("only works during fire alert"));
    }
  }
}

void returnServoToCenter() {
  scannerServo.writeMicroseconds(ServoStopUs);
  delay(200);
}

// Waits ms while checking LIDAR and IR every tick.
// Returns true the moment the state changes
bool scanWait(unsigned long ms) {
  unsigned long end = millis() + ms;
  while (millis() < end) {
    delay(1);

    if (IrReceiver.decode()) {
      handleIR(IrReceiver.decodedIRData.command);
      IrReceiver.resume();
      if (systemState != StateDoughRising) return true;
    }

    if (lidarReady && millis() - lastDistReadTime >= DistReadIntervalMs) {
      lastDistReadTime = millis();
      enemyDistMm = lidar.read(false);
      if (lidar.timeoutOccurred()) enemyDistMm = 0;
      if (enemyDistMm > 0 && enemyDistMm < EnemyThresholdMm) {
        transitionTo(StateEnemyAlert);
        return true;
      }
    }

    if (millis() - lastLcdUpdateTime >= LcdUpdateIntervalMs) {
      lastLcdUpdateTime = millis();
      int distCm = enemyDistMm / 10;
      lcd.setCursor(0, 0); lcd.print("Dough Rising    ");
      lcd.setCursor(0, 1); lcd.print("1:Move Dst:");
      lcd.print(distCm); lcd.print("cm  ");
    }
  }
  return false;
}

// checks if the temp sensor finished converting, called every loop
void checkTempConversion() {
  if (!tempConvPending) return;
  if (millis() - tempRequestTime < TempConvMs) return;

  float t = tempSensor.getTempCByIndex(0);
  if (t != DEVICE_DISCONNECTED_C && t > -50.0 && t != 85.0) {
    currentTempC = t;
  } else if (t == 85.0) {
    Serial.println(F("temp sensor returned 85C, check pull-up on pin 12"));
  } else {
    Serial.println(F("temp sensor not responding, check pull-up on pin 12"));
  }
  tempConvPending = false;
}

// toggles buzzer on a timer
void updateBuzzer() {
  if (!buzzerEnabled) {
    if (buzzerPinOn) {
      digitalWrite(BuzzerPin, LOW);
      buzzerPinOn = false;
    }
    return;
  }
  unsigned long interval = buzzerPinOn ? buzzerOnDurMs : buzzerOffDurMs;
  if (millis() - lastBuzzerToggle >= interval) {
    lastBuzzerToggle = millis();
    buzzerPinOn      = !buzzerPinOn;
    digitalWrite(BuzzerPin, buzzerPinOn ? HIGH : LOW);
  }
}

void setBuzzerPattern(unsigned long onMs, unsigned long offMs) {
  buzzerOnDurMs    = onMs;
  buzzerOffDurMs   = offMs;
  buzzerEnabled    = true;
  lastBuzzerToggle = millis();
}

void setBuzzer(bool on) {
  if (on) {
    setBuzzerPattern(buzzerOnDurMs, buzzerOffDurMs);
  } else {
    buzzerEnabled = false;
    buzzerPinOn   = false;
    digitalWrite(BuzzerPin, LOW);
  }
}

void setLEDs(bool green, bool red, bool yellow) {
  digitalWrite(GreenLedPin,  green  ? HIGH : LOW);
  digitalWrite(RedLedPin,    red    ? HIGH : LOW);
  digitalWrite(YellowLedPin, yellow ? HIGH : LOW);
}

void setAllLedsOff()    { setLEDs(false, false, false); }
void setAllOutputsOff() { setAllLedsOff(); setBuzzer(false); }

void lcdPrint(const char* line1, const char* line2) {
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print(line1);
  lcd.setCursor(0, 1); lcd.print(line2);
  lastLcdUpdateTime = millis();
}

void showMosesMessage() {
  setAllOutputsOff();
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("Moses is waiting");
  lcd.setCursor(0, 1); lcd.print("time to run!    ");
  Serial.println(F("Moses is waiting, time to run!"));
  delay(3000);
}

// changes active state and runs entry actions
void transitionTo(SystemState newState) {
  const char* stateNames[] = {
    "IDLE", "DOUGH_RISING", "ENEMY_ALERT",
    "TRANSFER", "BAKING", "MATZAH_READY", "FIRE_ALERT"
  };
  if (newState != systemState) {
    Serial.print(stateNames[systemState]);
    Serial.print(F(" -> "));
    Serial.println(stateNames[newState]);
  }

  prevSystemState = systemState;
  systemState = newState;
  lastLcdUpdateTime = 0;

  switch (newState) {

    case StateIdle:
      setAllOutputsOff();
      digitalWrite(SmokeRelayPin, HIGH);  // sensor off
      scannerServo.writeMicroseconds(ServoStopUs);
      prevBtnState = digitalRead(BtnStartPin);
      lastBtnPressTime = millis();
      lcdPrint("Matzah Bakery", "Press Start");
      break;

    case StateDoughRising:
      scannerServo.writeMicroseconds(ServoStopUs);
      if (lidarReady) {
        delay(300);
        calibD0Mm = lidar.read(false);
        Serial.print(F("scan start dist: "));
        Serial.print(calibD0Mm / 10);
        Serial.println(F("cm"));
      }
      lcdPrint("Dough Rising", "1:Move to Oven");
      break;

    case StateEnemyAlert:
      scannerServo.writeMicroseconds(ServoStopUs);
      setLEDs(false, true, false);
      setBuzzerPattern(400, 200);
      Serial.print(F("enemy at "));
      Serial.print(enemyDistMm / 10);
      Serial.println(F("cm"));
      lcd.clear();
      lcd.setCursor(0, 0); lcd.print("!! ENEMY !!");
      lcd.setCursor(0, 1);
      lcd.print("Dist:");
      lcd.print(enemyDistMm / 10);
      lcd.print("cm");
      lastLcdUpdateTime = millis();
      break;

    case StateTransfer:
      setAllLedsOff();
      setBuzzer(false);
      transferEntryTime    = millis();
      transferMsgShown     = false;
      transferLcdDistPhase = false;
      if (transferFromCritical) {
        lcdPrint("Press 2 to", "turn on Oven");
        transferMsgShown = true;
        Serial.println(F("auto transfer, press 2 to bake"));
      } else {
        lcdPrint("Transferring...", "Dough to oven");
        Serial.println(F("dough moving to oven, press 2 to bake"));
      }
      break;

    case StateBaking:
      bakingEntryTime = millis();
      smokeBaseline = 0;
      smokeBaselineSampled = false;
      smokeBaselineSum  = 0;
      smokeBaselineSamples = 0;
      lastBaselineSampleTime = 0;
      digitalWrite(SmokeRelayPin, LOW);   // smoke sensor on 
      returnServoToCenter();
      lcdPrint("Baking", "Oven ON");
      break;

    case StateMatzahReady:
      setLEDs(false, false, true);
      setBuzzer(false);
      digitalWrite(SmokeRelayPin, HIGH);  // sensor off
      scannerServo.writeMicroseconds(ServoStopUs);
      matzahReadyEntryTime = millis();
      Serial.print(F("matzahs done, temp was "));
      Serial.print(currentTempC, 1);
      Serial.println(F("C"));
      lcdPrint("Matzahs Ready!", "");
      break;

    case StateFireAlert:
      fireAlertAcknowledged = false;
      setLEDs(false, false, true);
      setBuzzerPattern(150, 100);
      digitalWrite(SmokeRelayPin, HIGH);  
      scannerServo.writeMicroseconds(ServoStopUs);
      Serial.print(F("smoke detected, level "));
      Serial.print(smokeLevel);
      Serial.println(F(", press 3 to acknowledge"));
      lcdPrint("Matzahs Burnt!", "Press 3");
      break;

    default: break;
  }
}

// full system reset back to idle
void resetSystem() {
  setAllOutputsOff();
  digitalWrite(SmokeRelayPin, HIGH);  
  scannerServo.writeMicroseconds(ServoStopUs);
  currentTempC = 0.0;
  smokeLevel   = 0;
  enemyDistMm  = 0;
  prevBtnState = HIGH;
  tempConvPending = false;
  buzzerOnDurMs = 500;
  buzzerOffDurMs = 500;
  transferFromCritical = false;
  transferMsgShown = false;
  transferLcdDistPhase = false;
  criticalAlertPending = false;
  fireAlertAcknowledged = false;
  smokeBaseline = 0;
  smokeBaselineSampled = false;
  smokeBaselineSum = 0;
  smokeBaselineSamples = 0;
  if (lidarReady) lidar.startContinuous(50);
  Serial.println(F("system reset"));
  transitionTo(StateIdle);
}
