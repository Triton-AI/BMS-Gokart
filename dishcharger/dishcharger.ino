//Libraries
#include "PINOUTS.h"
#include "GLOBALS.h"
#include "INIT_OLED.h"   //I2C Driver
#include "ADC_CAL.h"
#include <esp_adc_cal.h>  // ADC Calibration Library
#include "NOKIA.h"

void setup() {

  //Init serial and pins
  Serial.begin(9600);
  pinMode(powerPin, OUTPUT);
  pinMode(buzz, OUTPUT);
  pinMode(PB1, INPUT_PULLUP);  // Use internal pull-up resistor
  digitalWrite(powerPin, LOW);
  noTone(buzz);

  //ADC Configuration
  analogReadResolution(12);                    // Set resolution to 12-bit
  analogSetPinAttenuation(VBATPin, ADC_11db);  // 0-3.3V range
  analogSetPinAttenuation(currentPin, ADC_11db);
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adc_chars);

  //OLED Configuration
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  //start the oled called display with a a hex addy of 0x3c
  display.clearDisplay();                     //clear the screen

  initbuzz();  //initial buzzer sound.
  delay(1000);
  display.clearDisplay();  //clear the screen.
  display.display();       //show the buffer.

  //init Current Sensor Calibration
  CurrentSensorCalibrate();  //Calibrate the current sensor.
  minVBAT = VBAT;            //set minimum to initial VBAT
  lastLoopTime = millis();   //set to current time to start fixed loop rate
}
void loop() {

  unsigned long currentTime = millis();
  totalLoopTime = currentTime - lastLoopTime;
  // Check if enough time has passed for the fixed loop rate (50ms)
  if (totalLoopTime >= LOOP_INTERVAL_MS) {
    lastLoopTime = currentTime;

    loopTimeSample();  //Check instant update rate.
    buttonInputs();    //gets button status.
    adcSamples();      //Gets VBAT and current sense.
    coulombCounter();  //Adder for current sense and 1 second timer.
    minVmaxA();        //Save minimum voltage and maximum amps.
    statusCheck();     //Controls power fets, checks button logic, capacity testing, vbat, buzzer and alarms.
    OLEDdisplay();     //outputs globals and special UI screens.
    buzzer();          //Calls buzzer when triggered.
    printOUT();        //Serial output.
  }
  // Optional: small delay to give ESP32 a chance to process background tasks
  delay(1);
}
void initbuzz() {
  //initial buzzer sound.
  tone(buzz, freq);
  delay(100);
  noTone(buzz);
  delay(50);

}
void song() {
  int size = sizeof(durations) / sizeof(int);
  //int OFFSET = 3500;

  for (int note = 0; note < size; note++) {
    //to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int duration = 750 / durations[note];
    tone(buzz, melody[note], duration);

    //to distinguish the notes, set a minimum time between them.
    //the note's duration + 30% seems to work well:
    int pauseBetweenNotes = duration * 1.30;
    delay(pauseBetweenNotes);

    //stop the tone playing:
    noTone(buzz);
  }
}
void CurrentSensorCalibrate() {
  //Calibrate the current sensor.
  delay(100);
  adcSamples();
  ZERO_Amp = low_voltage_Isense;
  Serial.print("-----------------------| Current sensor calibrated:");
  Serial.print(ZERO_Amp, 3);
  Serial.print(" | BATTERY:");
  Serial.print(RESULT);
  Serial.println(" |-----------------------");
}
void powerFetsControl() {
  digitalWrite(powerPin, FETS);
}
void loopTimeSample() {
  unsigned long now = millis();
  if (x == 0) {
    x = now;
  }
  z = now - x;
  x = now;
}
void buttonInputs() {
  //gets button status for instant and long press.
  buttonPressed = false;
  buttonLongPressed = false;
  buttonStatePB1 = digitalRead(PB1);
  if (buttonStatePB1 == LOW) {  //PRESSED
    buttonPressed = true;       //instance
    if (buttonTimerStart == 0.0) {
      buttonTimerStart = millis();
    }
    buttonTimerEnd = millis();
    buttonTimerTotal = buttonTimerEnd - buttonTimerStart;
    if (buttonTimerTotal >= fullPressTime) {  //real press timer to filter button noise.
      buttonLongPressed = true;
    }
  } else {
    buttonTimerStart = 0.0;
  }
}
void adcSamples() {
  //Gets VBAT and current sense only.
  uint32_t rawADC_VBAT = 0;
  uint32_t rawADC_Isense = 0;
  // Low-pass filtering over LPF samples
  for (int i = 0; i < LPF; i++) {
    rawADC_VBAT += analogRead(VBATPin);
    rawADC_Isense += analogRead(currentPin);
  }
  // Average the raw ADC values
  rawADC_VBAT /= LPF;
  rawADC_Isense /= LPF;
  // Convert raw ADC to voltage using calibration
  uint32_t voltage_VBAT_mV = esp_adc_cal_raw_to_voltage(rawADC_VBAT, &adc_chars);
  uint32_t voltage_Isense_mV = esp_adc_cal_raw_to_voltage(rawADC_Isense, &adc_chars);
  // Convert mV to V and apply offsets
  low_voltage_VBAT = voltage_VBAT_mV / 1000.0 + OFFSET_VBAT;  // Convert to volts
  low_voltage_Isense = voltage_Isense_mV / 1000.0 + OFFSET_AMP;

  VBAT = low_voltage_VBAT * HIGH_VOLT_RATIO;
  AMPS = (low_voltage_Isense - ZERO_Amp) / SENSE;
  if (AMPS < 0.0) {
    AMPS = 0.0;
  }
  WATTS = VBAT * AMPS;
}
void coulombCounter() {
  //Adder for current sense and 1 second timer.
  mA = AMPS * 1000;    //convert to mA
  coulomb = mA / SPH;  //find coulomb constant.
  //1 second timer for coulomb counting
  if (coulombTimerStart == 0.0) {
    coulombTimerStart = millis();
  }
  coulombTimerEnd = millis();
  coulombTimerTotal = coulombTimerEnd - coulombTimerStart;
  if (coulombTimerTotal >= COULOMBSECOND) {
    coulombTimerStart = 0.0;

    if (TEST && !CAL && !DONE && !PAUSE && !ERROR) {  //Start coulomb countger if testing is running
      //mAh = mAh + coulomb;                            //add up the coulombs per second.
      mAh = mAh + (coulomb * CORRECTION_FACTOR);      //add up the coulombs per second with 2.3% correction factor.
    }
    SERIALPRINT = true;  //print out results after coulomb sampled and mAh calculated.
  }

  ampHOUR = mAh / 1000.0;  //amp hour conversion.
  wattHOUR = ampHOUR * voltNOM;
  PERCENT = ampHOUR / CAPACITY_SET * 100.0;
}
void minVmaxA() {
  //Check volts conditon
  if ((VBAT > lowVoltageOFF) && (VBAT < highVoltageOFF)) {
    CHECK_VOLTS = true;
  } else {
    CHECK_VOLTS = false;
  }
  //check for over voltage.
  if ((VBAT >= highVoltageOFF)) {
    RESULT = "ERR.";  //stuck until reboot
    DONE = true;      //stuck until reboot
    ERROR = true;     //stuck until reboot
    doneBuzz = 3;     //stuck until reboot
  }
  //Check amp conditon
  if ((AMPS > lowAmpsOFF) && (AMPS < highAmpsOFF)) {
    CHECK_AMPS = true;
  } else {
    CHECK_AMPS = false;
  }
  //memory of min of volts
  if (!DONE && !PAUSE) {
    if (voltTimerStart == 0.0) {
      voltTimerStart = millis();
    }
    voltTimerEnd = millis();
    voltTimerTotal = voltTimerEnd - voltTimerStart;
    if (voltTimerTotal >= minVoltTime) {  //real press timer to filter button noise.
      voltTimerStart = 0.0;
      if (VBAT < minVBAT) {
        minVBAT = VBAT;
      }
    }
  }
  //memory of max of amps
  if (AMPS > maxAMPS) {
    maxAMPS = AMPS;
  }
}
void statusCheck() {
  //Controls power fets, checks button logic, capacity testing, vbat, buzzer and alarms.
  /*
  Check volts conditon
  start countdown
  start test, fets on
  check current condition
  check errors
  bool newSession = true; //init true on boot.
  bool STOP = true;
  bool TEST = false;
  bool FETS = false;
  bool PAUSE = false;
  bool ERROR = false;
  bool DONE = false;
  bool CHECK_VOLTS = false;
  bool CHECK_AMP = false;
  */
  //  start countdown
  if (!DONE && buttonLongPressed && newSession && CHECK_VOLTS) {
    newSession = false;  //stuck until reboot
    COUNTDOWN = true;
    TEST = true;
    displaySTATE = "TESTING...";
    CAL = true;
  }

  //start test, fets on
  if (!DONE && CAL && TEST && !newSession && !COUNTDOWN) {
    minVBAT = VBAT;  //set minimum to initial VBAT
    OLEDdisplay();   //outputs globals and special UI screens.
    delay(500);
    CurrentSensorCalibrate();
    delay(500);
    FETS = true;
    powerFetsControl();
    CAL = false;              //stuck until reboot
    coulombTimerStart = 0.0;  //reset coulomb timer to start with fresh time on first sample.
  }
  //pause button
  if (!DONE && buttonPressed && TEST && !CAL && !pauseTriggered) {
    pauseTriggered = true;
    PAUSE = !PAUSE;
    if (PAUSE) {
      FETS = false;
      powerFetsControl();
      displaySTATE = "  PAUSED";
    } else {
      FETS = true;
      powerFetsControl();
      displaySTATE = "TESTING...";
    }
  }
  //reset pause button trigger
  if (!DONE && !buttonPressed && TEST) {
    pauseTriggered = false;
  }
  if (!DONE && TEST && !PAUSE && !CHECK_VOLTS) {
    TEST = false;  //stuck until reboot
    FETS = false;
    powerFetsControl();
    DONE = true;  //stuck until reboot
  }
  //end of test
  if (DONE) {
    FETS = false;
    powerFetsControl();

    if (doneBlink) {
      if (ERROR) {
        displaySTATE = "!OVERVOLT!";
      } else {
        displaySTATE = "  DONE!!!";
      }

      if (!ENDBEEP) {
        if (doneBuzz < endBuzz) {
          doneBuzz = doneBuzz + 1;
          //song();
          ENDBEEP = true;
        }
      }
    } else {
      displaySTATE = "";
      ENDBEEP = false;
    }
    if (doneTimerStart == 0.0) {
      doneTimerStart = millis();
    }
    doneTimerEnd = millis();
    doneTimerTotal = doneTimerEnd - doneTimerStart;
    if (doneTimerTotal >= doneBlinkTime) {  //real press timer to filter button noise.
      doneTimerStart = 0.0;
      doneBlink = !doneBlink;
    }
    if ((PERCENT >= CAPACITY_ERROR) || (ERROR == true)) {
      RESULT = "ERR.";
    } else if (PERCENT >= CAPACITY_PASS) {
      RESULT = "PASS";
    } else {
      RESULT = "FAIL";
    }
  }
}
void OLEDdisplay() {
  //outputs globals and special UI screens.
  //SCREEN_WIDTH 128 X
  //SCREEN_HEIGHT 64 Y
  display.clearDisplay();  //always start with a clear

  if (!COUNTDOWN) {
    display.setTextColor(WHITE);  //set text color
    display.setTextSize(2);       //set up text size
    display.setCursor(0, 0);      //where to position cursor (128,64)
    display.print(RESULT);        //Status
    display.setTextSize(1);
    display.setCursor(55, 7);
    display.print(ampHOUR, 2);  //amp hour
    display.print("Ah ");
    display.print(PERCENT, 0);  //percent
    display.print("%");
    // button pressed
    if (buttonPressed) {
      display.setTextSize(2);
      display.setCursor(0, 16);
      display.print("*");
    }
    display.setTextSize(1);
    //status
    display.setCursor(25, 20);
    display.print(displaySTATE);
    //wh
    display.setCursor(95, 20);
    display.print(wattHOUR, 0);
    display.print("Wh");
    // min volts
    display.setCursor(2, 37);
    display.print("Min:");
    display.print(minVBAT, 1);
    display.print("V");
    // max amps
    display.setCursor(72, 37);
    display.print("Max:");
    display.print(maxAMPS, 1);
    display.print("A");
    // Realtime V / W / A (Now at the bottom)
    display.setTextSize(1);
    display.drawRect(0, 49, 128, 15, WHITE);  // moved from y=16 to y=49
    display.setCursor(4, 53);                 // moved from y=20 to y=53
    display.print(VBAT, 1);
    display.print("V");
    display.setCursor(50, 53);
    display.print(AMPS, 1);
    display.print("A");
    display.setCursor(99, 53);
    display.print(WATTS, 0);
    display.print("W");
    display.display();  //show the buffer
  }

  else {
    if (countTimerStart == 0.0) {
      countTimerStart = millis();
      display.setTextColor(WHITE);  //set text color
      display.setTextSize(2);
      display.setCursor(16, 0);  // Centered in the yellow area
      display.print("STARTING");
      display.setTextSize(5);
      display.setCursor(49, 20);
      display.print(n);
      display.display();  //show the buffer
    }
    countTimerEnd = millis();
    countTimerTotal = countTimerEnd - countTimerStart;
    if (countTimerTotal >= countTimer) {
      n = n - 1;
      countTimerStart = 0.0;
      if (n == 0) {
        COUNTDOWN = false;  //stuck until reboot
      }
    }
  }
}
void buzzer() {
  //Calls buzzer when triggered.

  if (buttonPressed) {
    if (freqTimerStart == 0.0) {
      freqTimerStart = millis();
      tone(buzz, freq);  //start buzzing
      BEEP = true;
    }
    freqTimerEnd = millis();
    freqTimerTotal = freqTimerEnd - freqTimerStart;
    if (freqTimerTotal >= freqON) {  //check if exceeds ON time
      noTone(buzz);                  //turn off buzzer.
      BEEP = false;
    }
  } else if (ENDBEEP) {
    if (!BEEP) {
      tone(buzz, freq);  //start buzzing
      BEEP = true;
    }
  } else {
    freqTimerStart = 0.0;
    noTone(buzz);  //turn off buzzer.
    BEEP = false;
  }
}
void printOUT() {
  //Serial output.
  if (SERIALPRINT) {
    Serial.print(" new");
    Serial.print(newSession);
    Serial.print(" C");
    Serial.print(CAL);
    Serial.print(" T");
    Serial.print(TEST);
    Serial.print(" F");
    Serial.print(FETS);
    Serial.print(" P");
    Serial.print(PAUSE);
    Serial.print(" E");
    Serial.print(ERROR);
    Serial.print(" D");
    Serial.print(DONE);
    Serial.print(" CV");
    Serial.print(CHECK_VOLTS);
    Serial.print(" CA");
    Serial.print(CHECK_AMPS);
    Serial.print(" PB");
    Serial.print(buttonPressed);
    Serial.print(",");
    Serial.print(buttonLongPressed);
    Serial.print(" B");
    Serial.print(BEEP);
    Serial.print(" CAP");
    Serial.print(CAPACITY_SET, 2);
    Serial.print("Ah | ");
    Serial.print("(");
    Serial.print(low_voltage_VBAT, 3);
    Serial.print(")VBAT=");
    Serial.print(VBAT, 2);
    Serial.print(" (");
    Serial.print(low_voltage_Isense, 3);
    Serial.print(")AMPS=");
    Serial.print(AMPS, 2);
    Serial.print(" mA=");
    Serial.print(mA, 2);
    Serial.print(" coulomb=");
    Serial.print(coulomb, 2);
    Serial.print(" mAh=");
    Serial.print(mAh, 2);
    Serial.print("=");
    Serial.print(ampHOUR, 2);
    Serial.print("Ah=");
    Serial.print(PERCENT, 1);
    Serial.print("%");
    Serial.print(" WATTS=");
    Serial.print(WATTS, 1);
    Serial.print(" coulombIntervals=");
    Serial.print(coulombTimerTotal, 1);
    Serial.print("ms");
    Serial.print(" insideLoop=");
    Serial.print(z, 1);
    Serial.print("ms");
    Serial.print(" totalLoop=");
    Serial.print(totalLoopTime, 1);
    Serial.println("ms");
    SERIALPRINT = false;
  }
}