#ifndef _INC_MPU6050_H
#define _INC_MPU6050_H

#include "PJ_RPI.h"

#define MPU6050_ADDR 	0x68	// 7 bit adres 
//#define MPU6050_ADDR 0x69     //One of these address should work. Depends on module HW address
#define CONFIG 			0x1A
#define ACCEL_CONFIG 	0x1C
#define GYRO_CONFIG 	0x1B
#define PWR_MGMT_1 		0x6B

// I2C macros
#define BSC0_C        	*(bsc0.addr + 0x00)
#define BSC0_S        	*(bsc0.addr + 0x01)
#define BSC0_DLEN    	*(bsc0.addr + 0x02)
#define BSC0_A        	*(bsc0.addr + 0x03)
#define BSC0_FIFO    	*(bsc0.addr + 0x04)

#define BSC_C_I2CEN    	(1 << 15)
#define BSC_C_INTR    	(1 << 10)
#define BSC_C_INTT    	(1 << 9)
#define BSC_C_INTD    	(1 << 8)
#define BSC_C_ST    	(1 << 7)
#define BSC_C_CLEAR    	(1 << 4)
#define BSC_C_READ    	1

#define START_READ    	BSC_C_I2CEN|BSC_C_ST|BSC_C_CLEAR|BSC_C_READ
#define START_WRITE   	BSC_C_I2CEN|BSC_C_ST

#define BSC_S_CLKT	(1 << 9)
#define BSC_S_ERR    	(1 << 8)
#define BSC_S_RXF    	(1 << 7)
#define BSC_S_TXE    	(1 << 6)
#define BSC_S_RXD    	(1 << 5)
#define BSC_S_TXD    	(1 << 4)
#define BSC_S_RXR    	(1 << 3)
#define BSC_S_TXW    	(1 << 2)
#define BSC_S_DONE   	(1 << 1)
#define BSC_S_TA    	1

#define CLEAR_STATUS    BSC_S_CLKT|BSC_S_ERR|BSC_S_DONE

void MPU6050_Read(short * accData, short * gyrData);
void MPU6050_Init(void);
void MPU6050_SetRegister(unsigned char regAddr, unsigned char regValue);

#endif
