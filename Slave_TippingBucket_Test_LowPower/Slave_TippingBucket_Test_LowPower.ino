/*****************************************************************************
Salve_TippingBucket_Test.ino
Written by: Bobby Schulz (schu3119@umn.edu)
Updated by: Anthony Aufdenkampe (aaufdenkampe@limno.com)
Development Environment: PlatformIO Core 5.1.1
Hardware Platform: Adafruit Pro Trinket
Software License: BSD-3.
  Copyright (c) 2017, Stroud Water Research Center (SWRC)
  and the EnviroDIY Development Team

This sketch is firmware to be loaded onto a slave micro-controller and used for
counting pulse events, specifically those from a tipping bucket rain gauge

This sketch turns the device into a tip counter which is also an I2C slave,
every time a tip occours, the device counts it, then when the master device
reads from the slave device, it returns the number of tips since the last time
the salve was read in the form of an unsigned int

An example master code can be found at
https://github.com/EnviroDIY/TippingBucketRainCounter/blob/master/Master_Test/Master_Test.ino

Required connections are:
Pin (3 by default) -> Tipping bucket line A
Pin_Low (4 by default) -> Tipping bucket line B
SCL -> Master device SCL
SDA -> Master device SDA
Power -> External power (varies by device)

Wired to Adafruit Pro Trinket as described at:
https://github.com/EnviroDIY/TippingBucketRainCounter/blob/master/README.md

DISCLAIMER:
THIS CODE IS PROVIDED "AS IS" - NO WARRANTY IS GIVEN.
*****************************************************************************/


// ==========================================================================
//  Include required libraries
// ==========================================================================
#include <Arduino.h>    // The Arduino library is needed for every Arduino program.
#include <Wire.h>	    // Include wire library for I2C communication
#include <avr/wdt.h>    // Include library to turn off Watch Doc Timer
#include <avr/power.h>  // Includ power library to shut off internal systems (REMOVE??)
#include <avr/sleep.h>  // Include sleep library to turn off logger


// ==========================================================================
//   Settings
// ==========================================================================
const char*  libVersion = "v0.2.0";

volatile uint32_t NumTips = 0;  // Tip counter used by Interrupt Service Routine (ISR)
uint32_t ReadTips = 0; // Used as output for sample of tip counter to store
  // "old" value, while counter can increment, used to make code reentrant

const uint8_t SerialBufferSize = 4; // Maximum number of bytes
  // = 4 for uint32_t or long
uint8_t SerialBuffer[SerialBufferSize];  // Create a byte array
  // https://www.arduino.cc/reference/en/language/variables/data-types/array/


// Pins on I2C Slave to record interupts from the tipping bucket's reed switch
uint8_t Pin = 3; // Default pin value
uint8_t Pin_Low = 4;  // Just used to drive one side of the tipping bucket
  // reed switch low, a digitial pin is only used to make wiring easier

// The minimum length of pulse which will be counted be the device,
// all shorter pulses are assumed to be noise
uint8_t Debounce = 10;
  // In experimentation, tipping bucket pulses are close to 100ms, so a
  // debounce period of 10ms is sufficiently conservative to never miss a pulse,
  // but was also found to remove all "bounce" errors in testing. If erronious tips
  // seem to be counted (more tips in a period than you observe), you can try
  // increasing this value, but caution is advised, when in doubt, scope the line
  // and/or consult your friendly neighborhood electrical engineer

uint8_t ADR = 0x08; // The desired address of the slave device, can be modified
  // if nessicary to avoid address colision
const uint32_t WAIT_TIME = 500; // The number of dummy cycles which are run when
  // the device wakes up, each cycle is 1ms
volatile uint32_t Counter; //Used to count dummy cycles


// ==========================================================================
//  Working Functions
// ==========================================================================

// Call to update the number of tips from the ISR counter variable
bool Update() {
    ReadTips = NumTips; //Get number of tips
    NumTips = 0;  //Clear the tip counter
    return true; //Return true as a convention for EnviroDIY
}

// Returns the stored value without updating if more than one call of the value
// is required per read cycle
long GetValue() {
    return ReadTips;  //Returns the already updated (old) value, not currently used
}

// Interrupt Service Routine (ISR) for tipping events
void Tip() {
    static uint32_t StartPulse = 0; // Used as variable to measure time between interrupt edges
    // Check if the last edge was more than 1 debounce time period ago, and that the edge measured is rising
    if(((millis() - StartPulse) > Debounce) && digitalRead(Pin)) {
        NumTips++; // If true, increment the tip counter
    }
    StartPulse = millis(); //Keep a record of the last edge time
}

// Interrupt Service Routine (ISR) for I2C requests
void SendTips() {
    Counter = 0;  // Clear counter, this should have happened due to the wake up,
      // but in case a weird sequence of events occurs where that is
      // not the case, clear it again to be safe
    Update(); // Update tip counts

    // Convert uint32_t ReadTips into a byte array
    // https://thewanderingengineer.com/2015/05/06/sending-16-bit-and-32-bit-numbers-with-arduino-i2c/#
    // I2C default is Big-endian (most significant byte is written first)
    // Unfortunately, for v0.1 of this repo, Bobby implemented Little-endian,
    // but he only passed the first, least-signficant byte. See https://github.com/EnviroDIY/TippingBucketRainCounter/issues/5#issuecomment-813134796
    SerialBuffer[0] = (ReadTips >> 24) & 0xFF;
    SerialBuffer[1] = (ReadTips >> 16) & 0xFF;
    SerialBuffer[2] = (ReadTips >> 8) & 0xFF;
    SerialBuffer[3] = ReadTips & 0xFF;

    // Respond with number of tips since last call
    Wire.write(SerialBuffer, 4); // https://www.arduino.cc/en/Reference/WireWrite
}


// ==========================================================================
//  Arduino Setup Function
// ==========================================================================
void setup() {
    // Start the wire library
    Wire.begin(ADR);          // join i2c bus as slave with address #8

    // Register a function to be called when a master requests data from slave
    Wire.onRequest(SendTips); //call SendTips which address is recieved

    pinMode(Pin, INPUT_PULLUP); // Setup pin for tipping bucket using internal pullup
    pinMode(Pin_Low, OUTPUT);  // acts as "ground" for tipping bucket
    // Drive pin adjacent to interrupt pin low
    digitalWrite(Pin_Low, LOW);

    // Setup an interrupt for the tipping bucket pin, with Tip as the ISR,
    // which will activate on every edge
    attachInterrupt(digitalPinToInterrupt(Pin), Tip, CHANGE);
}


// ==========================================================================
//  Arduino Loop Function
// ==========================================================================
void loop() {
    // Dummy loops are run in order to ensure the device does not go back to
    // sleep before the I2C transaction is resolved
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
