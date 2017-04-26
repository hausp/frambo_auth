#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

void merlseson_doesnt_know_how_to_create_files(char* message, int size) {
    printk(KERN_INFO "FAZEDOR: Doing stuff with %zu characters\n", size);
}
