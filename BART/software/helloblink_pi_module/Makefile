
CCPREFIX=/home/student/buildroot_lin_ver/tools/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
KERNEL_SRC=/home/student/buildroot_lin_ver/linux-rpi-3.18.16-v7


obj-m += blinkpimodule.o
blinkpimodule-objs := blink_pi_module.o RPI.o RPI_GPIO.o

#RPI.o RPI_GPIO.o


all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SRC} M=$(PWD) modules
	
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean	
	
