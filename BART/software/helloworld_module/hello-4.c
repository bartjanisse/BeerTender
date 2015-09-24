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

#define DRIVER_AUTHOR	"Bart Janisse <b.janisse@student.fontys.nl>"
#define DRIVER_DESC		"A sample driver."

static int __init hello_4_init(void)
{
	printk(KERN_INFO "Hello world 4.\n");
	
	/*
	 * A non 0 return means init_module failed. Module can't be loaded.
	 */
	 
	return (0);
}

static void __exit hello_4_exit(void)
{
	printk(KERN_INFO "Goodbey world 4.\n");
}

module_init(hello_4_init);
module_exit(hello_4_exit);

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
