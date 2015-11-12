## Hardware PWM test results
The BCM_2835 manual has a table on page 140 which specifies the GPIO assignment to PWM channels. Since most of us know by now
we cannot fully trust that this information is also applicable for the BCM2836 (Pi2) i have tested all posibilities from
this table to see if the pwm is working or not. For these tests two modules pwm_1 and pwm_2 were used and can be found above. 

The table below shows the test results.

| GPIO    | PWM0      | PWM1      |  Working Y/N |
| :------ |:---------:|:----------|:------------:|
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
Both pwm's are driven by an internal clock which is nominally running at 100Mhz. This 
clock can be varied by the clock manager. The register definitions for this clock manager can be found on
page 107 of the BCM_2835 manual.

For the BCM2836 the base offeset address for the Clock Manager is 0x101000.  
