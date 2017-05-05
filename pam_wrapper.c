#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL v2");

#define ID_SIZE 4
#define MAX_AUTHORIZED_IDS 50

char* authorized_ids[MAX_AUTHORIZED_IDS];
int registered_ids = 0;

int decompose(char* full_command, char* command, char* id) {
	int size = strlen(command);
	memcpy(id, &full_command[size + 1], ID_SIZE - 1);
	id[ID_SIZE - 1] = '\0';
	return strncmp(full_command, command, size) == 0;
}

int search(char* id) {
	int i;
	for (i = 0; i < registered_ids; i++) {
		printk(KERN_INFO "HAUSP: Comparing to %s\n", authorized_ids[i]);
		if (strcmp(authorized_ids[i], id) == 0) {
			return i;
		}
	}
	return -1;
}

int process_command(char* command) {
	char id[ID_SIZE];
	int i, index;
	printk(KERN_INFO "HAUSP: Processing command\n");
	if (decompose(command, "AUTH", id)) {
		printk(KERN_INFO "HAUSP: id = %s\n", id);
		if (search(id) >= 0) {
			printk(KERN_INFO "HAUSP: Authorized\n");
			return 0;
		} else {
			printk(KERN_INFO "HAUSP: Police called!\n");
			return 1;
		}
	} else if (decompose(command, "ADD", id)) {
		index = search(id);
		if (index == -1 && registered_ids < MAX_AUTHORIZED_IDS) {
			authorized_ids[registered_ids] = kmalloc(sizeof(char) * ID_SIZE, GFP_KERNEL);
			memcpy(authorized_ids[registered_ids++], &id, ID_SIZE);
			printk(KERN_INFO "HAUSP: %s added\n", id);
		} else if (index != -1) {
			printk(KERN_INFO "HAUSP: Duplicate ID\n");
		} else {
			printk(KERN_INFO "HAUSP: Max number of IDs reached\n");
		}
	} else if (decompose(command, "REMOVE", id)) {
		index = search(id);
		if (index >= 0) {
			kfree(authorized_ids[index]);
			for (i = index; i < MAX_AUTHORIZED_IDS; i++) {
				authorized_ids[i] = authorized_ids[i + 1];
			}
			--registered_ids;
		}
	}
	return 2;
}
