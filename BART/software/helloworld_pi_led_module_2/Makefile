
CCPREFIX=/home/student/buildroot_lin_ver/tools-master/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
KERNEL_SRC=/home/student/buildroot_lin_ver/linux-rpi-3.18.16-v7


obj-m += helloled3.o

all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SRC} M=$(PWD) modules 
	
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean	
	
install:
	scp helloled3.ko led3 root@192.168.123.42:/bin
