CCPREFIX=/home/student/buildroot_lin_ver/tools-master/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
KERNEL_SRC=/home/student/buildroot_lin_ver/linux-rpi-3.18.16-v7

obj-m += gyromod.o

all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SRC} M=$(PWD) modules 
	
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean	
	
install:
	scp gyromod.ko gyroscript gyro_m root@192.168.123.42:/bin