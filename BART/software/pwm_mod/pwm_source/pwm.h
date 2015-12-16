/*
 * (C) 2015 - A.W. Janisse
 * 
 * pwm.h - Header file with the ioctl definitions.
 * 
 * The declarations here are in a header file, because they need to be 
 * known both to this kernel module and the user space process calling 
 * this module.
 * 
 */
 
#ifndef _PWM_H_
#define _PWM_H_

struct PWM {
	unsigned int id;
	unsigned int gpio;
	unsigned int enable;
	unsigned int range;
	unsigned int data;
};

struct CLOCK {
	unsigned int enable;
	unsigned int source;
	unsigned int divider;
};

#include <linux/ioctl.h>

#define DEVICE_FILE_NAME "/dev/pwm"
#define MAGIC_PATTERN    'K'

#define PWM_SET _IOW(MAGIC_PATTERN, 0, int)
#define PWM_GET _IOR(MAGIC_PATTERN, 1, int)

#define PWM_ECHO _IOWR(MAGIC_PATTERN, 2, int) // FOR TESTING ONLY !!!!


#endif
