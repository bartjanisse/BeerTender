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
 * r_irq_handler() - IRQ handler fired on interrupt
 */
static irqreturn_t pwm_0_irq_handler(int irq, void *dev_id, struct pt_regs *regs) 
{
	unsigned long flags;
   
	// disable hard interrupts (remember them in flag 'flags')
	local_irq_save(flags);

	// Set the output
	if (GPIO_READ(PWM_0_GPIO)) {
		GPIO_SET(pwm0->gpio);
	} else {
		GPIO_CLR(pwm0->gpio);
	}

	// restore hard interrupts
	local_irq_restore(flags);
	
	
	printk(KERN_INFO "PWM 0 int\n");
	return (IRQ_HANDLED);
}

/**
 * r_irq_handler() - IRQ handler fired on interrupt
 */
static irqreturn_t pwm_1_irq_handler(int irq, void *dev_id, struct pt_regs *regs) 
{
	unsigned long flags;
   
	// disable hard interrupts (remember them in flag 'flags')
	local_irq_save(flags);

	// Set the output
	if (GPIO_READ(PWM_0_GPIO)) {
		GPIO_SET(pwm0->gpio);
	} else {
		GPIO_CLR(pwm0->gpio);
	}

	// restore hard interrupts
	local_irq_restore(flags);

	return (IRQ_HANDLED);
}

/**
 * irq_config() - This function configures interrupts.
 */
void irq_config(struct PWM *p, struct PWM_DATA *data)
{
	// Store GPIO in PWM struct so it is available in the interrupt handler.
	p->gpio = data->gpio;
	
	//see .../arch/arm/mach-bcm2709/include/mach/gpio.h and irqs.h
	if ((p->irq_nr = gpio_to_irq(data->gpio)) < 0 ) {
		printk(KERN_CRIT "GPIO to IRQ mapping failure for GPIO %d\n", data->gpio);
		return;
	}
	//see ../include/linux/interrupt.h
	if (request_irq(p->irq_nr, (irq_handler_t)p->handler, IRQF_TRIGGER_RISING|IRQF_TRIGGER_FALLING, PWM0_GPIO_DESC, p->dev)) {
		printk(KERN_CRIT "Irq Request failure\n");
		return;
	}
	
	printk(KERN_INFO "PWM %d interrupt %d is mapped to GIO %d\n", data->id, p->irq_nr, data->gpio);
}

/**
 * irq_release() - This function releases interrupts.            
 */
/*void irq_release(unsigned int irq_nr, void *dev_id) 
{
	free_irq(irq_nr, dev_id);
	return;
}*/

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
struct PWM *pwm_create(unsigned int msen, 
						unsigned int pwen,
						unsigned int rng,
						unsigned int dat,
						irq_handler_t handler,
						void *dev)
{
	// Allocate memory for the struct
	struct PWM *p = kmalloc(sizeof(struct PWM), GFP_KERNEL);
    // If no memory allocated report it and return
    if(!p) {
		printk(KERN_CRIT "kmalloc failure!\n");
		return NULL;
	}
	// Assign values
    p->msen = msen;
    p->pwen = pwen;
    p->rng = rng;
    p->dat = dat;
    p->handler = handler;
    p->dev = dev;
    p->irq_nr = 0;
    
    return p; 
}

/**
 * Function pwm_destroy() - 
 */
void pwm_destroy(struct PWM *p)
{
	if(p) {
		if (p->irq_nr) {
			free_irq(p->irq_nr, p->dev);
		}
		kfree(p);
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
 * Function pwm_start - Starts the given PWM in M/S transmission mode.
 */ 
void pwm_start(struct PWM *pwm) 
{
	*(pwm_addr + PWM_CTL) = pwm->msen | pwm->pwen;
	
	printk(KERN_INFO "PWM is started\n");
}

/**
 * Function pwm_stop() - Stops the given PWM by clearing the enable flag.
 */
void pwm_stop(struct PWM *pwm) 
{
	*(pwm_addr + PWM_CTL) = *(pwm_addr + PWM_CTL) & ~pwm->pwen;
	
	printk(KERN_INFO "PWM is stopped\n");
}

/**
 * Function to set PWM cycle and duty cycle
 */
void pwm_set(struct PWM *pwm, struct PWM_DATA *data)
{
	/* disable PWM and start with a clean register */
	*(pwm_addr + PWM_CTL) = 0; // !!!!!!! KAN NIET OP 0 BEGINNEN we hebben 2x PWM !!!!!!
	
	/* needs some time until the PWM module gets disabled */
	udelay(10);  
	
	/* Set the PWM range and PWM dat */
	*(pwm_addr + pwm->rng) = data->cycle;
	*(pwm_addr + pwm->dat) = data->cycle / 1000 * data->duty;

	pwm_start(pwm); // ???? Moet dit hier ????
	
	printk(KERN_INFO "PWM %d cycle = %d, duty = %d \n", data->id, data->cycle, data->duty);
}

/* ------------------------------------------------------------------ */
/* Clock functions                                                    */
/* ------------------------------------------------------------------ */

/**
 * clk_set() - 
 */
void clk_set(struct CLOCK *clock) 
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
static void map_registers(void) 
{
	gpio_addr = (volatile unsigned int *)ioremap(GPIO_BASE,  4096);
	pwm_addr  = (volatile unsigned int *)ioremap(PWM_BASE,   4096);
	clk_addr  = (volatile unsigned int *)ioremap(CLOCK_BASE, 4096);
}

/**
 * Function unmap_registers() - 
 */
static void unmap_registers(void) 
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
			struct CLOCK c;
			
			if (copy_from_user(&c, (struct c*)arg, sizeof(c))){
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
			
			// Config the interrupt for the requested pwm
			/*if (data.id == 0) {
				pwm0->data = &data;
				irq_config(pwm0);
			} else {
				pwm1->data = &data;
				irq_config(pwm1);
			}*/
			if (data.id == 0) {
				irq_config(pwm0, &data);
			} else {
				irq_config(pwm1, &data);
			}
			
			mutex_unlock(&pwm_mutex); 
			/* End critical section */
			printk(KERN_INFO "PWM %d done initializing \n", data.id); 
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
			
		//	pwm_validate(&p); TODO
			
			/*if (data.id == 0) {
				pwm0->data = &data;
				pwm_set(pwm0);
			} else {
				pwm1->data = &data;
				pwm_set(pwm1);
			}*/
			if (data.id == 0) {
				pwm_set(pwm0, &data);
			} else {
				pwm_set(pwm1, &data);
			}
									
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
		return major;
	}
    
    map_registers(); 

    SET_GPIO_ALT(PWM_0_GPIO, GPIO_ALTFUN); 
    SET_GPIO_ALT(PWM_1_GPIO, GPIO_ALTFUN);
    
    pwm0 = pwm_create(PWM_CTL_MSEN1, PWM_CTL_PWEN1, PWM_RNG1, PWM_DAT1, (irq_handler_t)pwm_0_irq_handler, PWM0_NAME);
    pwm1 = pwm_create(PWM_CTL_MSEN2, PWM_CTL_PWEN2, PWM_RNG2, PWM_DAT2, (irq_handler_t)pwm_1_irq_handler, PWM1_NAME);
    
    // TODO: Controlle of pwm's aangemaakt zijn !!!!
    
    // Set pin directions for the output
	//INP_GPIO(14);
	//OUT_GPIO(14);
	
    //irq_config(pwm0); 
    
    printk(KERN_INFO "Succesfully registered module %s\n", PWM_DEVICE_NAME); 
	return SUCCESS;
}

/**
 * cleanup_module() - This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	pwm_destroy(pwm0);
	pwm_destroy(pwm1);
		
	unmap_registers();
	unregister_chrdev(major, DEVICE_NAME);
}

