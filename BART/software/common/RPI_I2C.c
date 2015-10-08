#include "RPI_I2C.h"

struct bcm_peripheral bsc1 = {BSC1_BASE};

void dump_bsc_status() 
{
	unsigned int s = BSC1_S;

	printf("BSC1_A: 0x%.2X BSC1_S: ERR=%d  RXF=%d  TXE=%d  RXD=%d  TXD=%d  RXR=%d  TXW=%d  DONE=%d  TA=%d\n",
		BSC1_A,
		(s & BSC_S_ERR)  != 0,
		(s & BSC_S_RXF)  != 0,
		(s & BSC_S_TXE)  != 0,
		(s & BSC_S_RXD)  != 0,
		(s & BSC_S_TXD)  != 0,
		(s & BSC_S_RXR)  != 0,
		(s & BSC_S_TXW)  != 0,
		(s & BSC_S_DONE) != 0,
		(s & BSC_S_TA)   != 0);
}

// Function to wait for the I2C transaction to complete
void wait_i2c_done() 
{
	//Wait till done, let's use a timeout just in case
	int timeout = 50;
    
	while((!((BSC1_S) & BSC_S_DONE)) && --timeout) 
	{
		usleep(1000);
	}

	if(timeout == 0)
	{
		printf("wait_i2c_done() timeout. Something went wrong.\n");
	}
}

int init_i2c()
{
	return map_peripheral(&bsc1);
}

void close_i2c()
{
	unmap_peripheral(&bsc1);
}

