/*
 *  main.c - the process to use ioctl's to control the kernel module
 *
 */

/* 
 * device specifics, such as ioctl numbers and the
 * major device file. 
 */
#include "../pwm_source/pwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>			/* for assert() */
#include <fcntl.h>			/* for open() */
#include <unistd.h>			/* for exit() */
#include <sys/ioctl.h>		/* for ioctl() */

struct PWM_DATA *pwm0, *pwm1;

static int pwm_file_desc;

/* 
 * Functions for the ioctl calls 
 */

static void clock_set(unsigned int divider)
{
	struct CLOCK clock;
	
	clock.source = 1;
	clock.divider = divider;
		
	ioctl(pwm_file_desc, CLK_SET, &clock);

	printf("Clock divider set wit %d\n", divider);
}

static void pwm_init(struct PWM_DATA *pwm, unsigned int gpio)
{	
	pwm->gpio = gpio;
	
	ioctl(pwm_file_desc, PWM_INIT, pwm);
	printf("PWM %d init at GPIO %d\n", pwm->id, gpio);
}

static void pwm_set(struct PWM_DATA *pwm)
{
	ioctl(pwm_file_desc, PWM_SET, pwm);
	printf("PWM %d set. cycle = %d, duty = %d\n", pwm->id, pwm->cycle, pwm->duty);
}

static void pwm_start(struct PWM_DATA *pwm)
{
	pwm->enable = 1;
	
	ioctl(pwm_file_desc, PWM_ENABLE, pwm);
	printf("PWM %d started\n", pwm->id);
}

static void pwm_stop(struct PWM_DATA *pwm)
{
	pwm->enable = 0;
		
	ioctl(pwm_file_desc, PWM_ENABLE, pwm);
	printf("PWM %d stopped\n", pwm->id);
}

struct PWM_DATA *pwm_data(unsigned int id, unsigned int gpio, unsigned int cycle, unsigned int duty)
{
	/* Allocate memory for the struct */
	struct PWM_DATA *pwm_data = malloc(sizeof(struct PWM_DATA));
	
	assert(pwm_data != NULL);
	
	pwm_data->id 	= id;
	pwm_data->gpio 	= gpio;
	pwm_data->cycle = cycle;
	pwm_data->duty 	= duty;
	
	return pwm_data;
	
}

/* 
 * Main - Call the ioctl functions 
 */
int main()
{	
	int         	choice = ' ';
	char        	line[80];
	char        	strVal[80];
	unsigned int 	val;
	unsigned int 	id;
		
	pwm_file_desc = open(PWM_DEVICE_NAME, 0);
	if (pwm_file_desc < 0) {
		printf("Can't open device file: %s\n", PWM_DEVICE_NAME);
		exit(-1);
	}
	
	pwm0 = pwm_data(0, 14, 10000, 500);
	pwm1 = pwm_data(1, 15, 10000, 500);
	
	while (choice != '9')
    {
        printf ("\nMenu\n"
                "===================\n"
                "   [1]    set clock\n"
                "   [2]    PWM init\n"
                "   [3]    PWM set cycle\n"
                "   [4]    PWM set duty\n"
                "   [5]    PWM start\n"
                "   [6]    PWM stop\n"
                ""
                "   [9]    quit\n"
                "Enter choice: "
               );
        fgets (line, sizeof (line), stdin);
        choice = line[0];
        printf ("\n");
        
         switch (choice)
        {
            case '1':
				printf("Enter clock divider [example 96]: ");
                fgets (strVal, sizeof (strVal), stdin);
                val = atoi(strVal);
                
                clock_set(val);
                
                break;
                
			case '2':
				printf("Enter PWM id [0, 1]: ");
                fgets (strVal, sizeof (strVal), stdin);
                id = atoi(strVal);
                
                printf("Enter GPIO [14, 15, etc]: ");
                fgets (strVal, sizeof (strVal), stdin);
                val = atoi(strVal);
				
				id ? pwm_init(pwm1, val) : pwm_init(pwm0, val);
				
                break;

			case '3':
				printf("Enter PWM id [0, 1]: ");
                fgets (strVal, sizeof (strVal), stdin);
                id = atoi(strVal);
                
                printf("Enter PWM cycle [10000]: ");
                fgets (strVal, sizeof (strVal), stdin);
                val = atoi(strVal);
                
                if (id == 0) {
					pwm0->cycle = val;
					pwm_set(pwm0);
				} else {
					pwm1->cycle = val;
					pwm_set(pwm1);
				}
               
                break;

			case '4':
				printf("Enter PWM id [0, 1]: ");
                fgets (strVal, sizeof (strVal), stdin);
                id = atoi(strVal);
                
                printf("Enter PWM duty [0..1000]: ");
                fgets (strVal, sizeof (strVal), stdin);
                val = atoi(strVal);
                
                if (id == 0) {
					pwm0->duty = val;
					pwm_set(pwm0);
				} else {
					pwm1->duty = val;
					pwm_set(pwm1);
				}
                
                break;

			case '5':
				printf("Enter PWM id [0, 1]: ");
                fgets (strVal, sizeof (strVal), stdin);
                id = atoi(strVal);
                
                id ? pwm_start(pwm1) : pwm_start(pwm0);
                
                break;
                
			case '6':
				printf("Enter PWM id [0, 1]: ");
                fgets (strVal, sizeof (strVal), stdin);
                id = atoi(strVal);
                
                id ? pwm_stop(pwm1) : pwm_stop(pwm0);
                
                break;
		}
	}
	
	close(pwm_file_desc);
	
	free(pwm0);
	free(pwm1);
	
	return 0;
}
