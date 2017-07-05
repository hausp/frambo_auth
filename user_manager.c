#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/buffer_head.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "user_list.h"

MODULE_LICENSE("GPL v2");

struct List* users; // = list_create()
int registered_users = 0;

void pam_init(void) {
	users = list_create();
}

int decompose(char* full_command, char* command, char* user, char** id) {
	int size = strlen(command);
	int full_size = strlen(full_command);
	int i = 0;
	int id_start = -1;
	unsigned buffer_size;

	while (i < full_size) {
		if (i < size && full_command[i] != command[i]) {
			return 0;
		}

		if (i == size && full_command[i] != ' ') {
			return 0;
		}

		if (i > size) {
			if (full_command[i] == ' ') {
				memcpy(user, &full_command[size + 1], i - size - 1);
				user[i - size - 1] = '\0';
				id_start = i + 1;
			}
		}

		i++;
	}

	buffer_size = full_size - id_start;
	*id = (char*) kmalloc(sizeof(char) * (buffer_size + 1), GFP_KERNEL);
	memcpy(*id, &full_command[id_start], buffer_size);
	(*id)[buffer_size] = '\0';

	return 1;
}

int process_command(char* command) {
	char user[USER_SIZE];
	char* id_text;
	int index;
	hash_t hash;
	int failure;

	printk(KERN_INFO "HAUSP: Processing command\n");
	if (decompose(command, "AUTH", user, &id_text)) {
		index = list_search(users, user);

		if (index >= 0 && users->entries[index]->rfid_hash == hash_it(id_text)) {
			printk(KERN_INFO "HAUSP: Authorized\n");
			return 0;
		} else {
			printk(KERN_INFO "HAUSP: Denied\n");
			return 1;
		}
	} else if (decompose(command, "ADD", user, &id_text)) {
		index = list_search(users, user);
		if (index == -1 && (users->size < MAX_AUTHORIZED_USERS)) {
			failure = kstrtouint(id_text, 10, &hash);

			if (failure == -EINVAL) {
				printk(KERN_DEBUG "HAUSP: Invalid ID, parsing error\n");
				return 1;
			} else if (failure == -ERANGE) {
				printk(KERN_DEBUG "HAUSP: Invalid ID, overflow\n");
				return 1;
			}

			list_insert(users, user, hash);
			printk(KERN_DEBUG "HAUSP: %s added\n", user);
		} else if (index != -1) {
			printk(KERN_INFO "HAUSP: Duplicate USER\n");
		} else {
			printk(KERN_INFO "HAUSP: Max number of USERs reached\n");
		}
	}
	return 2;
}
