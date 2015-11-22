/*
 * 22-11-2015 - Bart Janisse
 * 
 * pwm1.c: Creates a basic module to demonstrate the hardware PWM0 and the
 * posibility to use a interrupt for redirecting the PWM output to any GPIO.
 * 
 * Since this program is used for controlling a servo motor, the puls length
 * from the PWM can vary from 1.0ms to 2.0ms. The cycle time is fixed at 20ms.
 * 
 * notes:
 * 		- use make to build this module.
 * 		- use make install to copy the following files to the Pi /bin directory:
 * 			o  pwm1.ko	The module.
 * 			o  pwm1		Shell script. See below fo an explanation.
 * 
 * 		- You can use the script pwm1 with the following command params:
 * 			o  start:  	This loads the module and creates the appropiate 
 * 						device file (mknod).
 * 			o  stop:	This unloads the module and removes the device
 * 						file.
 * 			o  info:	Displays the last 20 messages.
 * 
 * 			Please the see the int1 script for detailed information.
 * 
 * When the module is loaded you can set the dutycycles from the prompt
 * by writing to the devicefile. This can be done with:
 * 
 * 		$echo 50 > /dev/pwm1
 * 
 * In here 50 is the percentage of the programmed dutycylce range which 
 * will result in a puls lengt of 1.5ms
 */

#include <linux/fs.h>      		// for file_operations
#include <asm/uaccess.h>		// for put_user(), get_user()
#include <linux/io.h>			// for ioremap(), iounmap()
#include <linux/interrupt.h>	// for irq_handler_t, 
#include <linux/gpio.h>			// for gpio_to_irq(), gpio_request(), gpio_free(), see ../Documentation/gpio
#include <linux/delay.h>		// for udelay()
#include <linux/kernel.h> 		// for kstrtoint()
#include "RPI.h"
#include "pwm1.h"

/** 
 * Global variables are declared as static, so are global within the file. 
 */
static int Major;               /* Major number assigned to our device driver */
static int Device_Open = 0;     /* Is device open?, used to prevent multiple access to device */
static char msg[BUF_LEN];       /* The msg the device will give when asked */

/** 
 * This structure will hold the functions to be called 
 * when a process does something to the device we 
 * created. Since a pointer to this structure is kept in 
 * the devices table, it can't be local to init_module.  
 */ 
static struct file_operations fops = {
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

	// Set the output
	if (GPIO_READ(GPIO_PWM0)) {
		GPIO_SET(GPIO_OUTP);
	} else {
		GPIO_CLR(GPIO_OUTP);
	}

	// restore hard interrupts
	local_irq_restore(flags);

	return (IRQ_HANDLED);
}

/**
 * interrupt_config() - This function configures interrupts.
 */
void interrupt_config(void) 
{
	//see .../arch/arm/mach-bcm2709/include/mach/gpio.h and irqs.h
	if ( (irq_number = gpio_to_irq(GPIO_PWM0)) < 0 ) {
		printk("GPIO to IRQ mapping failure %s\n", GPIO_ANY_GPIO_DESC);
		return;
	}

	printk(KERN_NOTICE "Mapped interrupt %d\n", irq_number);
	
	//see ../include/linux/interrupt.h
	if (request_irq(irq_number, (irq_handler_t ) r_irq_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, GPIO_ANY_GPIO_DESC, DEVICE_NAME)) {
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

void setServo(int percent)
{
	unsigned dat = 0;

	printk(KERN_INFO "Servo speed is set to %d\n", percent);

	dat = 200 + 200 * percent / 100;
	if (dat > 400) dat = 400;
	if (dat < 200) dat = 200;
	*(pwm + PWM_DAT1) = dat;
}

/** 
 * init_module() - This function is called when the module is loaded
 * 
 * Return: On succes the function retuns a 0 (SUCCES). Otherwise a 
 * negative number is returned.
 */
int init_module(void)
{
	int idiv;
	
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", Major);
		return Major;
	}
    
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a device file with\n");
        
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
    
	gpio = (volatile unsigned int *)ioremap(GPIO_BASE,  4096);
	pwm  = (volatile unsigned int *)ioremap(PWM_BASE,   4096);
	clk  = (volatile unsigned int *)ioremap(CLOCK_BASE, 4096);
	
	// Set pin directions for the output
	INP_GPIO(GPIO_OUTP);
	OUT_GPIO(GPIO_OUTP);
	
	// PWM and clock settings 
	
	SET_GPIO_ALT(GPIO_PWM0, GPIO_ALT);
	
	// stop clock and waiting for busy flag doesn't work, so kill clock
	*(clk + CLK_CTL) = CLK_PASSWD | CLK_CTL_KILL; 
	udelay(10);  
	
	//idiv = (int) (19200000.0f / 16000.0f);
	idiv = 96;
	*(clk + CLK_DIV)  = CLK_PASSWD | CLK_DIV_DIVI(idiv); 
	
	// source=osc and enable clock
	*(clk + CLK_CTL) = CLK_PASSWD | CLK_CTL_ENAB | CLK_CTL_SRC(CLK_CTL_SRC_OSC); 

	// disable PWM and start with a clean register
	*(pwm + PWM_CTL) = 0;
	
	// needs some time until the PWM module gets disabled, without the delay the PWM module crashes
	udelay(10);  
	
	// set the number of bits for a period of 20 milliseconds = 320 bits
	*(pwm + PWM_RNG1) = 4000;
	
	// 32 bits = 2 milliseconds, init with 1 millisecond
	setServo(50);
	
	// start PWM in M/S transmission mode
	*(pwm + PWM_CTL) = PWM_CTL_MSEN1 | PWM_CTL_PWEN1;
	interrupt_config();
    
	return SUCCESS;
}

/**
 * Cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	interrupt_release();
	
	if(gpio) {
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
	if (Device_Open) {
		return -EBUSY;
	}
     
	Device_Open++;
                 
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
 * device_write() - Called when a process writes to the device file 
 */
static ssize_t device_write(struct file *filp, 
							const char *buff, 
							size_t len, 
							loff_t * off)
{
	int size, percent;
	
	if (len > 4) {
		size = 4;
	} else {
		size = len;
	}
	
	// Copy data from user space to kernel space.
	if (copy_from_user(msg, buff, size)) {
		return -EFAULT;
	}
	msg[len] = '\0';
	
	// Convert to integer 
	if ( kstrtoint(msg, 10, &percent) < 0 ) {
		return -EFAULT;
	}
	
	setServo(percent);

	return size;
}
