/*
 * Author: Jeroen De Deken
 * Description: Module to make PID calculations
 * Date: 12-12-2015
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/delay.h>

#include "pid.h"

static int Major;
static int DeviceOpen = 0;
static char msg[BUF_LEN];
static char *msgPtr;

static int runCalculation = 0;
static int pidOutput;
static int pidSetpoint;
static int pidProcessVariable;

static struct file_operations fops = {
        .read    = device_read,
        .write   = device_write,
        .open    = device_open,
        .release = device_release
};

/*
 * init_module() is called when the module is loaded
 *
 */
int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);
	if (Major < 0)
	{
		printk(KERN_ALERT "Registering char device PID failed with %d\n", Major);
		return Major;
	}
	
	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a device file with\n");
        
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");
    
    return SUCCESS;
}

/*
 * cleanup_module() is called when the module is unloaded
 *
 */
void cleanup_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME);
	printk(KERN_INFO "Unregistered char device PID\n");
}

/*
 * device_open() is called when a process tries to open the device file
 *
 */
 static int device_open(struct inode *inode, struct file *file);
 {
	 if (DeviceOpen)
	 {
		 return -EBUSY;
	 }
	 
	 DeviceOpen++;
	 
	 msgPtr = msg;
	 
	 try_module_get(THIS_MODULE);
	 
	 runCalculation = 1;
	 calculationLoop();
	 
	 return SUCCESS;
 }
 
 /*
  * device_release() is called when a process closes the device file
  * 
  */
static int device_release(struct inode *inode, struct file *file)
{
	DeviceOpen--;
	
	runCalculation = 0;
	
	module_put(THIS_MODULE);
	
	return SUCCESS;
}

/*
 * device_read() is called when a process which opened the file attempts
 * to read from it
 * 
 */
 static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
 {
	 put_user(pidOutput, buffer);
 }
 
/*
 * device_write() is called when a process writes to the device file
 * 
 */
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
	// read the data
	int i;
	for(i=0; i<len && i<BUF_LEN; i++)
	{
		get_user(msg[i], buff+i);
	}
	msg_Ptr = msg;
	
	
}

/*
 * calculationLoop() is running the pid calculation at sample times
 * 
 */
void calculationLoop(void)
{
	pidSetpoint = 0;
	pidProcessVariable = 0;
	while(runCalculation)
	{
		pidOutput = PIDcal(pidSetpoint, pidProcessVariable);
		
		// wait for the sample time
		msleep(Ts);
	}
}

/*
 * PIDcal() is doing the calculation of the pid controller
 * 
 */
int PIDcal(int SPn, int PVn)
{
	static int MX = 0;
	static int PVnn = 0;
	int output, MPn, MIn, MDn;
	
	// proportional part
	MPn = Kc * (SPn - PVn);
	
	// integral part
	MIn = Kc * (Ts / Ti) * (SPn - PVn) + MX;
	MX = MIn;
	
	// differential part
	MDn = Kc * (Td / Ts) * (PVnn - PVn);
	PVnn = PVn;
	
	output = MPn + MIn + MDn;

	return output;
}
 
 
 
 
 
 
 
