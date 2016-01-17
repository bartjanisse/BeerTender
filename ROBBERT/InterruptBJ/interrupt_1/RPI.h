/*
 * 11-10-2015 - Bart Janisse
 */

#ifndef _RPI_H_
#define _RPI_H_

#define BCM2708_PERI_BASE       0x3F000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller 

#define GPSET0 					7	// GPIO Pin Output Set 0
#define GPCLR0 					10	// GPIO Pin Output Clear 0
#define GPLEV0					13	// GPIO Pin Level 0
  
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x)
#define INP_GPIO(g)   			*(gpio + ((g)/10))   &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g)   			*(gpio + ((g)/10))   |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) 		*(gpio + (((g)/10))) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3))
 
#define GPIO_SET(g) 			*(gpio + ((g)/32) + GPSET0)  = (1 << (g)) // Set GPIO bit 0..31, 32..53
#define GPIO_CLR(g) 			*(gpio + ((g)/32) + GPCLR0)  = (1 << (g)) // Clear GPIO bit 0..31, 32..53
#define GPIO_READ(g) 			*(gpio + ((g)/32) + GPLEV0) &= (1 << (g)) // Read GPIO input 0..31, 32..53
 
volatile unsigned int *gpio;
 
#endif
