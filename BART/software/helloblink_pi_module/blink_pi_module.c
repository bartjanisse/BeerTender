/*
 * 19-09-2015 - Bart Janisse
 * 
 * hello-4.c - Demonstrates module documentation.
 * 
 * This example is taken from the book The Linux Kernel Programming Guide.
 * The book is freely downloadable from the internet.
 * 
 * notes:
 * 		- use make to build this module.
 * 		- use modinfo hello-*.ko to see the information in it.
 * 		- use sudo insmod ./hello-1.ko to insert it in the kernel.
 * 		- use sudo rmmod hello-1 to remove it from the kernel.
 * 		- use cat /proc/modules | head to see if it is loaded.
 * 		- use cat /var/log/kern.log | tail to see what is logged.
 */

#include <linux/module.h>
#include <linux/kernel.h> 	// for printk()
#include <linux/init.h>		// for macros
#include <linux/delay.h>	// for msleep()

extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
//extern void dummy ( unsigned int );

#define BCM2708_PERI_BASE       0x3F000000

#define GPFSEL0 0x20200000	// GPFSEL0	- GPIO Function Select 1
//#define GPFSEL1 0x20200004	// GPFSEL1	- GPIO Function Select 1
#define GPSET0  0x2020001C	// GPSET0 	- GPIO Pin Output Set 0
#define GPCLR0  0x20200028	// GPCLR0 	- GPIO Pin Output Clear 0
#define BIT		12    


#define DRIVER_AUTHOR	"Bart Janisse <b.janisse@student.fontys.nl>"
#define DRIVER_DESC		"A sample driver."

unsigned int ra;

static void blink(void)
{
	PUT32(GPSET0,1<<BIT);
	mdelay(500);
	PUT32(GPCLR0,1<<BIT);
	mdelay(500);
}

static void led_on(void)
{
	PUT32(GPSET0,1<<BIT);
}

static void led_off(void)
{
	PUT32(GPCLR0,1<<BIT);
}


static int __init blink_pi_init(void)
{
	printk(KERN_INFO "Hello blink pi.\n");
	
	/*
	 * A non 0 return means init_module failed. Module can't be loaded.
	 */
	
	// Init the output 
	ra = GET32(GPFSEL0);	// Read the current GDFSEL1 register value into ra
	ra &= ~(7<<BIT);		// Make input first. (7 = 111, ~7 = 000) 
    ra |= 1<<BIT;			// Make output
    PUT32(GPFSEL0,ra);		
    
	led_on();
	 
	return (0);
}

static void __exit blink_pi_exit(void)
{
	printk(KERN_INFO "Goodbey blink pi.\n");
	
	led_off();
}

module_init(blink_pi_init);
module_exit(blink_pi_exit);

/*
 * Get rid of the tained message by declaring code as GPL
 */
MODULE_LICENSE("GPL");

MODULE_AUTHOR(DRIVER_AUTHOR);		// Who wrote the module
MODULE_DESCRIPTION(DRIVER_DESC);	// What does this module do?

/*
 * THis module uses /dev/testdevice. The MODULE_SUPPORTED_DEVICE macro might
 * be used in the future to help automatic configuration of modules, but is
 * currently unused other than for documentation purpose.
 */
MODULE_SUPPORTED_DEVICE("testdevice");
