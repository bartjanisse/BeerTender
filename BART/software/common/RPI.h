
/*
 * Aantekeningen:
 * 	- in /home/student/br-rpi/output/build/linux-2747cc0bde4ca441691260fae1e34c9ef954ccae/arch/arm/mach-bcm2709/include/mach
 *    staat de file platform.h. worden verschillende adressen zoals de BCM2708_PERI_BASE gedefinieerd.
 * 	  Deze is dus te gebruiken met de volgende include: #include <mach/platform.h>
 * 
 */
 // lsmod ... chroot
#ifndef _RPI_REG_
#define _RPI_REG_

/*
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>

#include <sched.h>		// To set the priority on linux

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <unistd.h>
*/
//#include <asm/io.h> // Waar staat deze file ??????
//#include <mach/platform.h>

// See https://www.raspberrypi.org/blog/upcoming-board-revision/
// The new connections are:
//
//    	SCL0 [BCM2835/GPIO1] routed to S5 pin 13
//    	SDA0 [BCM2835/GPIO0] routed to S5 pin 14
//    	SCL1 [BCM2835/GPIO3] routed to P1 pin 5
//    	SDA1 [BCM2835/GPIO2] routed to P1 pin 3
//
// These pins are verified on the board schematic.  
//
// There are three BSC masters inside BCM. The register addresses starts from
//
//		BSC0: 0x7E20_5000
//		BSC1: 0x7E80_4000
//		BSC2: 0x7E80_5000

// The 'BCM2835-ARM-Peripherals.pfd' is used as a reference. Page numbers
// below point tho pages in this document.

// Define which Raspberry Pi board used. Define only one at time.
//#define RPI
#define RPI2

#ifdef RPI
	#define BCM2708_PERI_BASE       0x20000000
	#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller 
	#define BSC0_BASE 				(BCM2708_PERI_BASE + 0x205000) 	// I2C controller
#endif

#ifdef RPI2
	#define BCM2708_PERI_BASE       0x3F000000
	#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller
	#define BSC1_BASE 				(BCM2708_PERI_BASE + 0x804000) 	// 0x804000 I2C controller (See https://www.raspberrypi.org/blog/upcoming-board-revision/)
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

// Constants for alternate GPIO functions. See page 102
#define ALT0		0
#define ALT1		1
#define ALT2		2
#define ALT3		3
#define ALT4		4
#define ALT5		5

#define BLOCK_SIZE 		(4*1024)

// IO Acces
struct bcm_peripheral 
{
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

// Function prototypes
int map_peripheral(struct bcm_peripheral *p);
void unmap_peripheral(struct bcm_peripheral *p);

#endif
