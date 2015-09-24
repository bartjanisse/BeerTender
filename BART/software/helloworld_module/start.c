/*
 * 19-09-2015 - Bart Janisse
 * 
 * start.c - Illustration of multifile modules.
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
#include <linux/kernel.h> // for printk()

int init_module(void)
{
	printk(KERN_INFO "Hello world, this is the kernel speaking.\n");
	
	/*
	 * A non 0 return means init_module failed. Module can't be loaded.
	 */
	 
	return (0);
}
