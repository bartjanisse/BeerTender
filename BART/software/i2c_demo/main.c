/*
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "../common/RPI.h"
#include "../common/RPI_I2C.h"
#include "../common/RPI_GPIO.h"

#define 	MCP_ADDRESS 0x20		// Offset in MCP23008
#define		I2C_ADDRESS 0x00		// MCP23008 device addres set by pins A0, A1 and A2

#define		MCP_IODIR	0x00		// I/O direction register
#define		MCP_GPIO	0x09		// General purpose I/O port regoster

void MCP_SetRegister(unsigned char  regAddr, unsigned char  regVal) 
{
	// I2C Device Address
    BSC1_A = MCP_ADDRESS|I2C_ADDRESS;
   
    dump_bsc_status();
    
	BSC1_DLEN = 1;
	BSC1_FIFO = (unsigned char)regAddr;
	BSC1_S = CLEAR_STATUS;		// Reset status bits (see #define)
    BSC1_C = START_WRITE;    	// Start Write (see #define)
    
    wait_i2c_done();
    
    BSC1_DLEN = 1;
    BSC1_FIFO = (unsigned char)regVal;
	BSC1_S = CLEAR_STATUS;		// Reset status bits (see #define)
    BSC1_C = START_WRITE;    	// Start Write (see #define)
    
    wait_i2c_done();
    
    dump_bsc_status();
    
    //printf("I2C done\n");
}

int main()
{
	// Initialize GPIO
	if (init_gpio() == -1)
    {
		printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
	}

	// Initialize I2C functions
	if (init_i2c() == -1)
	{
		printf("Failed to map the physical BSC0 (I2C) registers into the virtual memory space.\n");
	}
	
	// We need I2C so define alternate functions for GPIO0 and GPIO1.
    INP_GPIO(2);
	SET_GPIO_ALT(2, ALT0);
	INP_GPIO(3);
	SET_GPIO_ALT(3, ALT0);
	
	// I2C Device Address
    BSC1_A = MCP_ADDRESS|I2C_ADDRESS;
    	
	MCP_SetRegister(MCP_IODIR, 0x00);		// All MCP23008 ports are outputs
	       
	while(1)
	{
		printf("High\n");
		MCP_SetRegister(MCP_GPIO, 0xFF);	// All ports High
		
		usleep(3000 * 1000);
		
		printf("Low\n");
		MCP_SetRegister(MCP_GPIO, 0x00);	// All ports low
		
		usleep(3000 * 1000);
	}

	return 0;	
}
