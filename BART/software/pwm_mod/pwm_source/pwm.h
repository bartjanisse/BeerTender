/*
 * (C) 2015 - A.W. Janisse
 * 
 * pwm.h - Header file with ioctl definitions.
 * 
 * The declarations here are in a header file, because they need to be 
 * known both to this kernel module and the user space process calling 
 * this module.
 * 
 */
 
#ifndef _PWM_H_
#define _PWM_H_

/*
 * The struct used to pass data via the following ioctl. 
 */
struct PWM {
	unsigned int id;
	unsigned int gpio;
	unsigned int enable;
	unsigned int cycle;
	unsigned int duty;
};

struct CLOCK {
	unsigned int source;
	unsigned int divider;
};

#include <linux/ioctl.h>

#define PWM_DEVICE_NAME "/dev/pwm"
#define PWM_MAGIC 'K'

/* ioctl() calls that are permitted to the /dev/pwm interface. */
#define PWM_SET  _IOW(PWM_MAGIC, 0, struct PWM)
#define CLK_SET  _IOR(PWM_MAGIC, 1, struct CLOCK)
#define PWM_ECHO _IOWR(PWM_MAGIC, 2, int) // FOR TESTING ONLY !!!!

#define PWM_MAX_IOCTL 2

#endif /* _PWM_H_ */
