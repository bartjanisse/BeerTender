/*
 * Author: Jeroen De Deken
 * Description: Module to make PID calculations
 * Date: 12-12-2015
 */

#ifndef _PIDMODULE_H_
#define _PIDMODULE_H_

#define DEVICE_NAME		"PIDController"
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
	// reset for the statics, set 1 to reset
	int reset;
};


#include <linux/ioctl.h>

#define DEVICE_FILE_NAME	"/dev/pid"
#define MAGIC_PATTERN		'P'

#define PID_SET _IOW(MAGIC_PATTERN, 0, int)
#define PID_GET _IOR(MAGIC_PATTERN, 1, int)

#define PID_ECHO _IOWR(MAGIC_PATTERN, 2, int)

int PIDcal(int SPn, int PVn);

#endif
