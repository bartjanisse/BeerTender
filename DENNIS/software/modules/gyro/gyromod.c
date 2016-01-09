/* 
 * gyromod.c: Creates a read/write char device for starting or stopping
 * gyro measurements. The GYROSCOPE is connected to GPIO2/SDA (pin 3) and 
 * GPIO3/SCL (pin 5) of the Raspberry Pi.
 * 
 * notes:
 * 		- use make to build this module.
 * 		- use make install to copy the following files to the Pi /bin directory:
 * 			o  gyromod.ko		The module.
 * 			o  gyroscript		See below fo an explanation.
 * 
 * 		- You can use the script led with the following command params:
 * 			o  start:  	This loads the module and creates the appropiate 
 * 						device file (mknod).
 * 			o  stop:	This unloads the module and removes the device
 * 						file.
 * 			o  on:		Gyro starts measuring
 * 			o  off:		Gyro stops measuring
 * 			o  state:	Shows the actual LED state as reported by the 
 * 						module.
 * 
 * 			More... see the gyroscript for detailed information.
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>			// for ioremap()
#include <linux/slab.h>			// for kmalloc()
#include <asm/uaccess.h>        // for put_user
#include <asm/io.h>
#include <linux/ioport.h>		// for request_mem_region(), release_mem_region()

#include "RPI.h"
#include "gyromod.h"

MODULE_AUTHOR("Dennis Jessurun"); 

/** 
 * Global variables are declared as static, so are global within the file. 
 */
static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open?, used to prevent multiple access to device */
static char msg[BUF_LEN];       /* The msg the device will give when asked */
static char *msg_Ptr;

/** 
 * This structure will hold the functions to be called 
 * when a process does something to the device we 
 * created. Since a pointer to this structure is kept in 
 * the devices table, it can't be local to init_module.  
 */ 
static struct file_operations fops = {
        .read    = device_read,
        .write   = device_write,
        .open    = device_open,
        .release = device_release
};

/** 
 * init_module() - This function is called when the module is loaded
 * 
 * Return: On succes the function retuns a 0 (SUCCES). Otherwise a 
 * negative number is returned.
 */
int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) 
	{
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}
    
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a device file with\n");
        
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
    
	// $ cat /proc/iomem reports the following addresses:
	// 		3f200000-3f200fff : bcm2708_gpio (this 0xFFF or 4096 registers)
	gpio = (volatile unsigned int *)ioremap(GPIO_BASE, 4096);
	
	// Set pin directions for the gyro i2c
	INP_GPIO(2);
	OUT_GPIO(2);
	INP_GPIO(3);
	OUT_GPIO(3);
    
	return SUCCESS;
}

/**
 * Cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	if(gpio)
	{
        // release the mapping
        iounmap(gpio);
	}
	
	unregister_chrdev(Major, DEVICE_NAME);
}

/* ------------------------------------------------------------------ */
/* Module functions                                                   */
/* ------------------------------------------------------------------ */
 
/** 
 * device_open() - Called when a process tries to open the device 
 * file, like "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	if (Device_Open)
		return -EBUSY;
     
	Device_Open++;	
	msg_Ptr = msg;                 
	try_module_get(THIS_MODULE);	
	
	return SUCCESS;
}

/** 
 * device_release() - Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{    
	Device_Open--;          /* We're now ready for our next caller */
	
	/* 
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module. 
     */
	module_put(THIS_MODULE);	
	
	return 0;
}

/** 
 * device-read() - Called when a process, which already opened the 
 * device file, attempts to read from it.
 */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
                           char *buffer,        /* buffer to fill with data */
                           size_t length,       /* length of the buffer     */
                           loff_t * offset)
{
	if(GPIO_READ(2))
	{
		printk(KERN_INFO "Gyro is on and measuring\n");
		put_user(1, buffer);
	}
	else
	{
		printk(KERN_INFO "Gyro is off\n");
		put_user(0, buffer);
	}
	return 0;
}

/**  
 * device_write() - Called when a process writes to the device file 
 */
static ssize_t device_write(struct file *filp, 
							const char *buff, 
							size_t len, 
							loff_t * off)
{
	int i;
	
	// Copy data from user space to kernel space.
	for(i=0; i<len && i<BUF_LEN; i++)
	{
		get_user(msg[i], buff+i);
	}
	msg_Ptr = msg;

	if(strcmp(msg_Ptr, "1") == 1)
	{
		printk(KERN_INFO "Start gyro measurements\n");
		INP_GPIO(2);
		SET_GPIO_ALT(2, 0);
		INP_GPIO(3);
		SET_GPIO_ALT(3, 0);
	} 
	else
	{
		printk(KERN_INFO "Stop gyro measurements\n");	
		// Set 	
		INP_GPIO(2);
		SET_GPIO_ALT(4, 0);
		INP_GPIO(3);
		SET_GPIO_ALT(5, 0);		
	}

	return i;
}
