

#ifndef _RPI_I2C_H_
#define _RPI_I2C_H_

#include <unistd.h>

#include "RPI.h"

extern struct bcm_peripheral bsc1;

// I2C macros
#define BSC1_C        		*(bsc1.addr + 0) // We are using pointers here so use an index.
#define BSC1_S        		*(bsc1.addr + 1)
#define BSC1_DLEN    		*(bsc1.addr + 2)
#define BSC1_A        		*(bsc1.addr + 3)
#define BSC1_FIFO    		*(bsc1.addr + 4)

#define BSC_C_I2CEN    		(1 << 15)
#define BSC_C_INTR    		(1 << 10)
#define BSC_C_INTT    		(1 << 9)
#define BSC_C_INTD    		(1 << 8)
#define BSC_C_ST    		(1 << 7)
#define BSC_C_CLEAR    		(1 << 4)
#define BSC_C_READ    		1

#define START_READ    		BSC_C_I2CEN|BSC_C_ST|BSC_C_CLEAR|BSC_C_READ
#define START_WRITE   		BSC_C_I2CEN|BSC_C_ST

#define BSC_S_CLKT			(1 << 9)	// RW | Clock Stretch Timeout. 0 = No errors detected. 1 = Slave has held the SCL signal low (clock stretching) for longer and that specified in the I2CCLKT register Cleared by writing 1 to the field.
#define BSC_S_ERR    		(1 << 8)	// RW | ACK Error. 0 = No errors detected. 1 = Slave has not acknowledged its address.
#define BSC_S_RXF    		(1 << 7)	// RO | FIFO Full. 0 = FIFO is not full. 1 = FIFO is full
#define BSC_S_TXE    		(1 << 6)	// RO | FIFO Empty. 0 = FIFO is not empty. 1 = FIFO is empty
#define BSC_S_RXD    		(1 << 5)	// RO | FIFO contains Data. 0 = FIFO is empty. 1 = FIFO contains at least 1 byte
#define BSC_S_TXD    		(1 << 4)	// RO | FIFO can accept Data. 0 = FIFO is full. The FIFO cannot accept more data. 1 = FIFO has space for at least 1 byte
#define BSC_S_RXR    		(1 << 3)	// RO | FIFO needs Reading ( full). 0 = FIFO is less than full and a read is underway. 1 = FIFO is or more full and a read is underway. Cleared by reading sufficient data from the FIFO.
#define BSC_S_TXW    		(1 << 2)	// RO | FIFO needs Writing ( full). 0 = FIFO is at least full and a write is underway (or sufficient data to send). 1 = FIFO is less then full and a write is underway. Cleared by writing sufficient data to the FIFO.
#define BSC_S_DONE   		(1 << 1)	// RW | Transfer Done. 0 = Transfer not completed. 1 = Transfer complete. Cleared by writing 1 to the field.
#define BSC_S_TA    		1			// RO | Transfer Active. 0 = Transfer not active. 1 = Transfer active.

#define CLEAR_STATUS    	BSC_S_CLKT|BSC_S_ERR|BSC_S_DONE

// Function prototypes
void dump_bsc_status();
void wait_i2c_done();
int init_i2c();
void close_i2c();

#endif
