#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL v2");

#define AUTHENTICATE 0
#define ADD_ID 1

char** authorized_ids;

void process_command(int command) {
    switch (command) {
        case AUTHENTICATE:
            // TODO: authenticate
            break;
        case ADD_ID:
            
            break;
        default:
            printk(KERN_INFO "HAUSP: unknown command\n");
    }
    printk(KERN_INFO "HAUSP: Processing command\n");
}
