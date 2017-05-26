#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/uaccess.h>

#include "user_list.h"

MODULE_LICENSE("GPL v2");

char* users[MAX_AUTHORIZED_USERS];
int registered_users = 0;

int decompose(char* full_command, char* command, char* user) {
	int size = strlen(command);
	memcpy(user, &full_command[size + 1], USER_SIZE - 1);
	user[USER_SIZE - 1] = '\0';
	return strncmp(full_command, command, size) == 0;
}

int search(char* user) {
	int i;
	for (i = 0; i < registered_users; i++) {
		printk(KERN_INFO "HAUSP: Comparing to %s\n", users[i]);
		if (strcmp(users[i], user) == 0) {
			return i;
		}
	}
	return -1;
}

int process_command(char* command) {
	char user[USER_SIZE];
	int i, index;
	printk(KERN_INFO "HAUSP: Processing command\n");
	printk(KERN_INFO "HAUSP: received command = %s\n", command);
	if (decompose(command, "AUTH", user)) {
		printk(KERN_INFO "HAUSP: user = %s\n", user);
		if (search(user) >= 0) {
			printk(KERN_INFO "HAUSP: Authorized\n");
			return 0;
		} else {
			printk(KERN_INFO "HAUSP: Police called!\n");
			return 1;
		}
	} else if (decompose(command, "ADD", user)) {
		index = search(user);
		if (index == -1 && registered_users < MAX_AUTHORIZED_USERS) {
			users[registered_users] = kmalloc(sizeof(char) * USER_SIZE, GFP_KERNEL);
			memcpy(users[registered_users++], &user, USER_SIZE);
			printk(KERN_INFO "HAUSP: %s added\n", user);
		} else if (index != -1) {
			printk(KERN_INFO "HAUSP: Duplicate USER\n");
		} else {
			printk(KERN_INFO "HAUSP: Max number of USERs reached\n");
		}
	} else if (decompose(command, "REMOVE", user)) {
		index = search(user);
		if (index >= 0) {
			kfree(users[index]);
			for (i = index; i < MAX_AUTHORIZED_USERS; i++) {
				users[i] = users[i + 1];
			}
			--registered_users;
		}
	}
	return 2;
}
