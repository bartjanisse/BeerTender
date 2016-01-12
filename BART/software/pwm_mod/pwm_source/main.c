/*
 * (C) 2015 - A.W. Janisse
 * 
 * main.c - Main pwm module definitions.
 * 
 */
 
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>			// for fops
#include <linux/delay.h>		// for udelay()
#include <linux/io.h>			// for ioremap(), iounmap()
#include <linux/interrupt.h>	// for irq_handler_t, 
#include <linux/gpio.h>			// for gpio_to_irq(), gpio_request(), gpio_free(), see ../Documentation/gpio
#include <linux/slab.h>			// for kmalloc()
#include <asm/uaccess.h>        // for copy_from_user()

#include "pwm.h"
#include "RPI.h"
#include "pwm_mod.h"

static unsigned int major;				/* Major number assigned to our device driver */
static struct PWM *pwm0;				/* Structure which holds the PWM 0 registers and data */
static struct PWM *pwm1;				/* Structure which holds the PWM 1 registers and data */
static DEFINE_MUTEX(pwm_mutex);			/* Used for critical sections */

/* ------------------------------------------------------------------ */
/* Interrupt functions                                                */
/* ------------------------------------------------------------------ */

/**
 * pwm_0_irq_handler() - PWM 0 irq handler fired on interrupt
 */
static irqreturn_t pwm_0_irq_handler(int irq, void *dev_id, struct pt_regs *regs) 
{
	unsigned long flags;
   
	/* disable hard interrupts (remember them in flag 'flags') */
	local_irq_save(flags);

	/* Set the output */
	if (GPIO_READ(PWM_0_GPIO)) {
		GPIO_SET(pwm0->gpio);
	} else {
		GPIO_CLR(pwm0->gpio);
	}

	/* restore hard interrupts */
	local_irq_restore(flags);
	
	return (IRQ_HANDLED);
}

/**
 * pwm_1_irq_handler() - PWM 1 irq handler fired on interrupt
 */
static irqreturn_t pwm_1_irq_handler(int irq, void *dev_id, struct pt_regs *regs) 
{
	unsigned long flags;
   
	/* disable hard interrupts (remember them in flag 'flags') */
	local_irq_save(flags);

	/* Set the output */
	if (GPIO_READ(PWM_1_GPIO)) {
		GPIO_SET(pwm1->gpio);
	} else {
		GPIO_CLR(pwm1->gpio);
	}

	/* restore hard interrupts */
	local_irq_restore(flags);

	return (IRQ_HANDLED);
}

/**
 * irq_release() - This function releases interrupts.            
 */
static void irq_release(struct PWM *pwm) 
{
	if (pwm->irq_nr) {
		free_irq(pwm->irq_nr, pwm->dev);
		pwm->irq_nr = 0;
	}
}

/**
 * irq_config() - This function configures interrupts.
 */
static void irq_config(struct PWM *pwm, struct PWM_DATA *data)
{
	/* Store GPIO in PWM struct so it is available in the interrupt handler */
	pwm->gpio = data->gpio;
	/* Release irq if it was assigned */
	irq_release(pwm);
	
	/* see .../arch/arm/mach-bcm2709/include/mach/gpio.h and irqs.h */
	if ((pwm->irq_nr = gpio_to_irq(pwm->pwm_gpio)) < 0 ) {
		printk(KERN_CRIT "GPIO to IRQ mapping failure for GPIO %d\n", data->gpio);
		return;
	}
	/* see ../include/linux/interrupt.h */
	if (request_irq(pwm->irq_nr, (irq_handler_t)pwm->irq_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, pwm->name, pwm->dev)) {
		printk(KERN_CRIT "Irq Request failure\n");
		return;
	}
	
	printk(KERN_INFO "PWM %d interrupt %d is mapped to GPIO %d\n", data->id, pwm->irq_nr, data->gpio);
}

/* ------------------------------------------------------------------ */
/* PWM functions                                                      */
/* ------------------------------------------------------------------ */

/**
 * Function pwm_create() - Creates a new PWM structure by allocating 
 * memory for it. The new structure will be filled with the given 
 * parameters. When succesfull the new stucture will be returned, 
 * otherwise NULL;
 * 
 * Since memory allocation is done withe the GPF_KERNEL flag, this
 * is a normal allocation and might block.
 */
static struct PWM *pwm_create(unsigned int msen, 
						unsigned int pwen,
						unsigned int rng,
						unsigned int dat,
						irq_handler_t handler,
						unsigned int pwm_gpio,
						char *name,
						void *dev)
{
	/* Allocate memory for the struct */
	struct PWM *pwm = kmalloc(sizeof(struct PWM), GFP_KERNEL);
    /* If no memory allocated report it and return */
    if(!pwm) {
		printk(KERN_CRIT "kmalloc failure!\n");
		return NULL;
	}
	/* Assign values */
    pwm->msen = msen;
    pwm->pwen = pwen;
    pwm->rng = rng;
    pwm->dat = dat;
    pwm->irq_handler = handler;
    pwm->dev = dev;
    pwm->irq_nr = 0;
    pwm->name = name;
    pwm->pwm_gpio = pwm_gpio;
    
    return pwm; 
}

/**
 * Function pwm_destroy() - Free's the allocated memory and releases
 * the interrupt for the given PWM. 
 */
static void pwm_destroy(struct PWM *pwm)
{
	if(pwm) {
		irq_release(pwm);
		kfree(pwm);
	}
}

/*void pwm_validate(struct PWM *p)
{
	if (p->id > 1) {
		p->id = 1;
	}
	
	p->duty = (p->duty > 1000) ? 1000 : p->duty;
}*/

/**
 * Function pwm_set_enable() - Start or stops the given PWM by 
 * clearing or setting the enable flag. The PWM mode will be 
 * M/S transmission mode.
 */
static void pwm_set_enable(struct PWM *pwm, struct PWM_DATA *data) 
{
	if (data->enable) {
		/* Set enable flag */
		*(pwm_addr + PWM_CTL) = *(pwm_addr + PWM_CTL) | pwm->msen | pwm->pwen;
		printk(KERN_INFO "PWM %d started\n", data->id);
	} else {
		/* Clear enable flag */
		*(pwm_addr + PWM_CTL) = *(pwm_addr + PWM_CTL) & ~pwm->pwen;
		printk(KERN_INFO "PWM %d stopped\n", data->id);
	}
}

/**
 * Function to set PWM cycle and duty cycle....verder uitleggen hoe de bereking plaats vindt
 */
static void pwm_set_data(struct PWM *pwm, struct PWM_DATA *data)
{
	/* Set the PWM range and PWM dat */
	*(pwm_addr + pwm->rng) = data->cycle;
	*(pwm_addr + pwm->dat) = data->cycle / 1000 * data->duty;
	
	printk(KERN_INFO "PWM %d cycle = %d, duty = %d \n", data->id, data->cycle, data->duty);
}

/* ------------------------------------------------------------------ */
/* Clock functions                                                    */
/* ------------------------------------------------------------------ */

/**
 * clk_set() - 
 */
static void clk_set(struct CLOCK *clock) 
{	
	/* Stop clock by killing it. Clearing enable doesn't work */
	*(clk_addr + CLK_CTL) = CLK_PASSWD | CLK_CTL_KILL;

	/* Set clock divider */		
	*(clk_addr + CLK_DIV)  = CLK_PASSWD | CLK_DIV_DIVI(clock->divider); 

	/* set source=osc and enable clock */	
	*(clk_addr + CLK_CTL) = CLK_PASSWD | CLK_CTL_ENAB | CLK_CTL_SRC(clock->source); 
		
	/* Wait for busy flag to get high */	
	while ((*clk_addr + CLK_CTL) & !CLK_CTL_BUSY);
	
	printk(KERN_INFO "Clock is started with source = %d, divider = %d \n", clock->source, clock->divider);
}

/* ------------------------------------------------------------------ */
/* Memory functions                                                   */
/* ------------------------------------------------------------------ */

/**
 * Function map_registers() - 
 */
static unsigned int registers_map(void) 
{
	gpio_addr = (volatile unsigned int *)ioremap(GPIO_BASE,  4096);
	if (!gpio_addr) {
		return -1;
	}

	pwm_addr  = (volatile unsigned int *)ioremap(PWM_BASE,   4096);
	if (!pwm_addr) {
		return -1;
	}

	clk_addr  = (volatile unsigned int *)ioremap(CLOCK_BASE, 4096);
	if (!clk_addr) {
		return -1;
	}
	
	return SUCCESS;
}

/**
 * Function unmap_registers() - 
 */
static void registers_unmap(void) 
{
	if (gpio_addr) {
        iounmap(gpio_addr);
	}
	if (pwm_addr) {
		iounmap(pwm_addr);
	}
	if (clk_addr) {
		iounmap(clk_addr);
	}
}

/* ------------------------------------------------------------------ */
/* Module functions                                                   */
/* ------------------------------------------------------------------ */

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
	long ret = SUCCESS;
	
	/* Don't decode wrong cmds. It is better returning ENOTTY than EFAULT */
	if (_IOC_TYPE(cmd) != PWM_MAGIC) return -ENOTTY;
	if (_IOC_NR(cmd) > PWM_MAX_IOCTL) return -ENOTTY;
	
	/* Switch according to the ioctl called */
	switch (cmd) 
	{
		case CLK_SET:
		{		
			struct CLOCK clock;
			
			if (copy_from_user(&clock, (struct clock*)arg, sizeof(clock))){
				return -EFAULT;
			}
		
			/* Begin critical section. Data is written to the hardware */
			mutex_lock(&pwm_mutex); 
			
			//clk_validate(&c); //************** TODO ******
			clk_set(&clock);
									
			mutex_unlock(&pwm_mutex); 
			/* End critical section */
			
			break;
		}
		
		case PWM_INIT:
		{
			struct PWM_DATA data;
			
			if (copy_from_user(&data, (struct data*)arg, sizeof(data))){
				return -EFAULT;
			}
			printk(KERN_INFO "PWM %d initializing\n", data.id); 
			
			/* Begin critical section. Data is written to the hardware */
			mutex_lock(&pwm_mutex);
			
			// Set GPIO as output
			INP_GPIO(data.gpio);
			OUT_GPIO(data.gpio);
						
			data.id ? irq_config(pwm1, &data) : irq_config(pwm0, &data);
			
			mutex_unlock(&pwm_mutex); 
			/* End critical section */
			printk(KERN_INFO "PWM %d initializing done\n", data.id); 
			break;
		}
		
		case PWM_SET:
		{
			struct PWM_DATA data;
			
			if (copy_from_user(&data, (struct data*)arg, sizeof(data))){
				return -EFAULT;
			}
		
			/* Begin critical section. Data is written to the hardware */
			mutex_lock(&pwm_mutex); 
						
			data.id ? pwm_set_data(pwm1, &data) : pwm_set_data(pwm0, &data);			
									
			mutex_unlock(&pwm_mutex); 
			/* End critical section */
			break;
		}
		
		case PWM_ENABLE:
		{
			struct PWM_DATA data;
			
			if (copy_from_user(&data, (struct data*)arg, sizeof(data))){
				return -EFAULT;
			}
		
			/* Begin critical section. Data is written to the hardware */
			mutex_lock(&pwm_mutex); 
						
			data.id ? pwm_set_enable(pwm1, &data) : pwm_set_enable(pwm0, &data);			
									
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
	printk(KERN_INFO "Initializing module %s\n", PWM_DEVICE_NAME);
	
	/* Register device and report errors */
	if ((major= register_chrdev(0, PWM_DEVICE_NAME, &pwm_fops)) < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		goto error;
	}
    
    if (registers_map() != SUCCESS) {
		printk(KERN_ALERT "Mapping registers failed\n");
		goto error;
	} 

	/* These GPIO's are used by the PWM's and but not fysically */
	/* connected on the Raspberry circuit board connector. */
    SET_GPIO_ALT(PWM_0_GPIO, GPIO_ALTFUN); 
    SET_GPIO_ALT(PWM_1_GPIO, GPIO_ALTFUN);
    
    pwm0 = pwm_create(PWM_CTL_MSEN1, PWM_CTL_PWEN1, PWM_RNG1, PWM_DAT1, (irq_handler_t)pwm_0_irq_handler, PWM_0_GPIO, PWM0_NAME, PWM0_DEV);
    pwm1 = pwm_create(PWM_CTL_MSEN2, PWM_CTL_PWEN2, PWM_RNG2, PWM_DAT2, (irq_handler_t)pwm_1_irq_handler, PWM_1_GPIO, PWM1_NAME, PWM1_DEV);
    /* Check if pwm0 and pwm1 are created and report errors */
    if((!pwm0) || (!pwm1)) {	
		printk(KERN_ALERT "Creating PWM's failed\n");
		goto error;
	}
           
    printk(KERN_INFO "Succesfully registered module %s\n", PWM_DEVICE_NAME); 
	
	return SUCCESS;

error:
	registers_unmap();
	pwm_destroy(pwm0);
	pwm_destroy(pwm1);
	return (-1);
}

/**
 * cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	pwm_destroy(pwm0);
	pwm_destroy(pwm1);
		
	registers_unmap();
	
	unregister_chrdev(major, DEVICE_NAME);
}
