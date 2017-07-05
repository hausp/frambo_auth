export ARCH:=arm
export CROSS_COMPILE:=arm-linux-gnueabihf-

KDIR :=../linux-rpi
PATH +=:../raspberry-tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin

NAME   :=raspchar
CC     :=arm-linux-gnueabihf-gcc

obj-m +=$(NAME).o
$(NAME)-objs :=char_device.o user_manager.o user_list.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
