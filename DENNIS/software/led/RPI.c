#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <assert.h>
#include <unistd.h>
 
 
#define BCM2835_PERI_BASE   0x20000000 //Rpi2 use: 0x3F000000
#define GPIO_BASE           (BCM2835_PERI_BASE + 0x200000)	// GPIO controller  
#define PAGE_SIZE 			(4*1024)
#define BLOCK_SIZE 			(4*1024)

// IO Acces
struct bcm2835_peripheral {
    unsigned long addr_p;
    int mem_fd;
    void *map;
    volatile unsigned int *addr;
};

struct bcm2835_peripheral gpio = {GPIO_BASE};

// I/O access
//volatile unsigned int *gpio = GPIO_BASE;
 
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x)
#define INP_GPIO(g)   *(gpio.addr + ((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g)   *(gpio.addr + ((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio.addr + (((g)/10))) |= (((a)<=3?(a) + 4:(a)==4?3:2)<<(((g)%10)*3))
 
#define GPIO_SET  *(gpio.addr + 7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR  *(gpio.addr + 10) // clears bits which are 1 ignores bits which are 0
 
#define GPIO_READ(g)  *(gpio.addr + 13) &= (1<<(g))

 
// Exposes the physical address defined in the passed structure using mmap on /dev/mem
int map_peripheral(struct bcm2835_peripheral *p)
{	
   // Open /dev/mem
	if ((p->mem_fd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0) {
      printf("Failed to open /dev/mem, try checking permissions.\n");
      exit (-1);
	}
 
	//printf("mapping..\n");
	
	p->map = mmap(
      NULL,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      p->mem_fd,      // File descriptor to physical memory virtual file '/dev/mem'
      p->addr_p       // Address in physical map that we want this memory block to expose
	);

	if (p->map == MAP_FAILED) {
        perror("mmap");
        return -1;
	}	
	
	p->addr = (volatile unsigned int *)p->map;
	
	//printf("address: %d\n", p->addr);
	
	return 0;	
}

void unmap_peripheral(struct bcm2835_peripheral *p) 
{ 
    munmap(p->map, BLOCK_SIZE);
    close(p->mem_fd);
}

int main(void)
{
	printf("start\n");
	
	if(map_peripheral(&gpio) == -1) 
	{
		printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
		return -1;
	}
 
	// Define pin 7 as output
	INP_GPIO(4);
	OUT_GPIO(4);
 
	while(1)
	{
		// Toggle pin 7 (blink a led!)
		GPIO_SET = 1 << 4;
		sleep(1);
		//GPIO_READ(4);
		GPIO_CLR = 1 << 4;
		sleep(1);
	}
	
	return (0);
}
