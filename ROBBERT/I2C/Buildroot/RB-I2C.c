#include "RB-I2C.h"

struct bcm2835_peripheral gpio = {GPIO_BASE};
struct bcm2835_peripheral bsc0 = {BSC0_BASE};

int main()
{

  printf("Done\n");
  
    /* Gain access to raspberry pi gpio and i2c peripherals */
    if(map_peripheral(&gpio) == -1) {
        printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
        return -1;
    }
    
	  else
	  {
		  printf("Mapping Complete\n");
	  }
	  
    if(map_peripheral(&bsc0) == -1) 
    {
        printf("Failed to map the physical BSC0 (I2C) registers into the virtual memory space.\n");
        return -1;
    }
 
    /* BSC0 is on GPIO 0 & 1 */
    i2c_init();
	printf("Init complete\n");
    /* I2C Device Address 0x51 (See Datasheet) */
    
    BSC0_A = 0x08;
 
    /* Write operation to restart the PCF8563 register at index 2 ('secs' field) */
    BSC0_DLEN = 1;            // one byte
    BSC0_FIFO = 1;
    //printf("%i\n",BSC0_FIFO);
    BSC0_S = CLEAR_STATUS;    // Reset status bits (see #define)
    BSC0_C = START_WRITE;     // Start Write (see #define)
 
    wait_i2c_done();
    
    BSC0_DLEN = 1;
    BSC0_S = CLEAR_STATUS;  // Reset status bits (see #define)
    BSC0_C = START_READ;    // Start Read after clearing FIFO (see #define)
 
    wait_i2c_done();
    
    dump_bsc_status();
    int response;
	//response = bcd_to_decimal(BSC0_FIFO);

	printf("%i\n",BSC0_FIFO);
	
	
	
	return 0;
}

// Exposes the physical address defined in the passed structure using mmap on /dev/mem
int map_peripheral(struct bcm2835_peripheral *p)
{
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

void dump_bsc_status() {

    unsigned int s = BSC0_S;

    printf("BSC0_S: ERR=%d  RXF=%d  TXE=%d  RXD=%d  TXD=%d  RXR=%d  TXW=%d  DONE=%d  TA=%d\n",
        (s & BSC_S_ERR) != 0,
        (s & BSC_S_RXF) != 0,
        (s & BSC_S_TXE) != 0,
        (s & BSC_S_RXD) != 0,
        (s & BSC_S_TXD) != 0,
        (s & BSC_S_RXR) != 0,
        (s & BSC_S_TXW) != 0,
        (s & BSC_S_DONE) != 0,
        (s & BSC_S_TA) != 0 );
}

// Function to wait for the I2C transaction to complete
void wait_i2c_done() {
        //Wait till done, let's use a timeout just in case
        int timeout = 500;
        while((!((BSC0_S) & BSC_S_DONE)) && --timeout) {
            usleep(1000);
        }
        if(timeout == 0)
            printf("wait_i2c_done() timeout. Something went wrong.\n");
}

void i2c_init()
{
    INP_GPIO(2);
    SET_GPIO_ALT(2, 0);
    INP_GPIO(3);
    SET_GPIO_ALT(3, 0);
}

// Priority

int SetProgramPriority(int priorityLevel)
{
    struct sched_param sched;

    memset (&sched, 0, sizeof(sched));

    if (priorityLevel > sched_get_priority_max (SCHED_RR))
        priorityLevel = sched_get_priority_max (SCHED_RR);

    sched.sched_priority = priorityLevel;

    return sched_setscheduler (0, SCHED_RR, &sched);
}

unsigned int bcd_to_decimal(unsigned int bcd) 
{
    return ((bcd & 0xf0) >> 4) * 10 + (bcd & 0x0f);
}
