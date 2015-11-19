## PWM modes
The PWM controller consists of two independent channels implementing the pwm algorithm. Each channel can operate in either pwm
mode or serialiser mode. In PWM mode there are two sub-modes. All three modes are explained below.

Configuring can be done by setting the relevant bits in the PWM control register. The table below shows how this
must be done.

|               | MODEi | MSENi | USEFi |
|:--------------|:-----:|:-----:|:-----:|
| Serialiser    |   1   |   x   |   x   |
| Dutycycle M/N |   0   |   0   |  1/0  |
| Dutycycle M/S |   0   |   1   |  1/0  |


### PWM serialiser mode
In this mode the PWM always uses the FIFO to sends its data.  

???????????????????????????????????????????????
???????????????????????????????????????????????
???????????????????????????????????????????????
???????????????????????????????????????????????


### PWM dutycycle M/N mode


    +-------+                     +---
    |       |                     |
 ---+       +---------------------+
    |<- M ->|<-------- N -------->|


### PWM dutycycle M/S mode


    +-------+                     +---
    |       |                     |
 ---+       +---------------------+
    |<- M ->|
    |<------------ S ------------>|
    
    

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


## Clock divider
The PWM's are driven by a clock. The base frequency for this clock is 19.2MHz and 
can be varied. Variation is done by setting the values for a clock divider.






A servo's normaly works with pulses M varying from 1.0ms to 2.0ms for angles
from 0 to 180 degrees. The cycle S should be constant at 20ms.

    +-------+                     +---
    |       |                     |
 ---+       +---------------------+
    |<- M ->|
    |<------------ S ------------>|
 
S =  20ms 
M = 2.0ms => 180 degrees (DC = 10%)
M = 1.5ms =>  90 degrees (DC = 7.5%)
M = 1.0ms =>   0 degrees (DC = 5%)
