// ALL GLOBALS

//Battery types
#define LFP50AH 0 
#define LIPO 1

//Battery types
#define BATTEY_TYPE LFP50AH  //Set battery profile
#if (BATTEY_TYPE == LFP50AH)
//PROFILE 24V LFP 50AH
#include "LFP50AH_PROFILE.h"
#elif (BATTEY_TYPE == LIPO)
//PROFILE LIPO CUSTOM CONFIG
#include "LIPO_PROFILE.h"
#endif

//Capacity PASS/FAIL/ERROR limits
#define CAPACITY_PASS 79.6  //% 80% cuttoff
#define CAPACITY_ERROR 105  //% high cap error

//Battery error correction factor
#define CORRECTION_FACTOR 1.0      //LIPO
//#define CORRECTION_FACTOR 1.0592   //30AH

//Fixed loop rate 50ms = 20hz
const unsigned long LOOP_INTERVAL_MS = 50;  // Fixed loop interval (50 ms for 20 Hz)
unsigned long lastLoopTime = 0;             // Time of the last loop iteration
unsigned long totalLoopTime = 0;            // Time of the last loop iteration
unsigned long loopStartTime = 0;            // Time when the current loop starts

//ADC
float low_voltage_VBAT = 0.0;
float low_voltage_Isense = 0.0;
float VBAT = 0.0;
float AMPS = 0.0;
float WATTS = 0.0;

//volts and amps min and max
float minVBAT = 0.0;
float maxAMPS = 0.0;
float minVoltTime = 3000.0;  //ms
float voltTimerStart = 0.0, voltTimerEnd = 0.0, voltTimerTotal = 0.0;

//power fets
bool powered = false;
bool FETS = false;

//Coulombs
#define COULOMBSECOND 1000.0
#define SPH 3600               //seconds per hour = 3600s / h
float coulomb = 0.0;           // mA per SPH
float mA = 0.0;                //instant mA sample.
float mAh = 0.0;               //mAh adder.
float ampHOUR = mAh / 1000.0;  //amp hour sample conversion.
float wattHOUR = ampHOUR * voltNOM;
float PERCENT = ampHOUR / CAPACITY_SET * 100.0;
float coulombTimerStart = 0.0, coulombTimerEnd = 0.0, coulombTimerTotal = 0.0;

//button
float fullPressTime = 1000.0;  //ms
bool buttonPressed = false;
bool buttonLongPressed = false;
bool buttonStatePB1 = false;
float buttonTimerStart = 0.0, buttonTimerEnd = 0.0, buttonTimerTotal = 0.0;

//status
bool newSession = true;  //init true on boot.
bool pauseTriggered = false;
bool CAL = false;
bool STOP = true;
bool TEST = false;
bool PAUSE = false;
bool ERROR = false;
bool DONE = false;
bool CHECK_VOLTS = false;
bool CHECK_AMPS = false;
String displaySTATE = "   READY";
bool doneBlink = false;
float doneBlinkTime = 750.0;  //ms
float doneTimerStart = 0.0, doneTimerEnd = 0.0, doneTimerTotal = 0.0;

//buzzer
#define endBuzz 4  //3 buzzes to singal end.
int doneBuzz = 0;
int freq = 4125;
bool BEEP = false;
bool ENDBEEP = false;
bool LONGBEEP = false;
bool ALARM = false;
float freqON = 100.0;
float freqOFF = 50.0;
float freqLONG = 250.0;
float freqTimerStart = 0.0, freqTimerEnd = 0.0, freqTimerTotal = 0.0;

//OLED
float PERCENT_BAR = 0.0;
float CAPACITY_SET_BAR = 0.0;
bool COUNTDOWN = false;
int n = 3;
float countTimer = 1000.0;
float countTimerStart = 0.0, countTimerEnd = 0.0, countTimerTotal = 0.0;

//serial
bool SERIALPRINT = false;
float serialUpdateRate = 100.0;
float waitTimerStart = 0.0, waitTimerEnd = 0.0, waitTimer = 0.0;
float x = 0.0, y = 0.0, z = 0.0;

/*
  if (waitTimerStart == 0.0) {
    waitTimerStart = millis();
  }
  waitTimer = waitTimerEnd - waitTimerStart;
  if (waitTimer >= serialUpdateRate) {
    waitTimerStart = 0.0;
      } else {
    waitTimerEnd = millis();
  }
*/
