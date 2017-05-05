ARCH:=arm
CROSS_COMPILE:=arm-linux-gnueabihf-
tools_prefix :=/home/thiagobbt/Stuff/raspberry-tools
KDIR :=/home/thiagobbt/Stuff/linux-rpi
PWD :=$(shell pwd)
PATH +=:$(tools_prefix)/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin

module_name := hausp

obj-m += $(module_name).o
$(module_name)-objs := char_device.o pam_wrapper.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules ARCH:=$(ARCH) CROSS_COMPILE:=$(CROSS_COMPILE)
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
