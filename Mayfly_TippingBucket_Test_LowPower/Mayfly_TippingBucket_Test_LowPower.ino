#include <Wire.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/sleep.h>

volatile long NumTips = 0; //Tip counter used by ISR
long ReadTips = 0; //Used as output for sample of tip counter to store "old" value, while counter can increment, used to make code reentrant 
int Pin = 3; //Default pin value
int Pin_Low = 4;
int Debounce = 10;
int ADR = 8; //The desired address of the slave device
const unsigned long WAIT_TIME = 500;
volatile unsigned long Counter;

void setup() {
  Wire.begin(ADR);                // join i2c bus as slave with address #8
  Wire.onRequest(SendTips); //call SendTips which address is recieved
  pinMode(Pin, INPUT_PULLUP); //Setup pin for tipping bucket using internal pullup 
  pinMode(Pin_Low, OUTPUT);
  digitalWrite(Pin_Low, LOW); //Drive pin adjacent to interrupt pin low, acts as "ground" for tipping bucket
  attachInterrupt(digitalPinToInterrupt(Pin), Tip, CHANGE); //Setup an interrupt for the tipping bucket pin, with Tip as the ISR, which will activate on every edge
}

void loop() {
   //All handled by the interrupts and I2C hardware, no need for anything in the loop
   if(++Counter >= WAIT_TIME) {
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
    sleep_enable();
    sleep_cpu (); //Waits here while in sleep mode
    
    sleep_disable();
    Counter = 0;
    TWCR = bit(TWEN) | bit(TWIE) | bit(TWEA) | bit(TWINT);
    Wire.begin (ADR);
   }

   delay(1);
}


bool Update() {
  ReadTips = NumTips; //Get number of tips
  NumTips = 0;  //Clear the tip counter
  return true; //Return true as a convention for EnviroDIY
}

long GetValue() {
  return ReadTips;  //Returns the already updated (old) value, not currently used 
}

void Tip() {
  static long StartPulse = 0; //Used as variable to measure time between interrupt edges
  if(((millis() - StartPulse) > Debounce) && digitalRead(Pin)) { //Check if the last edge was more than 1 debounce time period ago, and that the edge measured is rising
    NumTips++; //If true, increment the tip counter
  }
  StartPulse = millis(); //Keep a record of the last edge time
}

void SendTips() {
  Counter = 0;
  Update(); //Update tip counts
  Wire.write(ReadTips); //Respond with number of tips since last call
}


