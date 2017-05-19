export ARCH:=arm
export CROSS_COMPILE:=arm-linux-gnueabihf-

KDIR :=../linux-rpi
PATH +=:../raspberry-tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin

NAME   :=hausp
CC     :=arm-linux-gnueabihf-gcc
PAMDIR :=pam_auth
PAMSO  :=$(patsubst $(PAMDIR)/%.c,%.so,$(wildcard $(PAMDIR)/*.c))

obj-m +=$(NAME).o
$(NAME)-objs :=char_device.o pam_wrapper.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

pam: $(PAMSO)

%.so: $(PAMDIR)/%.c
	$(CC) -fPIC -DPIC -shared -rdynamic -I/usr/include -o $@ $^

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
