/*
 * 11-10-2015 - Bart Janisse
 */

#ifndef _RPI_H_
#define _RPI_H_

#define BCM2708_PERI_BASE       0x3F000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller 
#define PWM_BASE				(BCM2708_PERI_BASE + 0x20C000) 	// PWM controller
#define CLOCK_BASE				(BCM2708_PERI_BASE + 0x101000)	// Clock base

#define GPSET0 					7	// GPIO Pin Output Set 0
#define GPCLR0 					10	// GPIO Pin Output Clear 0
#define GPLEV0					13	// GPIO Pin Level 0

//----- Clock manager

#define CLK_PASSWD  			(0x5A<<24)	// Clock Manager password

#define	CLK_CTL					40			// Clock Manager General Purpose Clocks Control register
#define CLK_CTL_MASH(x)			((x)<<9)	// MASH control. Valid values are: 0,1,2,3 
#define CLK_CTL_FLIP			(1<<8)		// Invert the clock generator output
#define CLK_CTL_BUSY    		(1<<7)		// Clock generator is running
#define CLK_CTL_KILL    		(1<<5)		// Kill the clock generator
#define CLK_CTL_ENAB    		(1<<4)		// Enable the clock generator
#define CLK_CTL_SRC(x) 			((x)<<0)	// Clock source. 

#define CLK_CTL_SRC_OSC  		1			// Oscillator 19.2 Mhz
#define CLK_CTL_SRC_PLLA 		4			// PLL 1000 MHz
#define CLK_CTL_SRC_PLLC 		5			// PLL 500 MHz
#define CLK_CTL_SRC_PLLD 		6			// PLL 216 MHz

#define	CLK_DIV					41  		// Clock Manager General Purpose Clocks Divisors
#define CLK_DIV_DIVI(x) 		((x)<<12)	// Integer part of divisor
#define CLK_DIV_DIVF(x) 		((x)<< 0)	// Fractional part of divisor

//----- PWM

#define PWM_CTL      			0			// PWM Control register
#define PWM_STA      			1			// PWM Status register
#define PWM_DMAC     			2			// PWM DMA configuration register
#define PWM_RNG1     			4			// PWM Channel 1 Range register
#define PWM_DAT1     			5			// PWM Channel 1 Data register
#define PWM_FIFO     			6			// PWM FIFO Input register
#define PWM_RNG2     			8			// PWM Channel 2 Range register
#define PWM_DAT2     			9			// PWM Channel 2 Data register

#define PWM_CTL_MSEN2 			(1<<15)		// Channel 2 M/S Enable
#define PWM_CTL_USEF2 			(1<<13)		// Channel 2 Use Fifo
#define PPM_CTL_POLA2			(1<<12)		// Channel 2 Polarity
#define PWM_CTL_SBIT2			(1<<11)		// Channel 2 Silence Bit
#define PWM_CTL_RPTL2			(1<<10)		// Channel 2 Repeat Last Data
#define PWM_CTL_MODE2 			(1<<9)		// Channel 2 Mode
#define PWM_CTL_PWEN2 			(1<<8)		// Channel 2 Enable
#define PWM_CTL_MSEN1 			(1<<7)		// Channel 1 M/S Enable
#define PWM_CTL_CLRF1 			(1<<6)		// Clear Fifo
#define PWM_CTL_USEF1 			(1<<5)		// Channel 1 Use Fifo
#define PPM_CTL_POLA1			(1<<4)		// Channel 1 Polarity
#define PWM_CTL_SBIT1			(1<<3)		// Channel 1 Silence Bit
#define PWM_CTL_RPTL1			(1<<2)		// Channel 1 Repeat Last Data
#define PWM_CTL_MODE1 			(1<<1)		// Channel 1 Mode
#define PWM_CTL_PWEN1 			(1<<0)		// Channel 1 Enable
  
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x)
#define INP_GPIO(g)   			*(gpio + ((g)/10))   &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g)   			*(gpio + ((g)/10))   |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) 		*(gpio + (((g)/10))) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3))
 
#define GPIO_SET(g) 			*(gpio + ((g)/32) + GPSET0)  = (1 << (g)) // Set GPIO bit 0..31, 32..53
#define GPIO_CLR(g) 			*(gpio + ((g)/32) + GPCLR0)  = (1 << (g)) // Clear GPIO bit 0..31, 32..53
#define GPIO_READ(g) 			*(gpio + ((g)/32) + GPLEV0) &= (1 << (g)) // Read GPIO input 0..31, 32..53
 
volatile unsigned int *gpio;
volatile unsigned int *pwm;
volatile unsigned int *clk;
 
#endif
