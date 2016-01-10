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
 * The structs used to pass data via the following ioctl. 
 */
struct PWM_DATA {
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
#define CLK_SET  	_IOW(PWM_MAGIC, 0, struct CLOCK)
#define PWM_INIT 	_IOW(PWM_MAGIC, 1, struct PWM_DATA)
#define PWM_SET  	_IOW(PWM_MAGIC, 2, struct PWM_DATA)
#define PWM_ENABLE	_IOW(PWM_MAGIC, 3, struct PWM_DATA)

#define PWM_MAX_IOCTL 3

#endif /* _PWM_H_ */
