#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define Kc 2
#define Ts 10
#define Ti 1000000000000000000
#define Td 1

int PIDcal(int SPn, int PVn)
{
	printf("SPn %d\tPVn %d\n", SPn, PVn);
	static int MX = 0;
	static int PVnn = 0;
	int output, MPn, MIn, MDn;
	
	MPn = Kc * (SPn - PVn);
	
	MIn = Kc * (Ts / Ti) * (SPn - PVn) + MX;
	MX = MIn;
	
	MDn = Kc * (Td / Ts) * (PVnn - PVn);
	PVnn = PVn;
	
	output = MPn + MIn + MDn;
	
	printf("MPn %d\tMIn %d\tMDn %d\tPVnn %d\toutput %d\n", MPn, MIn, MDn, PVnn, output);
	return output;
}


int main(int argc, char **argv)
{
	int counter = 0;
	float position = 0;
	float setpoint = 1;
	
	printf("start\n");	
	while (1)
	{
		counter++;
		printf("runcounter %d\n", counter);
		int inputSet = setpoint * 10000;
		int inputPos = position * 10000;
		float output = PIDcal(inputSet, inputPos);
		
		position = position + ((output /10000) / 5);
		printf("position = %f\n", position);
		
		usleep(100*1000);
		
		if (counter > 50)
		{
			printf("break the loop\n");
			break;
		}
	}
	
	return 0;
}
