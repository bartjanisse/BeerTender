/*
 * (C) 2015 - A.W. Janisse
 * 
 * pwm_mod.h - Header file with the module definitions.
 * 
 */
#ifndef _PWM_MOD_H_
#define _PWM_MOD_H_

MODULE_LICENSE("GPL");
MODULE_AUTHOR("A.W. Janisse");
MODULE_DESCRIPTION("PWM module");

#define DEVICE_NAME 	"PWM"						/* Device name as it appears in /proc/devices */
#define PWM0_DEV 		"PWM 0"						/* Used for assigning interrupt */
#define PWM1_DEV 		"PWM 1"						/* Used for assigning interrupt */
#define PWM0_NAME		"PWM channel 0 interrupt"	/* Name as it appears in /proc/interrupts */
#define PWM1_NAME		"PWM channel 1 interrupt"	/* Name as it appears in /proc/interrupts */

#define PWM_0_GPIO		12	/* Since GPIO 12 is not connected on the Raspberry Pi pcb we use it as pwm */
#define PWM_1_GPIO		13	/* Since GPIO 13 is not connected on the Raspberry Pi pcb we use it as pwm */
#define GPIO_ALTFUN		0	/* Alternate function for PWM */

#define SUCCESS 		0

struct PWM {
	unsigned int gpio;
	unsigned int pwm_gpio;
	unsigned int msen;
	unsigned int pwen;
	unsigned int rng;
	unsigned int dat;
	unsigned int irq_nr;
	irq_handler_t irq_handler;
	char *name;
	void *dev;
};

/* Function prototypes */
/*
extern void pwm_set_enable(struct PWM *, struct PWM_DATA *);
extern void pwm_set_data(struct PWM *, struct PWM_DATA *);
extern void irq_config(struct PWM *, struct PWM_DATA *);
extern void irq_release(struct PWM *); 
*/
#endif /* _PWM_MOD_H_ */
