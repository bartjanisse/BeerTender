/*
 * Program:
 *   gyro.c
 * AUTHOR: Dennis Jessurun
 * use "make" to build this module.
 * use "make install" to copy the gyro program to the Pi /bin directory
 * 
 * Datasheets:
 *   MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.0
 *       http://www6.in.tum.de/pub/Main/TeachingWs2015SeminarAuonomousFahren/RM-MPU-6000A.pdf
 * 
 *   MPU-6000 and MPU-6050 Product Specification Revision 3.3
 *       http://www.seeedstudio.com/wiki/images/b/b1/MPU6050.pdf
 * 
 * Accelero messurement registers:
 *   0x3B;    		ACCEL_XOUT[15:8]
 *   0x3C;			ACCEL_XOUT[7:0]
 *   0x3D;    		ACCEL_YOUT[15:8]
 *   0x3E;			ACCEL_YOUT[7:0]
 *   0x3F;    		ACCEL_ZOUT[15:8]
 *   0x40;			ACCEL_ZOUT[7:0]
 *   
 * Gyro messurement registers 
 *   0x43;    		GYRO_XOUT[15:8]
 *   0x44;			GYRO_XOUT[7:0]
 *   0x45;    		GYRO_YOUT[15:8]
 *   0x46;			GYRO_YOUT[7:0]
 *   0x47;    		GYRO_ZOUT[15:8]
 *   0x48;			GYRO_ZOUT[7:0]
 */

#include <time.h>
#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <assert.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

// MPU macros
//One of these addresses should work. Depends on module HW address
#define MPU6050_ADDR 	0x68	// reg 104 7 bit adres 
//#define MPU6050_ADDR 	0x69    // reg 105 
#define CONFIG 			0x1A	// reg 26
#define ACCEL_CONFIG 	0x1C	// reg 28
#define GYRO_CONFIG 	0x1B	// reg 27
#define PWR_MGMT_1 		0x6B	// reg 107
#define SELF_TEST		0x00

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
#define BSC_S_CLKT		(1 << 9)
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
#define I2C_SLV_DO		0x64

// GPIO setup macros.
// GPIO_SET: 	Get status of a Group of Pins (atomic). Reads the level of the given pins. 
// GPIO_CLEAR: 	Set a Group of Pins (atomic). Set one or more pins of the given GPIO port.
// GPIO_READ: 	Get status of a Group of Pins (atomic). Reads the level of the given pins.
#define INP_GPIO(g) 		*(gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) 		*(gpio.addr + ((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) 	*(gpio.addr + (((g)/10))) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET 			*(gpio.addr + 7)  // sets   bits which are 1 ignores bits which are 0 
#define GPIO_CLR 			*(gpio.addr + 10) // clears bits which are 1 ignores bits which are 0
#define GPIO_READ(g) 		*(gpio.addr + 13) &= (1<<(g))

// Global macros
#define ACCELEROMETER_SENSITIVITY 	8192.0	// Datasheet p.13 (6.1 gyro specifications)
#define GYROSCOPE_SENSITIVITY 		65.5	// Datasheet p.12 (6.2 accelero specifications)
#define filterConstant 				0.10 	// Complementary filter
#define PAGE_SIZE 		(4*1024)
#define BLOCK_SIZE 		(4*1024)

//#define RPI
#define RPI2

#ifdef RPI
#define BCM2708_PERI_BASE       0x20000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller 
#define BSC0_BASE 				(BCM2708_PERI_BASE + 0x205000)	// I2C controller	
#endif

#ifdef RPI2
#define BCM2708_PERI_BASE       0x3F000000
#define GPIO_BASE               (BCM2708_PERI_BASE + 0x200000)	// GPIO controller
#define BSC0_BASE 				(BCM2708_PERI_BASE + 0x804000)	// I2C controller	
#endif	

// IO Acces
struct bcm2835_peripheral {
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};
	
struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral bsc0 = {BSC0_BASE};

/**
 * wait_i2c_done() - Function to wait for the I2C transaction to complete
 */
void wait_i2c_done() {
	// Wait till done, lets use a timeout just in case
	int timeout = 50;
	while((!((BSC0_S) & BSC_S_DONE)) && --timeout) {
		usleep(1000);
	}
	if(timeout == 0)
		printf("wait_i2c_done() timeout. Something went wrong.\n");
}

/**
 * i2c_init() - Function to initialize the I2C pins
 */
void i2c_init() {  
	// Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)  
    INP_GPIO(2);
	SET_GPIO_ALT(2, 0);
	INP_GPIO(3);
	SET_GPIO_ALT(3, 0);
} 

// RPI
/**
 * map_peripheral() - Exposes the physical address defined in the passed structure using mmap on /dev/mem
 */
int map_peripheral(struct bcm2835_peripheral *p) {
   // Open /dev/mem
   if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("Failed to open /dev/mem, try checking permissions.\n");
      return -1;
   }

   p->map = mmap(
      NULL,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      p->mem_fd,  // File descriptor to physical memory virtual file '/dev/mem'
      p->addr_p      // Address in physical map that we want this memory block to expose
   );

   if (p->map == MAP_FAILED) {
        perror("mmap");
        return -1;
   }

   p->addr = (volatile unsigned int *)p->map;

   return 0;
}

void unmap_peripheral(struct bcm2835_peripheral *p) {

    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
}

// MPU
void MPU6050_SetRegister(unsigned char regAddr, unsigned char regValue)
{
    // See datasheet (PS) page 36: Single Byte Write Sequence

    // Master:   S  AD+W       RA       DATA       P
    // Slave :            ACK      ACK        ACK
	
    BSC0_A = MPU6050_ADDR;

    BSC0_DLEN = 2;
    BSC0_FIFO = (unsigned char)regAddr;
    BSC0_FIFO = (unsigned char)regValue;
	
    BSC0_S = CLEAR_STATUS; 		// Reset status bits (see #define)
    BSC0_C = START_WRITE;    	// Start Write (see #define)

    wait_i2c_done();
}

void MPU6050_Init() {
    //MPU6050_SetRegister(PWR_MGMT_1, 0x80);	// Device Reset
    MPU6050_SetRegister(PWR_MGMT_1, 0x00); 		// Clear sleep bit
    MPU6050_SetRegister(CONFIG, 0x00); 	
    MPU6050_SetRegister(GYRO_CONFIG, 0x08);
    MPU6050_SetRegister(ACCEL_CONFIG, 0x08);
}

void MPU6050_Read() {
    // See datasheet (PS) p.37: Burst Byte Read Sequence

    // Master:   S  AD+W       RA       S  AD+R           ACK        NACK  P
    // Slave :            ACK      ACK          ACK DATA       DATA

	short accData[3], gyrData[3];      	// Short is a 16 bit int!
	accData[0] = 0;
	accData[1] = 0;
	accData[2] = 0;
	gyrData[0] = 0;
	gyrData[1] = 0;
	gyrData[2] = 0;

    BSC0_DLEN = 1;    			// one byte
    BSC0_FIFO = 0x3B;    		// value of first register
    BSC0_S = CLEAR_STATUS; 		// Reset status bits (see #define)
    BSC0_C = START_WRITE;    	// Start Write (see #define)
    wait_i2c_done();
    BSC0_DLEN = 14;				// Read 14 registers of 1 byte 0x3B...0x48
    BSC0_S = CLEAR_STATUS;		// Reset status bits (see #define)
    BSC0_C = START_READ;    	// Start Read after clearing FIFO (see #define)
    wait_i2c_done();

    short tmp;
    int i = 0;	
    for(i=0; i<3; i++) 			// Gyroscope
    {
		tmp = BSC0_FIFO << 8;	
		tmp += BSC0_FIFO;
		gyrData[i] = tmp;
    }
    
    tmp = BSC0_FIFO << 8; 		// Temperature
    tmp += BSC0_FIFO;

    for(i=0; i<3; i++)			// Accelerometer
    {
		tmp = BSC0_FIFO << 8;
		tmp += BSC0_FIFO;
		accData[i] = tmp;
    }
    
    printf("gyr_XOUT: %.2f\t gyr_YOUT: %.2f\t gyr_ZOUT: %.2f\n", 
		((float)gyrData[0]/ACCELEROMETER_SENSITIVITY)*90, 
		((float)gyrData[1]/ACCELEROMETER_SENSITIVITY)*90, 
		((float)gyrData[2]/ACCELEROMETER_SENSITIVITY)*90);
	
	//printf("acc_XOUT: %.2f\t\t acc_YOUT: %.2f\t acc_ZOUT: %.2f\n", 
		//(float)accData[0]/GYROSCOPE_SENSITIVITY, 
		//(float)accData[1]/GYROSCOPE_SENSITIVITY, 
		//(float)accData[2]/GYROSCOPE_SENSITIVITY); 
	
	sleep(1);
    
    // Write data into file "gyro.dat" in order 
    // to save the data for analysing purposes.
    // Open with param "a" for appending data.
	FILE *fp;
	fp = fopen("gyro.dat", "a");
	if (fp == NULL) {
		printf("failed to open gyro.dat\n");
		exit(0);
	}	
	for(i=0; i<3; i++) {
		fprintf(fp, "gyro: %.2f\n", ((float)gyrData[i]/ACCELEROMETER_SENSITIVITY)*90);
	}	
	fclose(fp);
}

int main(int argc, char *argv[])
{
    // Check if the kernel module is loaded
    int fd;
	fd = open("/dev/gyromod", O_RDWR);
	if(fd < 0) {
		printf("Error opening file. Gyro module not loaded.\n");
		return(-1);
	}   
	else {     
		// Close file
		close(fd);
		
		// Global variables    
		short tmp;
		float time = 0;
		struct timespec tp;
		long startTime, procesTime;	  
		char filename[] = "gyro.dat";
		
		// Map memory areas			
		if(map_peripheral(&gpio) == -1) {
			printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
			return -1;
		}
		if(map_peripheral(&bsc0) == -1) {
			printf("Failed to map the physical BSC0 (I2C) registers into the virtual memory space.\n");
			return -1;
		}
		
		// Always remove "gyro.dat" when program is started.
		// This file is created when measurements are started. 
		remove(filename);
		printf("File \"%s\" successfully removed.\n", filename);
		
		/* BSC0 is on GPIO 2 & 3 */
		i2c_init();
			
		MPU6050_Init();
		printf("MPU6050 initialized.\n");
	
		while(1)
		{	
			// Get current time
			clock_gettime(CLOCK_REALTIME, &tp);
			startTime = tp.tv_sec*1000000000 + tp.tv_nsec;

			// Read MPU6050 sensor
			MPU6050_Read();		
		}
	}    
}

