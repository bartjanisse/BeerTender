/*
 * 
 * blink.c: Basic program to work with the helloled3 module3. This program
 * writes in a continues loop 1's and 0's to the device file /dev/helloled3.
 * 
 */ 
#include <stdio.h>
#include <fcntl.h>

int main()
{
	int fd;

	fd = open("/dev/helloled3", O_RDWR);
	if(fd < 0)
    {
		printf("Error opening file\n");
		return(-1);
	}
	
	while(1)
	{
		printf("Put LED on\n");
		write(fd, "1", 1);
		sleep (1);
		
		printf("Put LED off\n");
		write(fd, "0", 1);
		sleep(1);
	}	
}
