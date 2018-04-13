# TippingBucketRainGauge
An Arduino library to record reed switch "tips" from a tipping bucket rain gauge.

Uses Adafruit Trinket Pro to monitor tips and act as an I2C slave.

Program Adafruit Trinket Pro:
* Add the Adafruit Board Support package! https://cdn-learn.adafruit.com/downloads/pdf/adafruit-arduino-ide-setup.pdf
* https://learn.adafruit.com/introducing-pro-trinket/setting-up-arduino-ide


## Wiring Guide
The wiring of the slave to the master is shown in the following graphic, where 3v3, GND, SDA, and SCL come from the master device, and TipA and TipB are the two lines connecting to the tipping bucket rain gauge (these lines are generally interchaingable)

![Trinket Pro Slave Wiring Diagram](doc/TrinketProPinout.png)

## Reducing Power
The power consumption of the Trinket Pro can be greatly reduced by disconnecting the "Alive" LED. The power consumption can be further reduced be disconnecting the on board 3v3 regulator from the 3v3 line in order to reduce reverse current leakage. Both of these modifications are made by cutting a trace on the PCB using an XActo knife or metal scribe. Disconnecting of the alive LED is done by cutting trace "A" in the figure below, and disconnecting of the regulator is done by cutting trace "B". The current consumptions for all of these configurations is shown in the table below as well.

**NOTE:** Cutting trace A only disconnects the alive LED and has no impact on functionality, other than not having the LED turn on when power is applied, however, cutting trace B disconnects the regulator from the USB input, this means that once this trace is cut external powe will be required to program the device, this modification is only recomended if extreme power reduction is required! The results of this are not nearly as significant as disconnecting the alive LED (As can be seen in the current table), and can make things more difficult to work with. With either option proceed with caution and of course do not take a knife to anything you are not willing to sacrifice! 

![Power Reduction Trace Callout](doc/ProTrinketTraceCutGuide.jpg)

### Power Consumption in Sleep

The vast majority of the time the slave is in sleep mode, so this is the current consumption we primarily care about, the following table show the current consumption of a Trinket Pro running the low power slave code with various hardware modifications, all tests performed with an input votage of 3.3v

Moddification | Current [mA]
------------ | -------------
None | 2.9
Cut Trace A | 0.1
Cut Trace A & B | ~0.05

As you can see, disconnecting the alive LED has a very significant impact on the current consumption, the disconnecting of the regulator does reduce current further, but not nearly so dramatically and it does make working with the device more difficult, so for most applications it is probobly not worth it. 