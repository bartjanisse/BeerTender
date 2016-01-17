/*
 * Author: Jeroen De Deken
 * Description: Module to make PID calculations
 * Date: 12-12-2015
 */

#ifndef _PIDMODULE_H_
#define _PIDMODULE_H_

#define PID_DEVICE_NAME		"PIDController"
#define SUCCESS 		0


struct PID {
	// the loop gain
	int Kc;
	// the loop sample time
	int Ts;
	// the integration period, set Ti very large to dissable the integrator part
	int Ti;
	// the differation period
	int Td;
};

struct INPUT {
	// the setpoint for the pid
	int setpoint;
	// the processvalue for the pid
	int processValue;
};


#include <linux/ioctl.h>

#define PID_DEVICE_FILE_NAME	"/dev/pid"
#define MAGIC_PATTERN		'P'

// change settings for the pid module
#define PID_SET _IOW(MAGIC_PATTERN, 0, int)
// execute a calculation
#define PID_GET _IOR(MAGIC_PATTERN, 1, int)
// reset the integrator and differation
#define PID_RESET _IO(MAGIC_PATTERN, 2)

int PIDcal(int SPn, int PVn);
void ResetPid(void);

#endif
