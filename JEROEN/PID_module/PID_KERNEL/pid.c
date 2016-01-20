/*
 * Author: Jeroen De Deken
 * Description: Module to make PID calculations
 * Date: 12-12-2015
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h> 

#include "pid.h"

static int Major;

static DEFINE_MUTEX(pid_mutex);


// the loop gain
static int Kc;
// the loop sample time
static int Ts;
// the integration period, set Ti very large to dissable the integrator part
static int Ti;
// the differation period
static int Td;
// integrator
static int MX;
// previous process value
static int PVnn;

static int initialised = 0;

/*
 * pid_ioctl() is used for communication with the module
 * 	options:
 * 		PID_RESET - is used to reset the integrator and differation
 * 		PID_SET - is used for setting the pid values
 * 		PID_GET - is used to do the calculations
 */
static long pid_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	
	switch (cmd)
	{
		case PID_RESET:
		{
			ResetPid();
			break;
		}
		
		case PID_SET:
		{
			struct PID pidsettings;
			
			if (copy_from_user(&pidsettings, (struct pidsettings*)arg, sizeof(pidsettings)))
			{
				return -EFAULT;
			}
			
			mutex_lock(&pid_mutex);
			
			printk(KERN_INFO "PID kc = %d\n", pidsettings.Kc);
			Kc = pidsettings.Kc;
			printk(KERN_INFO "PID Ts = %d\n", pidsettings.Ts);
			Ts = pidsettings.Ts;
			printk(KERN_INFO "PID Ti = %d\n", pidsettings.Ti);
			Ti = pidsettings.Ti;
			printk(KERN_INFO "PID Td = %d\n", pidsettings.Td);
			Td = pidsettings.Td;
			
			ResetPid();
			
			initialised = 1;
			
			mutex_unlock(&pid_mutex);
			
			break;
		}
		
		case PID_GET:
		{
			struct INPUT pidinput;
			
			if (initialised != 1)
			{
				printk(KERN_ALERT "pid controller is not initialised with values\n");
				return -EFAULT;
			}
			
			if (copy_from_user(&pidinput, (struct pidinput*)arg, sizeof(pidinput)))
			{
				return -EFAULT;
			}
			
			mutex_lock(&pid_mutex);
			
			printk(KERN_INFO "PID setpoint = %d PID value = %d\n", pidinput.setpoint, pidinput.processValue);
			
			ret = PIDcal(pidinput.setpoint, pidinput.processValue);
			
			mutex_unlock(&pid_mutex);
			
			break;
		}
		
		default:
			return -ENOTTY;
	}
	
	return ret;
}

static const struct file_operations pid_fops = {
	.owner   		= THIS_MODULE,
	.unlocked_ioctl = pid_ioctl
};


/*
 * init_module() is called when the module is loaded
 *
 */
int init_module(void)
{
	Major = register_chrdev(0, PID_DEVICE_NAME, &pid_fops);
	if (Major < 0)
	{
		printk(KERN_ALERT "Registering char device PID failed with %d\n", Major);
		return Major;
	}
	
	printk(KERN_INFO "Succesfully registered the %s module\n", PID_DEVICE_NAME);
	
	initialised = 0;
    
    return SUCCESS;
}

/*
 * cleanup_module() is called when the module is unloaded
 *
 */
void cleanup_module(void)
{
	unregister_chrdev(Major, PID_DEVICE_NAME);
	printk(KERN_INFO "Unregistered char device PID\n");
}

/*
 * PIDcal() is doing the calculation of the pid controller
 * 
 */
int PIDcal(int SPn, int PVn)
{
	int output, MPn, MIn, MDn;
	
	// proportional part
	MPn = Kc * (SPn - PVn);
	
	// integral part
	MIn = Kc * (Ts / Ti) * (SPn - PVn) + MX;
	MX = MIn;
	
	// differential part
	MDn = Kc * Td * ((PVnn - PVn) / Ts);
	PVnn = PVn;
	
	output = MPn + MIn + MDn;
	
	printk(KERN_INFO "MPn %d\tMIn %d\tMDn %d\tMX %d\tPVnn %d\toutput %d\n", MPn, MIn, MDn, MX, PVnn, output);

	return output;
}

/*
 * reset integrator and previous process value
 */
void ResetPid(void)
{
	printk(KERN_INFO "Reset the integrator and differation\n");
	MX = 0;
	PVnn = 0;
}
