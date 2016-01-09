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
#define PWM0_NAME 		"PWM 0"						/* Used for assigning interrupt */
#define PWM1_NAME 		"PWM 1"						/* Used for assigning interrupt */
#define PWM0_GPIO_DESC	"PWM channel 0 interrupt"	/* Name as it appears in /proc/interrupts */
#define PWM1_GPIO_DESC	"PWM channel 1 interrupt"	/* Name as it appears in /proc/interrupts */

#define PWM_0_GPIO		12	// Since GPIO 12 is not connected on the Raspberry Pi pcb, use it as pwm 
#define PWM_1_GPIO		13	// Since GPIO 13 is not connected on the Raspberry Pi pcb, use it as pwm
#define GPIO_ALTFUN		0	// Sine we are using GPIO 12 and 13 we need alternate function 0

#define SUCCESS 		0

struct PWM {
	unsigned int gpio;
	unsigned int msen;
	unsigned int pwen;
	unsigned int rng;
	unsigned int dat;
	unsigned int irq_nr;
	irq_handler_t handler;
	//struct PWM_DATA *data;
	void *dev;
};

/* Function prototypes */
extern void pwm_gpio_init(void); 
//extern void pwm_validate(struct PWM *);

extern void pwm_start(struct PWM *);
extern void pwm_stop(struct PWM *);
//extern void pwm_set(struct PWM *); 
void pwm_set(struct PWM *pwm, struct PWM_DATA *);
void irq_config(struct PWM *p, struct PWM_DATA *);
#endif /* _PWM_MOD_H_ */
