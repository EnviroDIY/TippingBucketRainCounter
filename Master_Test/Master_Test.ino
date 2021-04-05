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

uint32_t tips = 0; // Used to measure the number of tips

const uint8_t SerialBufferSize = 4; // Maximum number of bytes
  // = 4 for uint32_t or long
uint8_t SerialBuffer[SerialBufferSize];  // Create a byte array
  // https://www.arduino.cc/reference/en/language/variables/data-types/array/


// ==========================================================================
//  Working Functions
// ==========================================================================

// http://projectsfromtech.blogspot.com/2013/09/combine-2-bytes-into-int-on-arduino.html
// uint16_t BitShiftCombine(uint8_t x_high, uint8_t x_low) {
//     uint16_t combined;
//     combined = x_high;         //send x_high to rightmost 8 bits
//     combined = combined<<8;    //shift x_high over to leftmost 8 bits
//     combined |= x_low;         //logical OR keeps x_high intact in combined and fills in                                                             //rightmost 8 bits
//     return combined;
// }

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

    tips = 0; // reset tips

    // Bytes to read then concatenate
    // uint8_t Byte0, Byte1, Byte2, Byte3;

    Wire.requestFrom(ADR, SerialBufferSize); // request byte array from slave device #8
    // added `false` to Wire.requestFrom as suggested by Bobby in:
    // https://github.com/EnviroDIY/TippingBucketRainCounter/issues/5#issuecomment-521408309

    // from https://forum.arduino.cc/index.php?topic=584872.0
    if (Wire.available()) {
        Serial.println("\nReading tips...");
        uint8_t chars_in = 0; // Start iterator for reading Bytes
        while (Wire.available()) { // slave may send less than requested
            // Byte1 = Wire.read();  //Read number of tips back
            // Byte2 = Wire.read();
            SerialBuffer[chars_in] = Wire.read();

            // Byte output for Debugging
            Serial.print("  SerialBuffer[");
            Serial.print(chars_in);
            Serial.print("] = ");
            Serial.println(SerialBuffer[chars_in]);

            chars_in++;  // increment by 1

            // alternate approach uses Serial.parseInt, https://arduinogetstarted.com/reference/serial-parseint
            // myInt = Serial.parseInt(SKIP_ALL, '\n');
        }
        SerialBuffer[chars_in + 1] = '\0';  // assigns to null character.

        // Concatenate bytes into uint32_t by bit-shifting
        // https://thewanderingengineer.com/2015/05/06/sending-16-bit-and-32-bit-numbers-with-arduino-i2c/#
        uint8_t SerialBufferLength = sizeof(SerialBuffer);
        Serial.println(SerialBufferLength);
        tips = SerialBuffer[0];
        tips = (tips << 8) | SerialBuffer[1];
        tips = (tips << 8) | SerialBuffer[2];
        tips = (tips << 8) | SerialBuffer[3];

        Serial.print("  Tips since last read = ");
        Serial.println(tips);  //Prints out tips to monitor

    } else {
        Serial.println("\nSensor not connected");  //
    }


    delay(UpdateRate); //Waits for next period
}
