/*
 * Author: Jeroen De Deken
 * Description: Module to make PID calculations
 * Date: 12-12-2015
 */

#ifndef _PIDMODULE_H_
#define _PIDMODULE_H_

#define DEVICE_NAME "PDController"
#define BUF_LEN 	6
#define SUCCESS 	0


// the loop gain
#define Kc 2
// the loop sample time
#define Ts 100
// the integration period, set Ti very large to dissable the integrator part
#define Ti 1000000000000000000
// the differation period
#define Td 10

/*
 * functions 
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

void calculationLoop(void);
int PIDcal(int SPn, int PVn);

#endif
