CFLAGS_helloworld.o = -DDEBUG

obj-m += char_device.o

# KDIR ?= /lib/modules/$(shell uname -r)/build
KDIR :=/home/aszdrick/ufsc/playground/linux
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
