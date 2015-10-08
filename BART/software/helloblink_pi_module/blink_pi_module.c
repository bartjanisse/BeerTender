/*
 * 20-09-2015 - Bart Janisse
 * 
 * chardev.c: Creates a read−only char device that says how many times
 * you've read from the dev file
 * 
 * This example is taken from the book The Linux Kernel Programming Guide.
 * The book is freely downloadable from the internet.
 * 
 * notes:
 * 		- use make to build this module.
 * 		- use modinfo chardev.ko to see the information in it.
 * 		- use sudo insmod ./chardev.ko to insert it in the kernel.
 * 		- use sudo rmmod chardev to remove it from the kernel.
 * 		- use cat /proc/modules | head to see if it is loaded.
 * 		- use cat /var/log/kern.log | tail to see how to how to create the devicefile 
 * 		  for example: sudo mknod /dev/chardev c 250 0
 * 		- Try cat /dev/chardev several times to see the output
 * 		- Try echo "hi" > /dev/chardev to see the output
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>        /* for put_user */

#include "RPI.h"
#include "RPI_GPIO.h"

/*  
 *  Prototypes − this would normally go in a .h file
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"   /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80              /* Max length of the message from the device */
#define LED_PIN		14 			/* GPIO 14 is pin 8 */

/* 
 * Global variables are declared as static, so are global within the file. 
 */
static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open?  
                                 * Used to prevent multiple access to device */
static char msg[BUF_LEN];       /* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .release = device_release
};

/*
 * This function is called when the module is loaded
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
	printk(KERN_INFO "the driver, create a dev file with\n");
        
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
    
    
    if(init_gpio() == -1) 
	{
		printk(KERN_ERR "Failed to map the physical GPIO registers into the virtual memory space.\n");
        return (-1);
    }
    
	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
//	close_gpio();
	
	unregister_chrdev(Major, DEVICE_NAME);
}

/*
 * Methods
 */

/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;
	
	if (Device_Open)
	{
		return -EBUSY;
	}
     
	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);
	
	return SUCCESS;
}
/* 
 * Called when a process closes the device file.
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

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,   /* see include/linux/fs.h   */
                           char *buffer,        /* buffer to fill with data */
                           size_t length,       /* length of the buffer     */
                           loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer 
     */
	int bytes_read = 0;
    /*
     * If we're at the end of the message, 
     * return 0 signifying end of file 
     */
	if (*msg_Ptr == 0)
	{
		return 0;
	}
    /* 
     * Actually put the data into the buffer 
     */
	while (length && *msg_Ptr) 
	{
		/* 
         * The buffer is in the user data segment, not the kernel 
         * segment so "*" assignment won't work.  We have to use 
         * put_user which copies data from the kernel data segment to
         * the user data segment. 
         */
		put_user(*(msg_Ptr++), buffer++);
                length--;
                bytes_read++;
	}
        
    /* 
     * Most read functions return the number of bytes put into the buffer
     */
	return bytes_read;
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/chardev 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation isn't supported.\n");

	return -EINVAL;
}


//************************


struct GpioRegisters
{
    uint32_t GPFSEL[6];
    uint32_t Reserved1;
    uint32_t GPSET[2];
    uint32_t Reserved2;
    uint32_t GPCLR[2];
};


struct GpioRegisters *s_pGpioRegisters;

static void SetGPIOFunction(int GPIO, int functionCode)
{
    int registerIndex = GPIO / 10;
    int bit = (GPIO % 10) * 3;

    unsigned oldValue = s_pGpioRegisters-> GPFSEL[registerIndex];
    unsigned mask = 0b111 << bit;
    printk("Changing function of GPIO%d from %x to %x\n",
           GPIO,
           (oldValue >> bit) & 0b111,
           functionCode);

    s_pGpioRegisters-> GPFSEL[registerIndex] =
        (oldValue & ~mask) | ((functionCode << bit) & mask);
}

static void SetGPIOOutputValue(int GPIO, bool outputValue)
{
    if (outputValue)
        s_pGpioRegisters-> GPSET[GPIO / 32] = (1 << (GPIO % 32));
    else
        s_pGpioRegisters-> GPCLR[GPIO / 32] = (1 << (GPIO % 32));
}

s_pGpioRegisters = (struct GpioRegisters *)__io_address(GPIO_BASE);

#include <linux/timer.h>
#include <linux/err.h>

static struct timer_list s_BlinkTimer;
static int s_BlinkPeriod = 1000;
static const int LedGpioPin = 18;

static void BlinkTimerHandler(unsigned long unused)
{
    static bool on = false;
    on = !on;
    SetGPIOOutputValue( LedGpioPin, on);
    mod_timer(&s_BlinkTimer,
              jiffies + msecs_to_jiffies( s_BlinkPeriod));
}

static int __init LedBlinkModule_init(void)
{
    int result;

    s_pGpioRegisters =
        (struct GpioRegisters *)__io_address(GPIO_BASE);
    SetGPIOFunction( LedGpioPin, 0b001); //Output

    setup_timer(&s_BlinkTimer, BlinkTimerHandler, 0);
    result = mod_timer( &s_BlinkTimer,
                       jiffies + msecs_to_jiffies( s_BlinkPeriod));
    BUG_ON(result < 0);
}

static void __exit LedBlinkModule_exit(void)
{
    SetGPIOFunction( LedGpioPin, 0); //Configure the pin as input
    del_timer(&s_BlinkTimer);
}
//************************************
static ssize_t set_period_callback(struct device* dev,
                                   struct device_attribute* attr,
                                   const char* buf,
                                   size_t count)
{
    long period_value = 0;
    if (kstrtol(buf, 10, &period_value) < 0)
        return -EINVAL;
    if (period_value < 10) //Safety check
        return -EINVAL;

    s_BlinkPeriod = period_value;
    return count;
}

static DEVICE_ATTR(period,
                   S_IWUSR | S_IWGRP | S_IWOTH,
                   NULL,
                   set_period_callback);

static struct class *s_pDeviceClass;
static struct device *s_pDeviceObject;

