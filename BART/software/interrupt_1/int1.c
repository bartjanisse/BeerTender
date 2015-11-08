/*
 * 08-11-2015 - Bart Janisse
 * 
 * int1.c: Creates a module to demonstrate basic interrupt handling.
 * For this a push button should be connected to GPIO 23 (pin 16). When
 * pushing this button a interrupt is fired. In this event the state of a 
 * led connected to GPIO 14 (pin 8) will be toggled on and off. Please
 * keep in mind that this module is kept as simple as possible. So there
 * is no function to debounce the push button. Because of this a single
 * press on the button can cause multiple interrupts to fire.  
 * 
 * notes:
 * 		- use make to build this module.
 * 		- use make install to copy the following files to the Pi /bin directory:
 * 			o  int1.ko	The module.
 * 			o  int1		See below fo an explanation.
 * 
 * 		- You can use the script led with the following command params:
 * 			o  start:  	This loads the module and creates the appropiate 
 * 						device file (mknod).
 * 			o  stop:	This unloads the module and removes the device
 * 						file.
 * 			o  info:	Outputs last 20 lines of /var/log/messages.
 * 
 * 			Please the see the int1 script for detailed information.
 * 
 */

#include <linux/io.h>			// for ioremap(), iounmap()
#include <linux/interrupt.h>	// for irq_handler_t, 
#include <linux/gpio.h>			// for gpio_to_irq(), gpio_request(), gpio_free() (see ../Documentation/gpio)

#include "RPI.h"
#include "int1.h"

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

	// Toggle LED
	if (GPIO_READ(GPIO_LED))
	{
		GPIO_CLR(GPIO_LED);
	}
	else
	{
		GPIO_SET(GPIO_LED);
	}

	// restore hard interrupts
	local_irq_restore(flags);

	return (IRQ_HANDLED);
}

/**
 * interrupt_config() - This function configures the interrupt.
 */
void interrupt_config(void) 
{
	if ( (irq_number = gpio_to_irq(GPIO_INPUT)) < 0 ) 
	{
		printk("GPIO to IRQ mapping failure %s\n", GPIO_ANY_GPIO_DESC);
		return;
	}

	printk(KERN_NOTICE "Mapped int %d\n", irq_number);
	
	//see ../include/linux/interrupt.h
	if (request_irq(irq_number, (irq_handler_t ) r_irq_handler, IRQF_TRIGGER_RISING, GPIO_ANY_GPIO_DESC, NULL)) 
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
	free_irq(irq_number, NULL);

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
	gpio = (volatile unsigned int *)ioremap(GPIO_BASE, 4096);
	
	// Set pin directions for the LED
	INP_GPIO(GPIO_LED);
	OUT_GPIO(GPIO_LED);
	// Set pin directions for the INPUT
	INP_GPIO(GPIO_INPUT);
 
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
}
