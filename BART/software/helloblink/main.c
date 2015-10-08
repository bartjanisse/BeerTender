
#include <stdio.h>
#include <unistd.h>

#include "../common/RPI.h"
#include "../common/RPI_GPIO.h"

#define LED_PIN		14 // GPIO 14 is pin 8

int 
main()
{
	if(init_gpio() == -1) 
	{
		printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        return (-1);
    }
	
	printf("Helloblink is started with GIOP%d at pin number 8.\n", LED_PIN);
	
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
