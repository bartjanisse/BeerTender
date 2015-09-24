
/*
 * Aantekeningen:
 * 	- in /home/student/br-rpi/output/build/linux-2747cc0bde4ca441691260fae1e34c9ef954ccae/arch/arm/mach-bcm2709/include/mach
 *    staat de file platform.h. worden verschillende adressen zoals de BCM2708_PERI_BASE gedefinieerd.
 * 	  Deze is dus te gebruiken met de volgende include: #include <mach/platform.h>
 * 
 */
 
#ifndef _RPI_REG_
#define _RPI_REG_


//#include <asm/io.h> // Waar staat deze file ??????
//#include <mach/platform.h>

// The 'BCM2835-ARM-Peripherals.pfd' is used as a reference. Page numbers
// below point tho pages in this document.

// Define which Raspberry Pi board used. Define only one at time.
//#define RPI
#define RPI2

#ifdef RPI
	#define BCM2708_PERI_BASE       0x20000000
	#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller 
#endif

#ifdef RPI2
	#define BCM2708_PERI_BASE       0x3F000000
	#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller
#endif

// Since we are using pointers, registers are defined as an offset. See page 90 
#define	GPFSEL0		0	// GPIO Function Select 0
#define	GPFSEL1		1	// GPIO Function Select 1
#define	GPFSEL2		2	// GPIO Function Select 2
#define	GPFSEL3		3	// GPIO Function Select 3
#define	GPFSEL4		4	// GPIO Function Select 4
#define GPSET0 		7	// GPIO Pin Output Set 0
#define GPSET1 		8	// GPIO Pin Output Set 1
#define GPCLR0 		10	// GPIO Pin Output Clear 0
#define GPCLR1 		11	// GPIO Pin Output Clear 1
#define GPLEV0		13	// GPIO Pin Level 0
#define GPLEV1		14	// GPIO Pin Level 1
#define	GPEDS0 		16	// GPIO Pin Event Detect Status 0
#define	GPEDS1 		17	// GPIO Pin Event Detect Status 1
#define GPREN0 		19	// GPIO Pin Rising Edge Detect Enable 0
#define GPREN1 		20	// GPIO Pin Rising Edge Detect Enable 1
#define GPFEN0 		22	// GPIO Pin Falling Edge Detect Enable 0
#define GPFEN1 		23	// GPIO Pin Falling Edge Detect Enable 1

#endif
