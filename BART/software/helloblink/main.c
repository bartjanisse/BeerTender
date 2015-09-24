
#include <stdio.h>
#include <unistd.h>

#include "../common/RPI.h"

#define LED_PIN		4 // GPIO 4 is pin 17

int 
main()
{
	if(map_peripheral(&gpio) == -1) 
	{
		printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        return (-1);
    }
	
	// Define gpio 4 (LED) as output
	INP_GPIO(LED_PIN);
	OUT_GPIO(LED_PIN);

	while(1)
	{
		GPIO_SET(LED_PIN);
		usleep(100 * 1000);
	
		GPIO_CLR(LED_PIN);
		usleep(100 * 1000);
	}

	return 0;	
}
