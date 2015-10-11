#include "RPI_GPIO.h"

struct bcm_peripheral gpio = {GPIO_BASE};

int init_gpio(void)
{
	return map_peripheral(&gpio);
}

void close_gpio(void)
{
	unmap_peripheral(&gpio);
}
