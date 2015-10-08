
#ifndef _RPI_GPIO_H_
#define _RPI_GPIO_H_

#include "RPI.h"

extern struct bcm_peripheral gpio;

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) 		*(gpio.addr +  ((g)/10))  &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) 		*(gpio.addr +  ((g)/10))  |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) 	*(gpio.addr + (((g)/10))) |=  (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3)) // Alternate pin function

#define GPIO_SET(g) 		*(gpio.addr + ((g)/32) + GPSET0)  = (1 << (g)) // Set GPIO bit 0..31, 32..53
#define GPIO_CLR(g) 		*(gpio.addr + ((g)/32) + GPCLR0)  = (1 << (g)) // Clear GPIO bit 0..31, 32..53
#define GPIO_READ(g) 		*(gpio.addr + ((g)/32) + GPLEV0) &= (1 << (g)) // Read GPIO input 0..31, 32..53

//Function prototypes
int init_gpio();
void close_gpio();

#endif 
