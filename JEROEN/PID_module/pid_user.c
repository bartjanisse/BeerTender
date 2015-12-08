#include<stdio.h>

#define epsilon 100   // minimal difference to let the integrator work
#define dt 10         // miliseconds
#define MAX 10000	  // max output
#define MIN -10000    // min output

#define Kp 1
#define Ki 0
#define Kd 1

// get the absolute value implemented instead of including stlib for kernel space implementation
int abs(int i)
{      /* compute absolute value of int argument */
	return (i < 0 ? -i : i);
}

// error als input
// pagina 146 van de siemens manual s7 1200
// in user space kan het met floats, module moet met int werken (-10000 tot 10000)
// output loggen in een file om zichtbaar te maken in excel 
// stap response als input omheen bouwen
int PIDcal(int error)
{
	static int previous_error = 0;
	static int integral = 0;
	int derivative;
	int output;
	
	// in case error is to small stop integration
	if (abs(error) > epsilon)
	{
		integral = integral + error * dt;
	}
	derivative = (error - previous_error)/dt;
	output = Kp*error + Ki*integral + Kd*derivative;
	
	if (output > MAX)
	{
		output = MAX;
	}
	else if (output < MIN)
	{
		output = MIN;
	}
	
	previous_error = error;
	
	return output;
}

int main(int argc, char **argv)
{
	int setpoint = 0;
	int position = 0;
	int counter = 0;
		
	while (position < MAX)
	{
		int error = setpoint - position;
		
		//printf("setpoint = %d, position = %d, error = %d\n", setpoint, position, error);
		printf("%d;", position);

		position = PIDcal(error);
		
		counter++;
		if (counter > 10)
			setpoint = 10;
		if (counter > 100)
			break;
	}
	
	return 0;
}
