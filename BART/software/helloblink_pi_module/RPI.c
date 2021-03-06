
#include "RPI.h"
//#include "utils.h"


// Exposes the physical address defined in the passed structure using mmap on /dev/mem
int map_peripheral(struct bcm_peripheral *p)
{
	// Open /dev/mem
	if ((p->mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) 
	{
		//P_ERR("Failed to open /dev/mem, try checking permissions.");
		return (-1);
	}

	p->map = mmap(
		NULL,
		B_SIZE,
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		p->mem_fd,  		// File descriptor to physical memory virtual file '/dev/mem'
		p->addr_p      		// Address in physical map that we want this memory block to expose
	);

	if (p->map == MAP_FAILED) 
	{
		//P_ERR("mmap failed");
        return (-1);
	}

	p->addr = (volatile unsigned int *)p->map;

	return (0);
}

void unmap_peripheral(struct bcm_peripheral *p) 
{
    munmap(p->map, B_SIZE);
    close(p->mem_fd);
}
