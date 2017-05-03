#include <linux/kernel.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL v2");

void process_command(char* command, int size) {
    printk(KERN_INFO "HAUSP: Processing command\n");
}
