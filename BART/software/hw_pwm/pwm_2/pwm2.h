/*
 * 11-10-2015 - Bart Janisse
 */
 
#ifndef _PWM2_H_
#define _PWM2_H_

MODULE_LICENSE("GPL");
MODULE_AUTHOR("A.W. Janisse");
MODULE_DESCRIPTION("PWM channel 1 and 2 proof of concept");

#define DEVICE_NAME 	"pwm1"		/* Device name as it appears in /proc/devices   */
#define BUF_LEN 		2			/* Max length of the message from the device */
#define SUCCESS 		0
#define GPIO_LED		14			/* GPIO 14 is connected to pen 8 of the Raspberry connector */
#define GPIO_INPUT		23			/* GPIO 23 is connected to pen 16 of the Raspberry connector */
#define GPIO_PWM1		13 			// 
#define GPIO_ALT		0

// text below will be seen in 'cat /proc/interrupts' command
#define GPIO_ANY_GPIO_DESC           "PWM channel 0 int"


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
