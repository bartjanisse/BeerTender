/*
 *  XXXXXX.c - the process to use ioctl's to control the kernel module
 *
 *  XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *  Until now we could have used cat for input and output.  But now
 *  we need to do ioctl's, which require writing our own process.
 * XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXxx
 */

/* 
 * device specifics, such as ioctl numbers and the
 * major device file. 
 */
#include "../pwm_source/pwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>			/* open */
#include <unistd.h>			/* exit */
#include <sys/ioctl.h>		/* ioctl */

struct PWM_DATA pwm1;
static int pwm_file_desc;

/* 
 * Functions for the ioctl calls 
 */

void pwm_set(int c, int d)
{
	pwm1.id 	= 0;//0;
	pwm1.gpio 	= 14;
	pwm1.cycle 	= c;
	pwm1.duty 	= d;
	pwm1.enable = 0;
	
	printf("pwm_set. cycle = %d, duty = %d\n", c, d);
	
	ioctl(pwm_file_desc, PWM_SET, &pwm1);
}

void pwm_init()
{
	pwm1.id	= 0;//0;
	pwm1.gpio	= 14;
	pwm1.cycle	= 10000;
	pwm1.duty	= 100;
	pwm1.enable = 0;
	
	printf("pwm_init\n");
	
	ioctl(pwm_file_desc, PWM_INIT, &pwm1);
}

void clock_set()
{
	struct CLOCK clock;
	
	clock.source = 1;
	clock.divider = 96;
	
	printf("clk_set\n");
	
	ioctl(pwm_file_desc, CLK_SET, &clock);
}

void pwm_start()
{
	pwm1.enable = 1;
	
	printf("pwm_start\n");
	
	ioctl(pwm_file_desc, PWM_ENABLE, &pwm1);
}

void pwm_stop()
{
	pwm1.enable = 0;
	
	printf("pwm_stop\n");
	
	ioctl(pwm_file_desc, PWM_ENABLE, &pwm1);
}

void pwm_cycle(int c)
{
	int d = 0;
	
	for(d = 0; d<10; d++)
	{
		pwm_set(c, d * 100);
		sleep(2);
	}
}

/* 
 * Main - Call the ioctl functions 
 */
int main()
{
	
	
	pwm_file_desc = open(PWM_DEVICE_NAME, 0);
	if (pwm_file_desc < 0) {
		printf("Can't open device file: %s\n", PWM_DEVICE_NAME);
		exit(-1);
	}
	clock_set();
	sleep(1);
	pwm_init();
	sleep(1);
	pwm_set(10000, 0);
	pwm_start();
	
	pwm_cycle(10000);
	pwm_cycle(1000);

	pwm_stop();
	sleep(2);
	pwm_start();
	sleep(2);
	pwm_stop();
	close(pwm_file_desc);
	
	return 0;
}
