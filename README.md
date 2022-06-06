# TippingBucketRainCounter I2C Slave Device
Arduino code & instructions to convert an [Adafruit Pro Trinket 3V board](https://www.adafruit.com/product/2010) into a tip counter for any reed switch sensor, such as a tipping bucket rain gauge or an analog anemometer for wind speed. The programmed Pro Trinket will continuously count tip events, and will respond as an I2C Slave to a Master data logger board, such as the [EnviroDIY Mayfly logger](https://github.com/EnviroDIY/EnviroDIY_Mayfly_Logger).

The TippingBucketRainCounter I2C Slave Device has been used with these sensors:
- [Davis Vantange Pro2 Tipping Bucket](https://www.davisinstruments.com/product/rain-collector-base-with-tipping-bucket-for-vantage-pro-pro2/) and [AeroCone Rain Collector](https://www.davisinstruments.com/product/aerocone-rain-collector-cone-replacement-kit/)
- [Inspeed Reed Switch Anemometer](https://www.store.inspeed.com/Inspeed-Version-II-Reed-Switch-Anemometer-Sensor-Only-WS2R.htm)


## Programming Pro Trinket to Count Events and Serve as I2C Slave

Tips on how to program Adafruit Trinket Pro:
* Add the Adafruit Board Support package! https://cdn-learn.adafruit.com/downloads/pdf/adafruit-arduino-ide-setup.pdf
* Follow this guide carefully: https://learn.adafruit.com/introducing-pro-trinket/setting-up-arduino-ide
  * Note that you will not see the Pro Trinket show up as a COM port, but it can still accept programs over USB when in Bootloader mode (when the red LED is pulsing).
  * If the red LED is not pulsing, press the button on the Pro Trinket, pause for it to start pulsing, then upload the program.

Load this file the Pro Trinket, which serves as an I2C Slave for this application:
* `Slave_TippingBucket_Test_LowPower.ino`

To test, load the `Master_Test.ino` file to test on your data logging board, such as the EnviroDIY Mayfly, which will serve as an I2C Master, then:
- Turn on the serial montior for your data logger, and it should start printing output.
- If the Pro Trinket "slave" is connected and programmed properly, then it will print “Tips since last read = ", followed by a count value, every 4 seconds.
  - If it is all wired up to the tipping bucket (or anemometer), and you manually tip or spin it, then you will the count output as an integer.
- If the Pro Trinket “slave” is not connected to I2C with the right address, then you will get a “Sensor not connected” output.

## Wiring Guide
The wiring of the slave to the master is shown in the following graphic, where 3v3, GND, SDA, and SCL come from the master device (I2C), and TipA and TipB are the two lines connecting to the tipping bucket rain gauge (these lines are generally interchangeable).

Note that the Pro Trinket needs to always have power available, so if connecting to the EnviroDIY Mayfly I2C grove port, switch the jumper to non-switched power.

![Trinket Pro Slave Wiring Diagram](doc/TrinketProPinout.png)

The Davis tipping bucket comes with a relatively short telephone cable (RJ11) with four wires. The Pro Trinket only needs to connect to two of these cables. For example: use just the red and green cables and cut the black and white/yellow. It doesn't matter which one (red or green) is connected to TipA or TipB, they simply need to be opposite each other in pins 3 and 4 on the Pro Trinket.    

**Tipping Bucket 		        -> Pro Trinket**  
Black/Black: Gnd 		      -> (not connected)  
Red/Red (TipA): 		      -> D3 (or D4, just opposite of TipB)  
Green/Green (TipB)	      -> D4 (or D3, just opposite of TipA)  
Yellow/White (alt TipB)	  -> D4 (or D3, just opposite of TipA)  

## Reducing Power
The power consumption of the Pro Trinket can be greatly reduced by cutting two connections on it's printed circuit board.

**Power Reduction A (recommended)** is to disconnect the green "Alive" LED. Cutting trace A only disconnects the green alive LED and has no impact on functionality, other than not having the green LED turn on when power is applied.

**Power Reduction B (pros only)** is to disconnect the on board 3v3 regulator from the 3v3 line in order to reduce reverse current leakage. **Do this with caution**, as cutting trace B disconnects the regulator from the USB input. This means that once this trace is cut external power will be required to program the device. This modification is only recommended if extreme power reduction is required! The results of this are not nearly as significant as disconnecting the alive LED (As can be seen in the current table below), and can make things more difficult to work with.

Both of these modifications are made by cutting a trace on the PCB using an XActo knife or metal scribe. Disconnecting of the alive LED is done by cutting trace "A" in the figure below, and disconnecting of the regulator is done by cutting trace "B". With either option proceed with caution and of course do not take a knife to anything you are not willing to sacrifice!

The current consumptions for all of these configurations is shown in the table below.

![Power Reduction Trace Callout](doc/ProTrinketTraceCutGuide.jpg)

### Power Consumption in Sleep

The vast majority of the time the slave is in sleep mode, so this is the current consumption we primarily care about, the following table show the current consumption of a Trinket Pro running the low power slave code with various hardware modifications, all tests performed with an input votage of 3.3v

Moddification | Current [mA]
------------ | -------------
None | 2.9
Cut Trace A | 0.1
Cut Trace A & B | ~0.05

As you can see, disconnecting the alive LED has a very significant impact on the current consumption, the disconnecting of the regulator does reduce current further, but not nearly so dramatically and it does make working with the device more difficult, so for most applications it is probobly not worth it.
