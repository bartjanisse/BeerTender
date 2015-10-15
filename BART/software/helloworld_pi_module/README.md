# How to cross compile a module for the Raspberry Pi 2

To build modules we need to compile the linux kernel. The kernel we need to compile is the specific one which runs on the Raspberry Pi.
## Necessities
There following things we need for building our kernel.
-   linux-rpi-3.18.y.tar.gz
-   tools

Each of those will be descibed in one of the following steps.

##### Step 1. Find the kernel version
On the Raspberry Pi invoke the following command:
```sh
$ uname -r
```
the output wil show the version we need. In this example I will be using **3.18.16-v7**

##### Step 2. Creating the directories
Since we need a place to put all our necessities we create a directory. In this example the following I will be using  
```sh
~/buildroot_lin_ver
```
##### Step 3. Finding our linux-rpi-3.18.y.tar.gz
The file we are looking for should be located in the following directory
`$ ~/buildroot/dl`. Copy the file `linux-rpi-3.18.y.tar.gz` from here to our newly created `buildroot_lin_ver` folder.

Next we need to extraxt this file. This can be simply done in the File Manager by right clicking the file and choose 'Extract Here'. 

After this is done you should end up with a folder named `~/buildroot_lin_ver/linux-rpi-3.18.y` in which all the files are extracted. 

Since we are going to build for Linux version **3.18.16-v7** we going to rename our folder to `~/buildroot_lin_ver/linux-rpi-3.18.16-v7`

##### Step 4. Get the tools
The tools we need can be downloaded from
*https://github.com/raspberrypi/tools* 

Place the downloade file in your `~/buildroot_lin_ver` folder. Extract it as we did in step 3.

After this is done you should end up with a folder named
'`~/buildroot_lin_ver/tools-master` in which the files are extracted.

##### Step 5. Setting some environment variables
Since we don't want to type the complete paths over and over again we set, for our convenience, some environment variable.

On your linux machine invoke the following commands
```sh
$ export CCPREFIX=~/buildroot_lin_ver/tools-master/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
```
```sh
$ export KERNEL_SRC=~/buildroot_lin_ver/linux-rpi-3.18.16-v7
```
##### Step 6. Cleaning up the kernel source tree
To ensure you have a clean kernel source tree execute the following
```sh
$ cd $KERNEL_SRC
$ make mrproper
```
##### Step 7. Get the .config file
Since we want to build the kernel exactly as it was done for the Raspberry Pi we need to get the .config file. Luckily it is available on the running Raspberry pi so we just can copy it. Make sure you get back in your `~/buildroot_lin_ver` folder and execute the following
```sh
$ scp root@192.168.123.42:/proc/config.gz ./
```
you should end up with this file on your Linux machin. Now extract it with
```sh
$ zcat config.gz > .config
```
This creates the wanted `.config` file. The last thing we need to do is move it to the right folder. For this execute
```sh
$ mv .config $KERNEL_SRC
```
##### Step 8. Rebuilding the kernel
Building the kernel involves the following two steps
```sh
$ cd $KERNEL_SRC
$ make ARCH=arm CROSS_COMPILE=${CCPREFIX} oldconfig
$ make ARCH=arm CROSS_COMPILE=${CCPREFIX}
```
The steps will produce a fresh `Module.symvers' and since you are using the cross compile tools, the intermediates the tool generate should be binary compatible with your Raspberry Pi.

## Compiling a module
To compile your kernel module you need a Makefile. The following is an example of this file in where all the above mentioned directories are included.
```sh
CCPREFIX=/home/student/buildroot_lin_ver/tools-master/arm-bcm2708/arm-bcm2708-linux-gnueabi/bin/arm-bcm2708-linux-gnueabi-
KERNEL_SRC=/home/student/buildroot_lin_ver/linux-rpi-3.18.y

obj-m += hello-1.o

all:
	make ARCH=arm CROSS_COMPILE=${CCPREFIX} -C ${KERNEL_SRC} M=$(PWD) modules
	
clean:
	make -C ${KERNEL_SRC} M=$(PWD) clean
```
When you build the module, in this case `hello-1` you should end up whith a module which will run on your Raspberry Pi. Take a look a what `modinfo` produces on the `hello-1.ko` module
```sh
$ modinfo hell*.ko
filename:       /home/student/bart/programming/school/prt/BeerTender/BART/software/helloworld_pi_module/hello-1.ko
srcversion:     250CAA7804A96E6BDF8D135
depends:        
vermagic:       3.18.16-v7 SMP preempt mod_unload modversions ARMv7 
```
 





