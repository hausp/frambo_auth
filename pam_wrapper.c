#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

void process_command(char* command, int size) {
    printk(KERN_INFO "HAUSP: Processing command\n");
}