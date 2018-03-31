#include <Wire.h>
#include <avr/wdt.h>
#include <avr/power.h>

volatile long NumTips = 0; //Tip counter used by ISR
long ReadTips = 0; //Used as output for sample of tip counter to store "old" value, while counter can increment, used to make code reentrant 
int Pin = 2; //Default pin value
int Debounce = 10;

void setup() {
  Wire.begin(8);                // join i2c bus as slave with address #8
  Wire.onRequest(SendTips); //call SendTips which address is recieved

//  ADCSRA = 0;  // disable ADC
  wdt_disable(); //Turn off the watchdog timer
  MCUCR = bit (BODS) | bit (BODSE);
  MCUCR = bit (BODS);
  power_adc_disable(); // ADC converter
  power_spi_disable(); // SPI
  power_usart0_disable();// Serial (USART) 
  power_timer1_disable();// Timer 1
  power_timer2_disable();// Timer 2
  pinMode(Pin, INPUT_PULLUP); //Setup pin for tipping bucket using internal pullup 
  attachInterrupt(digitalPinToInterrupt(Pin), Tip, CHANGE); //Setup an interrupt for the tipping bucket pin, with Tip as the ISR, which will activate on every edge
}

void loop() {
   //All handled by the interrupts and I2C hardware, no need for anything in the loop
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
  Update(); //Update tip counts
  Wire.write(ReadTips); //Respond with number of tips since last call
}
