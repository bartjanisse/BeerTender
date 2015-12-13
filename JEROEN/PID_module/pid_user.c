#include<stdio.h>

#define epsilon 100   	// minimal difference to let the integrator work
#define dt 10         	// miliseconds
#define MAX 1000000	  	// max output
#define MIN -1000000	// min output

#define Kp 0
#define Ki 1
#define Kd 0

// get the absolute value implemented instead of including stdlib for kernel space implementation
int abs(int i)
{      /* compute absolute value of int argument */
	return (i < 0 ? -i : i);
}

// error als input
// pagina 146 van de siemens manual s7 1200
// in user space kan het met floats, module moet met int werken (-10000 tot 10000)
// output loggen in een file om zichtbaar te maken in excel 
// stap response als input omheen bouwen
int PIDcal(int input)
{
	static int previous_input = 0;
	static int integral = 0;
	int derivative;
	int output;

	//printf("previous input = %d\n", previous_input);
	
	// in case error is to small stop integration
	if (abs(input) > epsilon)
	{
		integral = integral + input * dt;
	}
	derivative = (input - previous_input) / dt;
	//printf("integral = %d derivative = %d\n", integral, derivative);

	output = Kp*input + Ki*integral + Kd*derivative;
	//printf("output = %d\n", output);
	
	if (output > MAX)
	{
		output = MAX;
	}
	else if (output < MIN)
	{
		output = MIN;
	}
	
	previous_input = input;
	
	return output;
}

int main(int argc, char **argv)
{
	int output = 0;
	int counter = 0;
		
	while (1)
	{
		int input = 1;
		
		output = PIDcal(input * 10000) / 10000;
		printf("%d;", output);
		
		counter++;
		if (counter > 100)
		{
			printf("break the loop\n");
			break;
		}
	}
	
	return 0;
}
