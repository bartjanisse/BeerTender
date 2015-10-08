#include "RPI_GPIO.h"

struct bcm_peripheral gpio = {GPIO_BASE};

int init_gpio()
{
	return map_peripheral(&gpio);
}

void close_gpio()
{
	unmap_peripheral(&gpio);
}
