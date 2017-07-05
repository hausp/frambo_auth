#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define  DEVICE_NAME "raspchar"
#define  CLASS_NAME  "rasp"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("HAUSP Co.");
MODULE_DESCRIPTION("A simple authentication module");
MODULE_VERSION("0.1d");

#define BUF_SIZE 256

extern int process_command(char*);
extern int pam_init(void);

static int            majorNumber;
static char           message[BUF_SIZE] = {0};
static ssize_t        size_of_message;
static struct class*  raspcharClass  = NULL;
static struct         device* raspcharDevice = NULL;
static int            authStatus = 1;

static int     dev_open(struct inode*, struct file*);
static int     dev_release(struct inode*, struct file*);
static ssize_t dev_read(struct file*, char*, size_t, loff_t*);
static ssize_t dev_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init raspchar_init(void) {
    printk(KERN_INFO "HAUSP: Initializing the HAUSP LKM\n");

    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "HAUSP failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "HAUSP: registered correctly with major number %d\n", majorNumber);

    raspcharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(raspcharClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(raspcharClass);
    }
    printk(KERN_INFO "HAUSP: device class registered correctly\n");

    raspcharDevice = device_create(raspcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(raspcharDevice)) {
        class_destroy(raspcharClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(raspcharDevice);
    }

    printk(KERN_INFO "HAUSP: device class created correctly\n");
    pam_init();

    return 0;
}

static void __exit raspchar_exit(void) {
    device_destroy(raspcharClass, MKDEV(majorNumber, 0));
    class_unregister(raspcharClass);
    class_destroy(raspcharClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "HAUSP: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    char* auth_message;

    if (*offset != 0) {
        return 0;
    }

    if (authStatus == 0) {
        auth_message = "OK";
        size_of_message = 2;
    } else {
        auth_message = "FAIL";
        size_of_message = 4;
    }

    error_count = copy_to_user(buffer, auth_message, size_of_message);

    authStatus = 1;

    if (len < size_of_message) {
        return -EINVAL;
    }

    if (error_count == 0) {
        *offset += len;
        return len;
    } else {
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    len = (len > BUF_SIZE - 1) ? BUF_SIZE - 1: len;
    sprintf(message, "%.*s", len, buffer);
    size_of_message = strlen(message);
    authStatus = process_command(message);
    return len;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    return 0;
}

module_init(raspchar_init);
module_exit(raspchar_exit);
