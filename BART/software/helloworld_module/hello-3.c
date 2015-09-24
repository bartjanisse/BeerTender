/*
 * 19-09-2015 - Bart Janisse
 * 
 * hello-3.c - Demonstrating the __init, __initdata and __exit macros.
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

static int hello3_data __initdata = 3;

static int __init hello_3_init(void)
{
	printk(KERN_INFO "Hello world %d\n", hello3_data);
	
	/*
	 * A non 0 return means init_module failed. Module can't be loaded.
	 */
	 
	return (0);
}

static void __exit hello_3_exit(void)
{
	printk(KERN_INFO "Goodbey world 3.\n");
}

module_init(hello_3_init);
module_exit(hello_3_exit);
