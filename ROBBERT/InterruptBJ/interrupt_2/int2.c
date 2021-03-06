/*
 * 09-11-2015 - Bart Janisse
 * 
 * int2.c: Creates a module to demonstrate interrupt handling of a GPIO
 * output. This module generates an interrupt when the LED output is
 * switched high. As a result of this a message will be written.
 * 
 * notes:
 * 		- use make to build this module.
 * 		- use make install to copy the following files to the Pi /bin directory:
 * 			o  int2.ko		The module.
 * 			o  int2			Shell script. See below fo an explanation.
 * 
 * 		- You can use the script led with the following command params:
 * 			o  start:  	This loads the module and creates the appropiate 
 * 						device file (mknod).
 * 			o  stop:	This unloads the module and removes the device
 * 						file.
 * 			o  on:		Switches the LED on
 * 			o  off:		Switches the LED off
 * 			o  state:	Shows the actual LED state as reported by the 
 * 						module.
 * 
 * 			Please the see the led script for detailed information.
 * 
 */

#include <linux/fs.h>      		// for file_operations
#include <asm/uaccess.h>		// for put_user(), get_user()
#include <linux/io.h>			// for ioremap(), iounmap()
#include <linux/interrupt.h>	// for irq_handler_t, 
#include <linux/gpio.h>			// for gpio_to_irq(), gpio_request(), gpio_free(), see ../Documentation/gpio

#include "RPI.h"
#include "int2.h"

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
        .release = device_release,
        .owner	 = THIS_MODULE
};

/* ------------------------------------------------------------------ */
/* Interrupt functions                                                */
/* ------------------------------------------------------------------ */

short int irq_number    = 0;

/**
 * r_irq_handler() - IRQ handler fired on interrupt
 */
static irqreturn_t r_irq_handler(int irq, void *dev_id, struct pt_regs *regs) 
{
	unsigned long flags;
   
	// disable hard interrupts (remember them in flag 'flags')
	local_irq_save(flags);

	printk(KERN_NOTICE "Interrupt [%d] for device %s was triggered !\n", irq, (char *) dev_id);

	// restore hard interrupts
	local_irq_restore(flags);

	return (IRQ_HANDLED);
}

/**
 * interrupt_config() - This function configures interrupts.
 */
void interrupt_config(void) 
{
	if ( (irq_number = gpio_to_irq(GPIO_LED)) < 0 ) 
	{
		printk("GPIO to IRQ mapping failure %s\n", GPIO_ANY_GPIO_DESC);
		return;
	}

	printk(KERN_NOTICE "Mapped int %d\n", irq_number);
	
	//see ../include/linux/interrupt.h
	if (request_irq(irq_number, (irq_handler_t ) r_irq_handler, IRQF_TRIGGER_RISING, GPIO_ANY_GPIO_DESC, DEVICE_NAME)) 
	{
		printk("Irq Request failure\n");
		return;
	}

	return;
}

/**
 * interrupt_release() - This function releases interrupts.            
 */
void interrupt_release(void) 
{
	free_irq(irq_number, DEVICE_NAME);

	return;
}

/* ------------------------------------------------------------------ */
/* Module functions                                                   */
/* ------------------------------------------------------------------ */

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
    
	gpio = (volatile unsigned int *)ioremap(GPIO_BASE, 4096);
	
	// Set pin directions for the LED
	INP_GPIO(GPIO_LED);
	OUT_GPIO(GPIO_LED);
 
	interrupt_config();
    
	return SUCCESS;
}

/**
 * Cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	interrupt_release();
	
	if(gpio)
	{
        // release the mapping
        iounmap(gpio);
	}
	
	unregister_chrdev(Major, DEVICE_NAME);
}

/* ------------------------------------------------------------------ */
/* Device functions                                                   */
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
	if(GPIO_READ(GPIO_LED))
	{
		printk(KERN_INFO "LED state is on\n");
		put_user(1, buffer);
	}
	else
	{
		printk(KERN_INFO "LED state is off\n");
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
		printk(KERN_INFO "Put LED on\n");
		GPIO_SET(GPIO_LED);
	} 
	else
	{
		printk(KERN_INFO "Put LED off\n");
		GPIO_CLR(GPIO_LED);
	}

	return i;
}
