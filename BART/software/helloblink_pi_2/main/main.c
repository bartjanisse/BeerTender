
#include <stdio.h>
#include <unistd.h>

extern void PUT32 ( unsigned int, unsigned int );
extern unsigned int GET32 ( unsigned int );
extern void dummy ( unsigned int );

#define BIT					12 // GPIO 14 is pin 8

#define BCM2708_PERI_BASE       0x3F000000

#define GPFSEL0 (BCM2708_PERI_BASE + 0x20200000)
#define GPFSEL1 (BCM2708_PERI_BASE + 0x20200004)
#define GPSET0  (BCM2708_PERI_BASE + 0x2020001C)
#define GPCLR0  (BCM2708_PERI_BASE + 0x20200028)


int 
main()
{
	unsigned int ra;
	printf("Starting Helloblink 2");
	//printf("Starting Helloblink 2 with GIOP%d at pin number 8.\n", LED_PIN);
	
	// Initialize 
    ra = GET32(GPFSEL0);
//    ra &= ~(7<<BIT);			
//    ra |= 1<<BIT;
//    PUT32(GPFSEL0,ra);

/*	while(1)
	{
		PUT32(GPSET0,1<<BIT);
		usleep(100 * 1000);
	
		PUT32(GPCLR0,1<<BIT);
		usleep(100 * 1000);
	}
*/
	return 0;	
}
