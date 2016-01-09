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

/* 
 * Functions for the ioctl calls 
 */
 /*
void pwm_echo(int file_desc)
{
	int i;
	//char c;

	printf("pwm_echo: ");

	i = ioctl(file_desc, PWM_ECHO, 122);
	
	printf("received %d back\n", i);
}*/

void pwm_set(int file_desc)
{
	struct PWM_DATA pwm1;
	
	pwm1.id = 0;
	pwm1.gpio = 14;
	pwm1.cycle = 100000;
	pwm1.duty = 500;
	
	printf("pwm_set\n");
	
	ioctl(file_desc, PWM_SET, &pwm1);
}

void pwm_init(int file_desc)
{
	struct PWM_DATA pwm1;
	
	pwm1.id = 0;
	pwm1.gpio = 14;
	pwm1.cycle = 100000;
	pwm1.duty = 500;
	
	printf("pwm_init\n");
	
	ioctl(file_desc, PWM_INIT, &pwm1);
}

void clock_set(int file_desc)
{
	struct CLOCK clock;
	
	clock.source = 1;
	clock.divider = 96;
	
	printf("clk_set\n");
	
	ioctl(file_desc, CLK_SET, &clock);
}

/* 
 * Main - Call the ioctl functions 
 */
int main()
{
	int pwm_file_desc;
	
	pwm_file_desc = open(PWM_DEVICE_NAME, 0);
	if (pwm_file_desc < 0) {
		printf("Can't open device file: %s\n", PWM_DEVICE_NAME);
		exit(-1);
	}
	clock_set(pwm_file_desc);
	sleep(1);
	pwm_init(pwm_file_desc);
	sleep(1);
	pwm_set(pwm_file_desc);

	close(pwm_file_desc);
	
	return 0;
}
