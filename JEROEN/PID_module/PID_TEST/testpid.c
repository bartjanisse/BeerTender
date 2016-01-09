#include "../PID_KERNEL/pid.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define FLOATTOINT 10000.0

void pid_echo(int file_desc)
{
	int i;
	printf("pid_echo: ");
	
	i = ioctl(file_desc, PID_ECHO, 122);
	
	printf("received %d\n", i);
}

void pid_set(int file_desc)
{
	struct PID pidsettings;
	
	pidsettings.Kc = 2;
	pidsettings.Ts = 10;
	pidsettings.Ti = 999999;
	pidsettings.Td = 1;
	
	printf("pid_set:\n");
	
	ioctl(file_desc, PID_SET, &pidsettings);
}

float pid_get(int file_desc, float Sp, float Pv, int reset)
{
	int returnvalue;
	struct INPUT input;
	
	input.setpoint = Sp;
	input.processValue = Pv;
	input.reset = reset;
	
	printf("pid_get: ");
	
	returnvalue = ioctl(file_desc, PID_GET, &input);
	
	printf("received %d\n", returnvalue);
	
	return returnvalue;
}

int main()
{
	int file_desc;
	
	float Sp = 1.0;
	float Pv = 0.0;
	float retval;
	
	file_desc = open(DEVICE_FILE_NAME, 0);
	if (file_desc < 0)
	{
		printf("Error opening device file: %s\n", DEVICE_FILE_NAME);
		exit(-1);
	}
	
	pid_set(file_desc);
	sleep(1);

	while(1)
	{
		retval = pid_get(file_desc, Sp * FLOATTOINT, Pv * FLOATTOINT);
		Pv = Pv + ((retval / FLOATTOINT) / 5);
		usleep(100*1000);
	}
	
	close(file_desc);
	
	return 0;
}
