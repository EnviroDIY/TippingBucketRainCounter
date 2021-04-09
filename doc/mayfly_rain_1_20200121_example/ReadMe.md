# Tipping Bucket Sketch for the Mayfly

### From 2020-11-03 Post by Shannon Hicks at:
https://www.envirodiy.org/topic/rain-gauge-tip-data-without-rain/#post-14783

> Below is the code I wrote several years ago that is still running on all our local tipping buckets.  This was way before all the ModularSensors libraries and MMW portal, so things could be written differently now, but it can give you and idea on how the Mayfly handles the interrupt signal from the tipping bucket.  There’s a 1-second delay after a tip is sensed to filter out any switch bouncing that sometimes occurs when a tip happens.
> 
> First you need to enable the pullup resistor on pin D10, which is already built in to the Mayfly board, you just have to close jumper SJ12 on the back of the Mayfly.  Then this snippet of the code shows how to set up your sketch to look for a low trigger from the tipping bucket switch:

```cpp
//enable the D10 pullup jumper on Mayfly solder jumper SJ12 in order for the next 2 lines to work
pinMode(10, INPUT); //sets pin 10 as an input
attachInterrupt(2, pin10interrupt, LOW); //sets up an interrupt looking for a LOW trigger
//connect one terminal of tipping bucket to Mayfly pin D10, connect other side of bucket switch to Mayfly ground
```

> I’ve sent the full sketch code to various people in the past few years who have requested it, and I thought it was already posted somewhere here on the forum, but if not, here’s the entire sketch.  The biggest weak point is probably the delays that surround the Xbee radio wakeup and sleep, totaling 5 seconds.  If a bucket tip were to happen during these delays, they don’t always get counted.  A better option would be to use a milli timer to let the Mayfly go back to looking for tips and then execute the radio on and off functions independently, without using delays.  But missing 5 seconds out of a 5 minute period isn’t too bad.  (We’ve got Hobo event loggers on these rain gauges to record the actual data, these Mayfly boards are just there to transmit the live tips to our in-house data viz page for realtime viewing.)  If the Mayfly has to do even more time-consuming things like cellular transmissions, then having the secondary device like the Trinket would ensure that no tips are missed.  There are certainly more efficient ways to build a simple counter, but cost-wise, it’s hard to beat a $7 Trinket.  If someone simply wants to count tips of a rain gauge (or any switch close/open event), then this code minus the telemetry stuff works great as an event counter/logger.

### From 2020-11-03 Post by Shannon Hicks at:
https://www.envirodiy.org/topic/rain-gauge-tip-data-without-rain/#post-14785

> After thinking about this old code yesterday, I was curious about what actually happens if a tip occurs during a long delay period, like the 2-second delay on line 123 and and 2.5-second delay on line 125 (which are delays to give the Xbee 900mhz radio module time to successfully wake up and then finish its routine before sleeping).   I programmed a test board with the sketch from above, but increased the delay time to 10 seconds for each of those lines, to give me enough time to experiment.  What I found was that if a simulated bucket tip on pin D10 happens anytime during a 10-second delay, the delay instantly ends and the interrupt routine successfully gets executed (meaning a tip gets counted) and then the sketch moves on with the next line of code (either line 124 or 126, depending on which delay the tip happened in).  So this shows that no tips will get missed by the Mayfly, but the delay period gets instantly cut short.  If you’ve got a radio or cell module that needs multiple seconds of uninterrupted wait times before doing the next step, then care should be taken about how you handle interrupts from D10 during that time.  You could unattach the D10 interrupt during those important delay times, and then reattach it when you’re done, or find another way to handle either the interrupt or the telemetry delays.

### Shannon also shared separately:

> The sketch I currently use on all my rain gages is 5 years old and uses libraries that have been renamed or replaced and aren't available on our Github repo, so you won't be able to easily replicate the sketch. But I can send you the code and you can see if the functions are helpful. It essentially just has an integer named "dailytips" that gets incremented by 1 every time there's a tip. At midnight, it resets the integer back to 0. Would you be transmitting the data via xbee or cellular, or just recording to a memory card? Because I have examples of all 3.
> 
> My code only handles 2G cellular, because in order for the 4G LTE boards to work happily with our boards, Sara Damiano took over the sketch writing a couple years ago and rewrote all the libraries to work together, which is why I don't think my old sketch is going to work properly without being rewritten and readjusted for the new libraries. But in any case, the sketch is attached so you can see the basic structure and get some ideas on how it works.
> 
> The hardest part about counting tips with a sleeping logger, is that you have 2 separate interrupts that can wake the Mayfly. One is the clock, which happens every minute. The other is when there's a tip detected on Pin 10. You do that by attaching an interrupt to those 2 inputs, and then clearing the interrupt when it happens so that the board can go back to sleep. It took me a long time of experimenting to get them to work together without causing conflicts, or getting ignored. Resetting at midnight was the easy part.
