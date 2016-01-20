# Description:
The goal of this code is to make a PID calculation module for the raspberry pi2
The calculation of the pid is based on Siemens S7-200 System Manual, page 146
http://www1.siemens.cz/ad/current/content/data_files/automatizacni_systemy/mikrosystemy/simatic_s7200/manual_s7_200_2005_en.pdf


# Compilation:
To cross compile this module for the raspberry pi first make sure you have the specific kernel build on your system.
See the Readme file in folder Bart/Software/helloworld_pi_module on how to do this.

Use make to build the module. Adapt the makefile where necessary (path to the correct folder? correct kernel?)
After making the objects copy the pid.ko to the pi


# Usage:
To start the module:
$ insmod pid.ko
$ mknod /dev/pid c 247 0
The module is now started. Check the log file to see the kernel messages
$ tail /var/log/messages

To stop the module:
$ rmmod pid.ko
$ rm /dev/pid

Communication with the module can be done by using ioctl to the device file /dev/pid
3 controls are implemented.
	- PID_SET to initialize the pid with values from a struct
	- PID_RESET to reset the integrator and differation
	- PID_GET to do a calculation

2 structs are used to communicate with the module. Include the .h file to make use of these structs.

struct PID is used to set the initial values for the pid calculator
	Kc -> the loop gain
	Ts -> the loop sample time
	Ti -> the integration period (set very large to dissable integration)
	Td -> the differation period (set to zero to dissable differation)

struct INPUT is used to do the calculations
	setpoint -> the setpoint for the pid
	processValue -> the process value for the pid

See the testpid.c file for details about the usage of the pid module.
