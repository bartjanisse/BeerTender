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
#include <asm/uaccess.h>        // for copy_from_user()

#include "pwm.h"
#include "RPI.h"
#include "pwm_mod.h"

static unsigned int major;				/* Major number assigned to our device driver */
static unsigned int pwm_0_irq = 0;		/* Interrupt for PWM 0 */
static unsigned int pwm_1_irq = 0;		/* Interrupt for PWM 1 */
static DEFINE_MUTEX(pwm_mutex);			/* Used for critical sections */

/**
 * 
 */
static void map_registers(void) 
{
	gpio = (volatile unsigned int *)ioremap(GPIO_BASE,  4096);
	pwm  = (volatile unsigned int *)ioremap(PWM_BASE,   4096);
	clk  = (volatile unsigned int *)ioremap(CLOCK_BASE, 4096);
}

/**
 * 
 */
static void unmap_registers(void) 
{
	if (gpio) {
        iounmap(gpio);
	}
	if (pwm) {
		iounmap(pwm);
	}
	if (clk) {
		iounmap(clk);
	}
}

/**
 * Assigns the the alternate function for the gpio's which will be 
 * used for both PWM.  
 */
void pwm_gpio_init(void) 
{
	SET_GPIO_ALT(PWM_0_GPIO, GPIO_ALTFUN); 
    SET_GPIO_ALT(PWM_1_GPIO, GPIO_ALTFUN);
} 

void pwm_validate(struct PWM *p)
{
	if (p->id > 1) {
		p->id = 1;
	}
	
	p->duty = (p->duty > 1000) ? 1000 : p->duty;
}

/**
 * 	Start PWM in M/S transmission mode.
 */ 
void pwm_start(struct PWM *p) 
{
	if (p->id == 0) {
		*(pwm + PWM_CTL) = PWM_CTL_MSEN1 | PWM_CTL_PWEN1;
	} else {
		*(pwm + PWM_CTL) = PWM_CTL_MSEN2 | PWM_CTL_PWEN2;
	}
}

/**
 * 
 */
void pwm_stop(struct PWM *p) 
{
	if (p->id == 0) {
		*(pwm + PWM_CTL) = *(pwm + PWM_CTL) & ~PWM_CTL_PWEN1;
	} else {
		*(pwm + PWM_CTL) = *(pwm + PWM_CTL) & ~PWM_CTL_PWEN2;
	}
}

/**
 * Function to set PWM cycle and duty cycle
 * 
 * @param cycle. Value used for the pwm range. 
 * @param duty. PWM Duty cycle. 0 = 0%, 1000 = 100%.
 */
void pwm_set(struct PWM *p)
{
	/* Limit duty to 1000 */
	//p.duty = (duty > 1000) ? 1000 : duty;
	
	/* disable PWM and start with a clean register */
	*(pwm + PWM_CTL) = 0;
	
	/* needs some time until the PWM module gets disabled */
	udelay(10);  
	
	/* Set the PWM range and PWM dat */
	*(pwm + PWM_RNG1) = p->cycle;
	*(pwm + PWM_DAT1) = p->cycle / 1000 * p->duty;
	
	// start PWM in M/S transmission mode
	//*(pwm + PWM_CTL) = PWM_CTL_MSEN1 | PWM_CTL_PWEN1;	
	pwm_start(p);
}

/**
 * 
 */
void clk_set(struct CLOCK *c) 
{	
	/* Stop clock by killing it. Clearing enable doesn't work */
	*(clk + CLK_CTL) = CLK_PASSWD | CLK_CTL_KILL;

	/* Set clock divider */		
	*(clk + CLK_DIV)  = CLK_PASSWD | CLK_DIV_DIVI(c->divider); 

	/* set source=osc and enable clock */	
	*(clk + CLK_CTL) = CLK_PASSWD | CLK_CTL_ENAB | CLK_CTL_SRC(c->source); 
		
	/* Wait for busy flag to get high */	
	while ((*clk + CLK_CTL) & !CLK_CTL_BUSY);
}

/* ------------------------------------------------------------------ */
/* Interrupt functions                                                */
/* ------------------------------------------------------------------ */

/**
 * r_irq_handler() - IRQ handler fired on interrupt
 */
static irqreturn_t pwm_0_irq_handler(int irq, void *dev_id, struct pt_regs *regs) 
{
	unsigned long flags;
   
	// disable hard interrupts (remember them in flag 'flags')
	local_irq_save(flags);

	// Set the output
	if (GPIO_READ(PWM_0_GPIO)) {
		GPIO_SET(14); // ******* GPIO_OUTP
	} else {
		GPIO_CLR(14);  // **** GPIO_OUTP
	}

	// restore hard interrupts
	local_irq_restore(flags);

	return (IRQ_HANDLED);
}

/**
 * irq_config() - This function configures interrupts.
 */
void irq_config(unsigned int irq_nr) 
{
	//see .../arch/arm/mach-bcm2709/include/mach/gpio.h and irqs.h
	/*if ((pwm_0_irq_number = gpio_to_irq(PWM_0_GPIO)) < 0 ) {
		printk("GPIO to IRQ mapping failure %s\n", PWM0_GPIO_DESC);
		return;
	}*/
	
	//see ../include/linux/interrupt.h
	if (request_irq(irq_nr, (irq_handler_t )pwm_0_irq_handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, PWM0_GPIO_DESC, PWM0_NAME)) {
		printk(KERN_CRIT "Irq Request failure\n");
		return;
	}
}

/**
 * irq_request_nr() - This function requests an interrupt number for
 * the given GPIO.
 */
unsigned int irq_request_nr(unsigned int gpio_nr)
{
	unsigned int irq_nr;
	
	//see .../arch/arm/mach-bcm2709/include/mach/gpio.h and irqs.h
	if ((irq_nr = gpio_to_irq(gpio_nr)) < 0 ) {
		printk(KERN_CRIT "GPIO to IRQ mapping failure for GPIO %d\n", gpio_nr);
		return (0);
	}
	printk(KERN_INFO "Mapped interrupt %d to GIO %d\n", irq_nr, gpio_nr);
	
	return irq_nr;
}

/**
 * irq_release() - This function releases interrupts.            
 */
void irq_release(unsigned int irq_nr, void *dev_id) 
{
	free_irq(irq_nr, dev_id);
	return;
}

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
			struct PWM p;
			
			if (copy_from_user(&p, (struct p*)arg, sizeof(p))){
				return -EFAULT;
			}
		
			/* Begin critical section. Data is written to the hardware */
			mutex_lock(&pwm_mutex); 
			
			pwm_validate(&p);
			pwm_set(&p);
									
			mutex_unlock(&pwm_mutex); 
			/* End critical section */
			
			break;
		}	
		case CLK_SET:
		{
			struct CLOCK c;
			
			if (copy_from_user(&c, (struct c*)arg, sizeof(clk))){
				return -EFAULT;
			}
		
			/* Begin critical section. Data is written to the hardware */
			mutex_lock(&pwm_mutex); 
			
			//clk_validate(&c); //************** TODO ******
			clk_set(&c);
									
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
	/* Register device and report errors */
	if ((major= register_chrdev(0, PWM_DEVICE_NAME, &pwm_fops)) < 0) {
		printk(KERN_ALERT "Registering char device failed with %d\n", major);
		return major;
	}
    
    printk(KERN_INFO "Succesfully registered the %s module\n", PWM_DEVICE_NAME);
     
    map_registers(); 
    pwm_gpio_init();
    
    // Set pin directions for the output
	INP_GPIO(14);
	OUT_GPIO(14);
     
	return SUCCESS;
}

/**
 * Cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	
	unmap_registers();
	unregister_chrdev(major, DEVICE_NAME);
}
