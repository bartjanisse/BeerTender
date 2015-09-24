/*
 * 19-09-2015 - Bart Janisse
 * 
 * hello-2.c - Demonstrating the module_init() and module_exit() macros.
 * This is preferred over using init_module() and cleanup_module().
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

static int __init hello_2_init(void)
{
	printk(KERN_INFO "Hello world 2.\n");
	
	/*
	 * A non 0 return means init_module failed. Module can't be loaded.
	 */
	 
	return (0);
}

static void __exit hello_2_exit(void)
{
	printk(KERN_INFO "Goodbey world 2.\n");
}

module_init(hello_2_init);
module_exit(hello_2_exit);
