/*
 * (C) 2015 - A.W. Janisse
 * 
 * hardware.h - Header file for the functions for GPIO, CLK and PWM hardware control.
 * 
 * xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 * 
 */
 
#ifndef _PWM_HW_
#define _PWM_HW_

#include "pwm.h"
#include "RPI.h"

#define PWM_0_GPIO		12	// Since GPIO 12 is not connected on the Raspberry Pi pcb, use it as pwm 
#define PWM_1_GPIO		13	// Since GPIO 13 is not connected on the Raspberry Pi pcb, use it as pwm
#define GPIO_ALTFUN		0	// Sine we are using GPIO 12 and 13 we need alternate function 0
 
extern void registers_map(void);
extern void registers_unmap(void);

extern void pwm_init_gpio(void); 

extern void pwm_start(struct PWM);
extern void pwm_stop(struct PWM);

extern void clk_set(struct CLOCK);
 
 #endif
