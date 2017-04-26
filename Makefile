ARCH:=arm
CROSS_COMPILE:=arm-linux-gnueabihf-
# KDIR ?= /lib/modules/$(shell uname -r)/build
KDIR :=/home/aszdrick/ufsc/playground/linux
PWD := $(shell pwd)

module_name := char_device

obj-m += $(module_name).o
$(module_name)-objs := pam_wrapper.o

default:
	@echo $($(module_name)-objs)
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
