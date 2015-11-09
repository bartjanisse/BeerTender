/*
 * 11-10-2015 - Bart Janisse
 */
 
#ifndef _INT2_H_
#define _INT2_H_

#define DEVICE_NAME 	"int2"		/* Device name as it appears in /proc/devices   */
#define BUF_LEN 		2			/* Max length of the message from the device */
#define SUCCESS 		0
#define GPIO_LED		14			/* GPIO 14 is connected to pen 8 of the Raspberry connector */
#define GPIO_INPUT		23			/* GPIO 23 is connected to pen 16 of the Raspberry connector */

// text below will be seen in 'cat /proc/interrupts' command
#define GPIO_ANY_GPIO_DESC           "GPIO interrupt"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("A.W. Janisse");
MODULE_DESCRIPTION("A Simple Character Device Driver module for demonstrating interrupts");

/* 
 * Function prototypes
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#endif
