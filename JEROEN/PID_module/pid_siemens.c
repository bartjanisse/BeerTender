#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define Kc 2.0
#define Ts 1
#define Ti 1000000000000000000
#define Td 0.1

float PIDcal(float SPn, float PVn)
{
	static float MX = 0;
	static float PVnn = 0;
	float output, MPn, MIn, MDn;
	
	MPn = Kc * (SPn - PVn);
	
	MIn = Kc * (Ts / Ti) * (SPn - PVn) + MX;
	MX = MIn;
	
	MDn = Kc * (Td / Ts) * (PVnn - PVn);
	PVnn = PVn;
	
	output = MPn + MIn + MDn;
	
	printf("MPn %f\tMIn %f\tMDn %f\tPVnn %f\toutput %f\n", MPn, MIn, MDn, PVnn, output);
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
		float output = PIDcal(setpoint, position);
		
		position = position + (output / 5);
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
