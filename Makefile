ARCH:=arm
CROSS_COMPILE:=arm-linux-gnueabihf-
# KDIR ?= /lib/modules/$(shell uname -r)/build
KDIR :=/home/aszdrick/ufsc/playground/linux
PWD := $(shell pwd)

module_name := hausp

obj-m += $(module_name).o
$(module_name)-objs := char_device.o pam_wrapper.o

default:
	@export PATH=$PATH:$HOME/ufsc/playground/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
	$(MAKE) -C $(KDIR) M=$(PWD) modules ARCH:=$(ARCH) CROSS_COMPILE:=$(CROSS_COMPILE)
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
