#include "../PID_KERNEL/pid.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define FLOATTOINT 10000.0

static int file_desc;

/*
 * Send a command to the pid module to reset the values
 */
void Pid_reset()
{
	printf("pid_reset:\n");	
	ioctl(file_desc, PID_RESET);
}

/*
 * Send a command to the pid module to change the settings
 */
void Pid_set(int Kc, int Ts, int Ti, int Td)
{
	struct PID pidsettings;
	
	pidsettings.Kc = Kc;
	pidsettings.Ts = Ts;
	pidsettings.Ti = Ti;
	pidsettings.Td = Td;
	
	printf("\npid_set:\n");
	
	ioctl(file_desc, PID_SET, &pidsettings);
}

/*
 * Send a command to the pid module to calculate a value
 */
int Pid_get(int Sp, int Pv)
{
	int returnvalue;
	
	struct INPUT input;

	input.setpoint = Sp;
	input.processValue = Pv;
	
	printf("\npid_get: ");
	
	returnvalue = ioctl(file_desc, PID_GET, &input);
	
	printf("received %d\n", returnvalue);
	
	return returnvalue;
}

/*
 * Ask for the settings of the pid calculation
 */
void Enter_pid_settings()
{
	static int loopGain = 1;
	static int sampleTime = 10;
	static int integrationPeriod = 999999;
	static int differationPeriod = 0;
	
	char stringValue[99];
	int value;
	
	printf("Send new settings to the pid module\n");
	
	printf("Enter the loop gain (%d) ", loopGain);
	fgets(stringValue, sizeof(stringValue), stdin);
	loopGain = atoi(stringValue);
	
	printf("Enter the sample time (%d) ", sampleTime);
	fgets(stringValue, sizeof(stringValue), stdin);
	sampleTime = atoi(stringValue);
	
	printf("Enter the integration period, set very large to dissable (%d) ", integrationPeriod);
	fgets(stringValue, sizeof(stringValue), stdin);
	integrationPeriod = atoi(stringValue);
	
	printf("Enter the differation period, set to zero to dissable (%d) ", differationPeriod);
	fgets(stringValue, sizeof(stringValue), stdin);
	differationPeriod = atoi(stringValue);
	
	Pid_set(loopGain, sampleTime, integrationPeriod, differationPeriod);
	printf("\n\n");
}

/*
 * Ask for the input values for the calculation
 */
void Execute_calculation()
{
	static int setpoint = 1 * FLOATTOINT;
	static int processValue = 0 * FLOATTOINT;
	float result;
	
	char stringValue[99];
	
	printf("Send a calculation to the pid module\n");

	printf("Enter the set point as int (%d) ", setpoint);
	fgets(stringValue, sizeof(stringValue), stdin);
	setpoint = atoi(stringValue);
	
	printf("\nEnter the process value as int (%d) ", processValue);
	fgets(stringValue, sizeof(stringValue), stdin);
	processValue = atoi(stringValue);
	
	result = Pid_get(setpoint, processValue);
	
	printf("\n\nPID module returned %f", result);
	
	printf("\n\n");
}

/*
 * Show the menu and get the user input
 */
char Show_menu()
{
	char input[99];
	
	printf("PID MENU\n");
	printf("--------\n");
	printf("[1] enter pid settings\n");
	printf("[2] execute a calculation\n");
	printf("[3] reset the PID\n");
	printf("\n");
	printf("[q] quit menu\n");
	printf("\n");
	printf("Enter your option: ");
	
	fgets(input, sizeof(input), stdin);
	printf("\n");
	return input[0];
}

/*
 * Do the selected option
 */
void Switch_option(char option)
{
	switch(option)
	{
		case '1':
			Enter_pid_settings();
			break;
		case '2':
			Execute_calculation();
			break;
		case '3':
			Pid_reset();
			break;
		default:
			printf("Invalid option\n");
			break;
	}
}

/*
 * The main loop of the program
 */
int main()
{
	char option = '0';
	
	// open file for communication
	file_desc = open(PID_DEVICE_FILE_NAME, 0);
	if (file_desc < 0)
	{
		printf("Error opening device file: %s\n", PID_DEVICE_FILE_NAME);
		exit(-1);
	}
	
	while (1)
	{
		option = Show_menu();
		
		if (option == 'q') break;
		
		Switch_option(option);
	}
	
	close(file_desc);
	
	return 0;
}
