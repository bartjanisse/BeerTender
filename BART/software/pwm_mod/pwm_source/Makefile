# 
#	Makefile for pwm module.
#
#	(C) 2015, A.W. Janisse
#
#	Macro's:
#		APP 	: Name of the module.
#		OBJS	: List of objects needed to build this module.
#		PIUSR	: Raspberry PI user name. Used during install.
#		PIIP	: Raspberry PI IP address. Used during install.
#		PIDIR   : Directory name for installation on the Raspberry PI.
#		UTILS	: List of utilities which will be copied. Used during install.

APP	= pwm
OBJS	= main.o
PIUSR	= root
PIIP	= 192.168.123.42
PIDIR	= /bin
UTILS	= pwm_load pwm_unload

# Comiler prefix for cross compilation.
CCPREFIX=~/buildroot_lin_ver/tools-master/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
# Linux source for cross compilation.
KERNEL_SRC=~/buildroot_lin_ver/linux-rpi-3.18.16-v7

### -----[ Do not change anything below this line ]----- ###

pwm-objs := $(OBJS)
obj-m := $(APP).o

all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SRC} M=$(PWD) modules 
	
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean	
	
install:
	scp $(APP).ko $(UTILS) $(PIUSR)@$(PIIP):$(PIDIR)
