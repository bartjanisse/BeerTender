/**
 * This is the main program for controlling the beertender robot. 
 * 
 * Author: Jeroen De Deken
 * 	
 * basic flow:
 * read the sensor value from gyro-accelero
 * interpretation of the value
 * send input to PID
 * get output from PID
 * interpretation of the PID output value
 * send input to PWM module
 * ...
 * 
 */
 
#include "pid.h"
#include "pwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define FLOATTOINT 10000.0

static int gyro_file_desc;
static int pwm_file_desc;
static int pid_file_desc;


/*
 * 
 */
void InitializePID()
{
	struct PID pidsettings;
	
	pidsettings.Kc = 2;
	pidsettings.Ts = 10;
	pidsettings.Ti = 999999;
	pidsettings.Td = 1;

	ioctl(pid_file_desc, PID_SET, &pidsettings);
}
/*
 * 
 */
void InitializePWM()
{
}

/*
 * 
 */
void InitializeGyro()
{
}


/*
 * 
 */
float CalculatePID(float Sp, float Pv, int reset)
{
	int feedback;
	float returnvalue;
	struct INPUT input;
	
	int iSp = Sp * FLOATTOINT;
	int iPv = Pv * FLOATTOINT;
	
	input.setpoint = iSp;
	input.processValue = iPv;
	input.reset = reset;

	feedback = ioctl(pid_file_desc, PID_GET, &input);
	returnvalue = feedback / FLOATTOINT;

	return returnvalue;
}


/*
 * 
 */
int OpenDeviceFiles()
{
	//gyro_file_desc = open(GYRO_DEVICE_FILE_NAME, 0);
	//if (gyro_file_desc < 0)
	//{
		//printf("Error opening device file: %s\n", GYRO_DEVICE_FILE_NAME);
		//return -1;
	//}
	
	pwm_file_desc = open(PWM_DEVICE_NAME, 0);
	if (pwm_file_desc < 0)
	{
		printf("Error opening device file: %s\n", PWM_DEVICE_NAME);
		return -1;
	}
	
	pid_file_desc = open(PID_DEVICE_FILE_NAME, 0);
	if (pid_file_desc < 0)
	{
		printf("Error opening device file: %s\n", PID_DEVICE_FILE_NAME);
		return -1;
	}
	
	return 0;
}

/*
 * 
 */
void CloseDeviceFiles()
{
	close(gyro_file_desc);
	close(pwm_file_desc);
	close(pid_file_desc);
}


int main(int argc, char **argv)
{	
	if (OpenDeviceFiles() != 0)
		return -1;
		
	InitializePWM();
	InitializeGyro();
	InitializePID();
	
	while (1)
	{
		// read gyro value
		
		// interpreation of gyro value
		
		// pid calculation
		CalculatePID(0.0, 0.0, 0);
		
		// interpretation pid value
		
		// send input to pwm
		
	}
	
	CloseDeviceFiles();
	
	return 0;
}
