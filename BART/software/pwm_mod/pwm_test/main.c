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
#include <fcntl.h>		/* open */
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */

/* 
 * Functions for the ioctl calls 
 */
void pwm_echo(int file_desc)
{
	int i;
	//char c;

	printf("pwm_echo: ");

	i = ioctl(file_desc, PWM_ECHO, 122);
	
	printf("received %d back\n", i);
}

void pwm_set(int file_desc)
{
	struct PWM pwm1;
	
	pwm1.id = 1;
	pwm1.gpio = 18;
	
	printf("pwm_set\n");
	
	ioctl(file_desc, PWM_SET, &pwm1);
}

/* 
 * Main - Call the ioctl functions 
 */
int main()
{
	int file_desc;
	
	file_desc = open(DEVICE_FILE_NAME, 0);
	if (file_desc < 0) {
		printf("Can't open device file: %s\n", DEVICE_FILE_NAME);
		exit(-1);
	}

	pwm_echo(file_desc);
	sleep(1);
	pwm_set(file_desc);

	close(file_desc);
	
	return 0;
}
