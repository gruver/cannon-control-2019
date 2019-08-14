//SIMPLE CANNON

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

//CANNON TIMER

const int TRIGGER_IN = 4;
const int CANNON_1 = 10;
const int CANNON_2 = 0;
const int CANNON_3 = 0;
const int CANNON_4 = 0;
const int LED_PIN = 12;

const int TIME_ENABLE = 5;
const int TIME_DOWN = 6;
const int TIME_UP = 7;

const int VALVE_ON = 1;
const int VALVE_OFF = 0;

//OPERATION PARAMETERS
unsigned int debounceDelay = 40; //milliseconds - how long to press before firing
unsigned int resetTime = 3 * 1000; //convert to seconds
unsigned int shotInterval = 300; //valve open period; milliseconds

//state timers
unsigned long loopTime = 0; //set time at start of each loop
unsigned long lastTriggerTime = 0; //for debounce
unsigned long valveOpenTime = 0; //track start time of cannon firing
unsigned long debounceTimer = 0;

unsigned int shotCycleCounter = 0; //number of loop cycles per shot, as a cross-check

int valveState = VALVE_OFF;
int triggerDisable = 1; //delay for reset, start delayed

int triggerButtonState = 0;
int lastTriggerButtonState = 0;

int cannonState = 0; //initial state

int timeEnableButtonState = 0;
int timeDownButtonState = 0;
int timeUpButtonState = 0;

const int LED_COUNT = 24;
Adafruit_NeoPixel lightRing(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_7segment timerDisplay = Adafruit_7segment();

void setup() {
  //setup trigger input button pin state
  pinMode(TRIGGER_IN, INPUT_PULLUP);
  pinMode(TIME_ENABLE, INPUT_PULLUP);
  pinMode(TIME_DOWN, INPUT_PULLUP);
  pinMode(TIME_UP, INPUT_PULLUP);

  pinMode(CANNON_1, OUTPUT);

  //setup calce states
  digitalWrite(CANNON_1, VALVE_OFF);
  /*digitalWrite(CANNON_2, VALVE_OFF);
  digitalWrite(CANNON_3, VALVE_OFF);
  digitalWrite(CANNON_4, VALVE_OFF);
  */

  //setup output wire for LEDs
  //display reset lights
  lightRing.begin();
  lightRing.setBrightness(80);
  lightRing.show();
  lightRingBlink();
  
  //setup i2c output
  timerDisplay.begin(0x70);
  //display reset state on display
  timerDisplay.print(3210, DEC);
  timerDisplay.writeDisplay();

  loopTime = millis();
  lastTriggerTime = loopTime - 4000; //delays trigger enable initially
}

void loop() {
  loopTime = millis();
  valveState = VALVE_OFF;
  
  triggerButtonState = digitalRead(TRIGGER_IN) && (!triggerDisable);
  timeEnableButtonState = digitalRead(TIME_ENABLE);
  timeDownButtonState = digitalRead(TIME_DOWN);
  timeUpButtonState = digitalRead(TIME_UP);
  //LOW = pressed - HIGH = open

  if (timeEnableButtonState == LOW) { // ADJUST FIRING TIME
    if (timeDownButtonState == LOW) {
      shotInterval = max(shotInterval - 10, 40);
      delay(50);
    }
    else if (timeUpButtonState == LOW) {
      shotInterval = min(shotInterval + 10, 1500);
      delay(50);
    }
  }
  
  if (triggerButtonState == LOW && timeEnableButtonState == HIGH) { //FIRE CANNONS
    if(lastTriggerButtonState != triggerButtonState) {
      //first press
      lastTriggerTime = loopTime;
      triggerDisable = 1;
      shotCycleCounter = 0;
    }
    //timerDisplay.print(0xFFFF, HEX);
    //timerDisplay.writeDisplay();
  }
  else if (triggerButtonState == HIGH) {
    lightRing.fill(0x009000); // light green for ready
    timerDisplay.print(shotInterval, DEC);
  }
  else {
    //something is wrong
    timerDisplay.print(0x0000, HEX);
  }
  lastTriggerButtonState = triggerButtonState; //reset latch

  if (loopTime - lastTriggerTime < shotInterval) { //CANNONS FIRING
    valveState = VALVE_ON;
    
    shotCycleCounter += 1;
    //timerDisplay.print(shotCycleCounter%10000, DEC);
    timerDisplay.print(loopTime-lastTriggerTime,DEC);

    lightRingSpin(shotCycleCounter);
  }
  else if (loopTime - lastTriggerTime < (shotInterval + resetTime)) { //KEEP TRIGGER BUTTON DISABLED
    lightRing.fill(0x600010); //purple for disable
  }
  else {
    triggerDisable = 0; //re-enable trigger
  }

  timerDisplay.writeDisplay();
  lightRing.show();

  //SET VALVES - maybe this needs to latched so pin isn't constantly set?
  digitalWrite(CANNON_1, valveState);
  digitalWrite(CANNON_2, valveState);
  digitalWrite(CANNON_3, valveState);
  digitalWrite(CANNON_4, valveState);

}

void lightRingSpin(int spinIndex) {
  int lightStep = spinIndex % LED_COUNT;
  
  for (int i = 0; i < LED_COUNT; i++) {
    if ( (i+lightStep)%LED_COUNT < 8 ) {
      lightRing.setPixelColor(i, 0x000006);
    }
    else {
      lightRing.setPixelColor(i, 0x2000FF);
    }
  }
  lightRing.show();
}

void lightRingBlink() {
  //show initialization colors
  lightRing.fill(0xFF0000);
  lightRing.show();
  delay(200);
  lightRing.fill(0x00FF00);
  lightRing.show();
  delay(200);
  lightRing.fill(0x0000FF);
  lightRing.show();
  delay(200);
}

