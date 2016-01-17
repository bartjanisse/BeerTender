/*
 * 15-01-2016 - ROBBAX
 * 
 * beerButton.c: Creates a module that can read values of an incoming digital port in this example the input from those digital ports is pushed to the kernel information stack
 *
 * notes:
 * 		- use make to build this module.
 * 		- use make install to copy the following files to the Pi /bin directory:
 * 			o  beerButton.ko		The module.
 * 			o  beerButton			See below fo an explanation.
 * 
 * 		- You can use the script beerButton with the following command params:
 * 			o  start:  	Load module into pi - button thread is starting
 * 			o  stop:    Unload module - thread reading button is killed
 * 			o  on:      This command puts power on the LEDS (if attached)
 * 			o  off:		This command closes power on the leds
 * 			o  state:	Shows if there is power on the leds
 * 
 * 			The leds are mainly for indication the for functioning of the button the                    leds are not nescessary.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/io.h>			// for ioremap()
#include <linux/slab.h>			// for kmalloc()
#include <asm/uaccess.h>        // for put_user
#include <asm/io.h>
#include <linux/ioport.h>		// for request_mem_region(), release_mem_region()
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/kthread.h>      //used for creation of thread
#include <linux/input.h>
#include <linux/delay.h>        //used for the sleep of the thread
#include "RPI.h"
#include "beerButton.h"         //set correct adresses of RPI2

/** 
 * Global variables are declared as static, so are global within the file. 
 */
static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open?, used to prevent multiple access to device */
static char msg[BUF_LEN];       /* The msg the device will give when asked */
static char *msg_Ptr;
static struct task_struct *checkGpioThread; // Create struct for the GPIO thread

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
 * checkGpioThread_fn() - Function is runned as lon as thread isn't stopped
 *
 * Function print to kernel_info the value on the input pin of the RPI2
 */
int checkGpioThread_fn(void *data)
{
	while (!kthread_should_stop())
    {	
		
		msleep(1000);
		printk(KERN_INFO "%i\n",GPIO_READ(GPIO_KEY));
        
        //TODO: Write values to User-space

	}
	
	return 0;
   
}

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
	
	
	//Set thread to handle button
    //Thread must be created and directly started to view the button presses
	checkGpioThread = kthread_create(checkGpioThread_fn, NULL, "checkGpioThread");
    if(!checkGpioThread)
    {
        printk(KERN_CRIT "kthread_create failed\n");
    }
    else
    {
		printk(KERN_CRIT "kthread created\n");
	}
	
    //thread must be awakened to start
	wake_up_process(checkGpioThread);
	
	
	// Set pin directions for the LED
	INP_GPIO(GPIO_LED);
    OUT_GPIO(GPIO_LED);

    
    // Set pin directions for the INPUT
	INP_GPIO(GPIO_KEY);
    
	return SUCCESS;
}

/**
 * Cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    //Button thread must be killed when module is unloaded
	kthread_stop(checkGpioThread);
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
	{
		return -EBUSY;
	}
     
	Device_Open++;
	
	/* Initialize the msg */
	msg_Ptr = msg;
                 
	try_module_get(THIS_MODULE);
	
	return SUCCESS;
}

/** 
 * device_release() - Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    /* 
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module. 
     */
	Device_Open--;          /* We're now ready for our next caller */
	
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
    //Power on the leds shows if button is reacting on the signal
	if(GPIO_READ(GPIO_LED))
	{
		printk(KERN_INFO "POWER = HIGH\n");
		put_user(1, buffer);
	}
	else
	{
		printk(KERN_INFO "POWER = LOW\n");
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
		printk(KERN_INFO "Put POWER on\n");
		GPIO_SET(GPIO_LED);
	} 
	else
	{
		printk(KERN_INFO "Put POWER off\n");
		GPIO_CLR(GPIO_LED);
	}

	return i;
}



