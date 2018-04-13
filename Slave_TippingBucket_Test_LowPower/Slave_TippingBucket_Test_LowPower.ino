/*****************************************************************************
Salve_TippingBucket_Test.ino
Written By:  Bobby Schulz (schu3119@umn.edu)
Development Environment: PlatformIO 3.5.3
Hardware Platform: EnviroDIY Mayfly Arduino Datalogger
Software License: BSD-3.
  Copyright (c) 2017, Stroud Water Research Center (SWRC)
  and the EnviroDIY Development Team

This sketch is firmware to be loaded onto a slave micro-controller and used for counting pulse events, specifically those from a tipping bucket rain gauge

This sketch turns the device into a tip counter which is also an I2C slave, every time a tip occours, the device counts it, then when the master device reads from the slave
device, it returns the number of tips since the last time the salve was read in the form of an unsigned int

an example master code can be found at https://github.com/EnviroDIY/TippingBucketRainGauge under the name "Master_Test.ino"

Required connections are:
Pin (3 by default) -> Tipping bucket line A
Pin_Low (4 by default) -> Tipping bucket line B
SCL -> Master device SCL
SDA -> Master device SDA
Power -> External power (varies by device)

See readme located at https://github.com/EnviroDIY/TippingBucketRainGauge for more detailed wiring documentation and example using the Adafruit Trinket Pro

DISCLAIMER:
THIS CODE IS PROVIDED "AS IS" - NO WARRANTY IS GIVEN.
*****************************************************************************/

#include <Wire.h>	//Include wire library for I2C communication 
#include <avr/wdt.h> //Include library to turn off WDT
#include <avr/power.h> //Includ power library to shut off internal systems (REOMVE??)
#include <avr/sleep.h>  //Include sleep library to turn off logger

volatile long NumTips = 0; //Tip counter used by ISR
long ReadTips = 0; //Used as output for sample of tip counter to store "old" value, while counter can increment, used to make code reentrant 
int Pin = 3; //Default pin value
int Pin_Low = 4;  //Just used to drive one side of the tipping bucket reed switch low, a digitial pin is only used to make wiring easier 
int Debounce = 10;  //The minimum length of pulse which will be counted be the device, all shorter pulses are assumed to be noise
//In experimentation, tipping bucket pulses are close to 100ms, so a debounce period of 10ms is sufficiently conservative to never miss a pulse, but was also found to remove all 
//"bounce" errors in testing. If erronious tips seem to be counted (more tips in a period than you observe), you can try increasing this value, but caution is advised, when in doubt, scope the line
//and/or consult your friendly neighborhood electrical engineer 

int ADR = 8; //The desired address of the slave device, can be modified if nessicary to avoid address colision
const unsigned long WAIT_TIME = 500; //The number of dummy cycles which are run when the device wakes up, each cycle is 1ms
volatile unsigned long Counter; //Used to count dummy cycles

void setup() {
  Wire.begin(ADR);                // join i2c bus as slave with address #8
  Wire.onRequest(SendTips); //call SendTips which address is recieved
  pinMode(Pin, INPUT_PULLUP); //Setup pin for tipping bucket using internal pullup 
  pinMode(Pin_Low, OUTPUT);
  digitalWrite(Pin_Low, LOW); //Drive pin adjacent to interrupt pin low, acts as "ground" for tipping bucket
  attachInterrupt(digitalPinToInterrupt(Pin), Tip, CHANGE); //Setup an interrupt for the tipping bucket pin, with Tip as the ISR, which will activate on every edge
}

void loop() {
   //Dummy loops are run in order to ensure the device does not go back to sleep before the I2C transaction is resolved 
   if(++Counter >= WAIT_TIME) {  //If we are done with dummy counts
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);  //Set deepest sleep mode
    sleep_enable(); //Enable the sleep option
    sleep_cpu (); //Waits here while in sleep mode
    //DEVICE SLEEPING HERE
    sleep_disable();  //First line after wake up
    Counter = 0;  //Clear counter
    TWCR = bit(TWEN) | bit(TWIE) | bit(TWEA) | bit(TWINT);  //Turn on I2C acknowledgement bits
    Wire.begin (ADR); //Restart I2C to ensure propper operation
   }

   delay(1);  //Length of dummy loops
}


bool Update() {  //Call to update the number of tips from the ISR counter variable
  ReadTips = NumTips; //Get number of tips
  NumTips = 0;  //Clear the tip counter
  return true; //Return true as a convention for EnviroDIY
}

long GetValue() {  //Returns the stored value without updating if more than one call of the value is required per read cycle
  return ReadTips;  //Returns the already updated (old) value, not currently used 
}

void Tip() {  //ISR for tipping events
  static long StartPulse = 0; //Used as variable to measure time between interrupt edges
  if(((millis() - StartPulse) > Debounce) && digitalRead(Pin)) { //Check if the last edge was more than 1 debounce time period ago, and that the edge measured is rising
    NumTips++; //If true, increment the tip counter
  }
  StartPulse = millis(); //Keep a record of the last edge time
}

void SendTips() {  //ISR for I2C requests
  Counter = 0;  //Clear counter, this should have happened due to the wake up, but in case a weird sequence of events occours where that is not the case, clear it again to be safe
  Update(); //Update tip counts
  Wire.write(ReadTips); //Respond with number of tips since last call
}


