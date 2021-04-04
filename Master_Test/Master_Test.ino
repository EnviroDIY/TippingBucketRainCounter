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
    // Start the wire library (sensor power not required)
    Wire.begin();        // join i2c bus (address optional for master)
    // Eliminate any potential extra waits in the wire library
    // These waits would be caused by a readBytes or parseX being called
    // on wire after the Wire buffer has emptied.  The default stream
    // functions - used by wire - wait a timeout period after reading the
    // end of the buffer to see if an interrupt puts something into the
    // buffer.  In the case of the Wire library, that will never happen and
    // the timeout period is a useless delay.
    Wire.setTimeout(0);
      //  As done in https://github.com/EnviroDIY/ModularSensors/blob/97bf70902010272d2e826f8a99d64f870368208e/src/sensors/RainCounterI2C.cpp#L84-L91

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

    Wire.requestFrom(ADR, 2, false);    // request 2 bytes from slave device #8
    // added `false` to Wire.requestFrom as suggested by Bobby in:
    // https://github.com/EnviroDIY/TippingBucketRainCounter/issues/5#issuecomment-521408309
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
