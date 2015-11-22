/*
 * 08-11-2015 - Bart Janisse
 */
 
#ifndef _INT1_H_
#define _INT1_H_

#define DEVICE_NAME 		"int1"				/* Device name as it appears in /proc/devices   */
#define GPIO_ANY_GPIO_DESC	"GPIO 23 interrupt"	/* text below will be seen in 'cat /proc/interrupts' */

#define BUF_LEN 		2						/* Max length of the message from the device */
#define GPIO_LED		14						/* GPIO 14 is connected to pen 8 of the Raspberry connector */
#define GPIO_INPUT		23						/* GPIO 23 is connected to pen 16 of the Raspberry connector */

#define SUCCESS 		0

MODULE_LICENSE("GPL");
MODULE_AUTHOR("A.W. Janisse");
MODULE_DESCRIPTION("A Simple Character Device Driver module for demonstrating interrupts");

/* 
 * Function prototypes
 */
int init_module(void);
void cleanup_module(void);

#endif
