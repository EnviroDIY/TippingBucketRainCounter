/*****************************************************************************
Master_Test.ino
Written by: Bobby Schulz (schu3119@umn.edu)
Updated by: Anthony Aufdenkampe (aaufdenkampe@limno.com)
Development Environment: PlatformIO Core 5.1.1
Hardware Platform: EnviroDIY Mayfly Arduino Datalogger
Software License: BSD-3.
  Copyright (c) 2017, Stroud Water Research Center (SWRC)
  and the EnviroDIY Development Team

Based on Wire Reader Master by Nicholas Zambetti <http://www.zambetti.com>

This sketch is designed to be loaded onto the master device in the system and reads from the slave tipping bucket counter which is loaded with the Salve_TippingBucket_Test.ino
firmware. This program simply reads the number of tips every 2 seconds, then prints this value to the serial monitor.

Connections to the salve device should be:
SCL -> Slave device SCL
SDA -> Slave device SDA

DISCLAIMER:
THIS CODE IS PROVIDED "AS IS" - NO WARRANTY IS GIVEN.
*****************************************************************************/


// ==========================================================================
//  Include required libraries
// ==========================================================================
#include <Arduino.h>
#include <Wire.h>


// ==========================================================================
//   Settings
// ==========================================================================
int32_t serialBaud = 115200;  // Baud rate for serial monitor debugging

uint32_t UpdateRate = 4000; // Milliseconds between measurement updates
uint8_t ADR = 0x08; // Address of slave device, 0x08 by default


// ==========================================================================
//  Arduino Setup Function
// ==========================================================================
void setup() {
    Wire.begin();        // join i2c bus (address optional for master)
    Serial.begin(serialBaud);  // start serial for output
    Serial.print("\nTippingBucketRainGauge Master_Test.ino sketch.\n\n"); //Generic begin statment for monitor
}

// ==========================================================================
//  Arduino Loop Function
// ==========================================================================
void loop() {
    uint16_t tips = 0; //Used to measure the number of tips
    uint8_t Byte1 = 0; //Bytes to read then concatenate
    uint8_t Byte2 = 0;

    Wire.requestFrom(ADR, 2);    // request 2 bytes from slave device #8
    Byte1 = Wire.read();  //Read number of tips back
    Byte2 = Wire.read();

    tips = ((Byte2 << 8) | Byte1); //Concatenate bytes

    Serial.print("Number of Tips since last read = ");

    if (tips == 65535) {
        Serial.println("Sensor not connected");  // tips == 65535 if I2C isn't connectected
    } else {
        Serial.println(tips);  //Prints out tips to monitor
    }

    delay(UpdateRate); //Waits for next period
}
