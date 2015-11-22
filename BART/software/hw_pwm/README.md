## PWM modes
The PWM controller consists of two independent channels implementing the pwm algorithm. Each channel can operate in either pwm
mode or serialiser mode. In PWM mode there are two sub-modes. All three modes are explained below.

Configuring can be done by setting the relevant bits in the PWM control register. The table below shows how this
can be done.

|               | MODEi | MSENi |
|:--------------|:-----:|:-----:|
| Serialiser    |   1   |   x   |
| Dutycycle M/N |   0   |   0   |
| Dutycycle M/S |   0   |   1   |


### PWM serialiser mode ====> ja
In this mode the PWM always uses PWM_DATi or FIFO to serial sends its data.  

???????????????????????????????????????????????
???????????????????????????????????????????????
???????????????????????????????????????????????
???????????????????????????????????????????????


### PWM dutycycle M/N mode


    +-------+                     +---
    |       |                     |
 ---+       +---------------------+
    |<- M ->|<-------- N -------->|


### PWM dutycycle M/S mode ====> ja


    +-------+                     +---
    |       |                     |
 ---+       +---------------------+
    |<- M ->|
    |<------------ S ------------>|
    
    



## Controlling servo's
A hobby servo motor works with variable pulses at a fixed cycle of 20ms. If the puls length
is varried, the angle of the servo motor shaft varries accordingly. Common for these type of
servos is that the puls length can be varried from 1ms to 2 ms. When pulses of 1ms are 
given the servo shaft will be positioned at 0 degrees and when pulses of 2ms are given the servo 
shaft will be positioned to a 180 degrees. See te diagram below for clarification.

```
    +-------+                     +---
    |       |                     |
 ---+       +---------------------+
    |<- M ->|
    |<------------ S ------------>|
 
S =  20ms (fixed)
M = 2.0ms => 180 degrees
M = 1.5ms =>  90 degrees
M = 1.0ms =>   0 degrees
```
###Setting up the PWM
The dutycycle for the PWM is managed the registers PWM_RNGi and PWM_DATi. PWM_RNGi is responsible
for the complete cycle time S and PWM_DATi is responsible for the pulstime M. The dutycycle ca
be easily calculated with:
```
          PWM_DATi
	dc = ---------- * 100%
          PWM_RNGi
```          
The next thing we need to calculate is the frequency of the pulses which are needed by the PWM. 
This can be done with the following formula:
```
                              PWM cycle time (S)
	puls frequency =  1 / ( --------------------- )
                                  PWM_RNGi 
```
### Setting up the clock
An internal clock which has a base frequency of 19.2MHz delivers the pulses needed by the PWM's. So to get 
the right PWM puls frequency we need to scale down this clock which can be done bij setting the correct value
in DIVI register of the Clock Manager. Calculating this value can be done with this formula:
```
			base frequency
	DIVI = ----------------
	        puls frequency
```

## Hardware PWM test results
The BCM_2835 manual has a table on page 140 which specifies the GPIO assignment to PWM channels. Since most of us know by now
we cannot fully trust that this information is also applicable for the BCM2836 (Pi2) i have tested all posibilities from
this table to see if the pwm is working or not. For these tests two modules pwm_1 and pwm_2 were used and can be found above. 

The table below shows the test results.

| GPIO    | PWM0      | PWM1      |  Working Y/N |
|:--------|:----------|:----------|:------------:|
| GPIO 12 | Alt fun 0 | -         | **Y**        |
| GPIO 13 | -         | Alt fun 0 | **Y**        |
| GPIO 18 | Alt fun 5 | -         | **Y**        |
| GPIO 19 | -         | Alt fun 5 | **Y**        |
| GPIO 40 | Alt fun 0 | -         | **N**        |
| GPIO 41 | -         | Alt fun 0 | **N**        |
| GPIO 45 | -         | Alt fun 0 | **N**        |
| GPIO 52 | Alt fun 1 | -         | **N**        |
| GPIO 53 | -         | Alt fun 1 | **N**        |
