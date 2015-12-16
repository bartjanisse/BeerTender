/*
 * (C) 2015 - A.W. Janisse
 * 
 * main.c - Main pwm module definitions.
 * 
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>        // for copy_from_user()

//#include <linux/io.h>			// for ioremap()
//#include <linux/slab.h>		// for kmalloc()
//#include <asm/io.h>
//#include <linux/ioport.h>		// for request_mem_region(), release_mem_region()

//#include "RPI.h"
#include "pwm.h"
#include "pwm_mod.h"

static int major;               /* Major number assigned to our device driver */
static DEFINE_MUTEX(pwm_mutex);

/** 
 * This function is called whenever a process tries to do an ioctl on our
 * device file. We get two extra parameters (additional to the inode and file
 * structures, which all device functions get): the number of the ioctl called
 * and the parameter given to the ioctl function.
 *
 * If the ioctl is write or read/write (meaning output is returned to the
 * calling process), the ioctl call returns the output of this function.
 *
 */
static long pwm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	
	/* Switch according to the ioctl called */
	switch (cmd) {

		case PWM_ECHO:
		{
			// THIS IS FOR TESTING !!!!!!
			printk(KERN_INFO "Function is called %lu\n", arg);
			// 
			// This ioctl is both input (ioctl_param) and 
			// output (the ret value) 
			//
			ret = arg;
			break;
		}
			
		case PWM_SET:
		{
			struct PWM pwm;
			
			if (copy_from_user(&pwm, (struct pwm*)arg, sizeof(pwm))){
				return -EFAULT;
			}
			/* Begin critical section */
			mutex_lock(&pwm_mutex); 

			printk(KERN_INFO "PWM id = %d\n", pwm.id);
			printk(KERN_INFO "PWM gpio = %d\n", pwm.gpio);
			
			/* Store pwm running state */
			
			/* Update registers */
			
			/* Restore pwm running state */
						
			mutex_unlock(&pwm_mutex); 
			/* End critical section */
			
			break;
		}	
		default: // Defined by POSIX standard
			return -ENOTTY;
	}

	return ret;
}

/** 
 * This structure will hold the functions to be called 
 * when a process does something to the device we 
 * created. Since a pointer to this structure is kept in 
 * the devices table, it can't be local to init_module.  
 */ 
static const struct file_operations pwm_fops = {
		.owner   		= THIS_MODULE,
        .unlocked_ioctl = pwm_ioctl
};

/** 
 * init_module() - This function is called when the module is loaded
 * 
 * Return: On succes the function retuns a 0 (SUCCES). Otherwise a 
 * negative number is returned.
 */
int init_module(void)
{
	int ret_val;

	/* Register device and report errors */
	if ((major= register_chrdev(0, DEVICE_NAME, &pwm_fops)) < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}
    
    printk(KERN_INFO "Succesfully registered the %s module\n", DEVICE_NAME);
     
	return SUCCESS;
}

/**
 * Cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	/*if(gpio)
	{
        // release the mapping
        iounmap(gpio);
	}*/
	
	unregister_chrdev(major, DEVICE_NAME);
}
