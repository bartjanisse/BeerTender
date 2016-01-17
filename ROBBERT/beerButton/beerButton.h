#ifndef _BUTTONS_H_
#define _BUTTONS_H_

#define DEVICE_NAME "beerButton"	/* Device name as it appears in /proc/devices   */
#define BUF_LEN 	2			/* Max length of the message from the device */
#define SUCCESS 	0
#define GPIO_LED	14			/* GPIO 14 is connected to pen 8 of the Raspberry connector */
#define GPIO_KEY	2           // Pin 2 used because of the pull up resistor

/* 
 * Function prototypes
 */

int checkGpioThread_fn(void *);
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#endif
