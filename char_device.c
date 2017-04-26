#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define  DEVICE_NAME "raspchar"
#define  CLASS_NAME  "rasp"

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("HAUSP Co.");
MODULE_DESCRIPTION("A simple Linux char driver");
MODULE_VERSION("0.1d");

#define BUF_SIZE 256

// forward declarations
extern void process_command(char*, int);

static int    majorNumber;
static char   message[BUF_SIZE] = {0};
static ssize_t  size_of_message;
static int    numberOpens = 0;
static struct class*  raspcharClass  = NULL;
static struct device* raspcharDevice = NULL;
 
// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
 
/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};
 
/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init raspchar_init(void) {
    printk(KERN_INFO "HAUSP: Initializing the HAUSP LKM\n");
 
    // Try to dynamically allocate a major number for the device -- more difficult but worth it
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "HAUSP failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "HAUSP: registered correctly with major number %d\n", majorNumber);
 
    // Register the device class
    raspcharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(raspcharClass)) {                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(raspcharClass);
    }
    printk(KERN_INFO "HAUSP: device class registered correctly\n");
 
    // Register the device driver
    raspcharDevice = device_create(raspcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(raspcharDevice)) {               // Clean up if there is an error
        class_destroy(raspcharClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(raspcharDevice);
    }
    printk(KERN_INFO "HAUSP: device class created correctly\n");
    return 0;
}
 
/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit raspchar_exit(void) {
    device_destroy(raspcharClass, MKDEV(majorNumber, 0));
    class_unregister(raspcharClass);
    class_destroy(raspcharClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "HAUSP: Goodbye from the LKM!\n");
}
 
/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep) {
    numberOpens++;
    printk(KERN_INFO "HAUSP: Device has been opened %d time(s)\n", numberOpens);
    return 0;
}
 
/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
    if (*offset != 0) {
        return 0;
    }

    error_count = copy_to_user(buffer, message, size_of_message);

    if (len < size_of_message) {
        return -EINVAL;
    }
 
    // if true then have success
    if (error_count == 0) {
        printk(KERN_INFO "HAUSP: Sent %d characters to the user\n", size_of_message);
        *offset += len;
        return len;
    } else {
        printk(KERN_INFO "HAUSP: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }
}
 
/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    // int original = len;
    len = (len > BUF_SIZE - 1) ? BUF_SIZE - 1: len;
    sprintf(message, "%.*s\n", len, buffer);
    size_of_message = strlen(message);
    printk(KERN_INFO "HAUSP: Received %zu characters from the user\n", len);
    process_command(message, len);
    return len;
}
 
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "HAUSP: Device successfully closed\n");
    return 0;
}
 
/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(raspchar_init);
module_exit(raspchar_exit);