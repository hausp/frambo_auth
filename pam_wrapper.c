#include <asm/segment.h>
#include <asm/uaccess.h>
// #include <linux/device.h>
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

// struct file* file_open(const char *path, int flags, int rights) {
//     struct file* filp = NULL;
//     mm_segment_t oldfs;
//     int err = 0;

//     oldfs = get_fs();
//     set_fs(get_ds());
//     filp = filp_open(path, flags, rights);
//     set_fs(oldfs);
//     if (IS_ERR(filp)) {
//         err = PTR_ERR(filp);
//         return NULL;
//     }
//     return filp;
// }

// void file_close(struct file* file) {
//     filp_close(file, NULL);
// }

// int file_read(struct file* file, unsigned long long offset, unsigned char *data, unsigned int size) {
//     mm_segment_t oldfs;
//     int ret;

//     oldfs = get_fs();
//     set_fs(get_ds());

//     ret = vfs_read(file, data, size, &offset);

//     set_fs(oldfs);
//     return ret;
// }

void pam_init(void) {
	users = list_create();
	// file* users_file = file_open("/etc/frusers", O_RDONLY, 0);


}

// int decompose(char* full_command, char* command, char* user) {
// 	int size = strlen(command);
// 	memcpy(user, &full_command[size + 1], USER_SIZE - 1);
// 	user[USER_SIZE - 1] = '\0';
// 	return strncmp(full_command, command, size) == 0;
// }

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
	printk(KERN_INFO "HAUSP: received command = %s\n", command);
	if (decompose(command, "AUTH", user, &id_text)) {
		printk(KERN_INFO "HAUSP: user = %s\n", user);
		index = list_search(users, user);
		if (index >= 0 && users->entries[index]->rfid_hash == hash_it(id_text)) {
			printk(KERN_INFO "HAUSP: Authorized\n");
			return 0;
		} else {
			printk(KERN_INFO "HAUSP: Police called!\n");
			return 1;
		}
	} else if (decompose(command, "ADD", user, &id_text)) {
		index = list_search(users, user);
		if (index == -1 && (users->size < MAX_AUTHORIZED_USERS)) {
			failure = kstrtoint(id_text, 10, &hash);
			printk(KERN_INFO "HAUSP: id_text = %s\n", id_text);
			printk(KERN_INFO "HAUSP: hash = %d\n", hash);
			if (failure) {
				printk(KERN_INFO "HAUSP: Invalid ID\n");
				return 1;
			}

			list_insert(users, user, hash);
			// users[registered_users] = kmalloc(sizeof(char) * USER_SIZE, GFP_KERNEL);
			// memcpy(users[registered_users++], &user, USER_SIZE);
			printk(KERN_INFO "HAUSP: %s added\n", user);
		} else if (index != -1) {
			printk(KERN_INFO "HAUSP: Duplicate USER\n");
		} else {
			printk(KERN_INFO "HAUSP: Max number of USERs reached\n");
		}
	// } else if (decompose(command, "REMOVE", user, &rfid_hash)) {
	// 	// index = list_search(users, user);
	// 	list_remove(users, user);
	// 	// if (index >= 0) {
	// 	// 	kfree(users[index]);
	// 	// 	for (i = index; i < MAX_AUTHORIZED_USERS; i++) {
	// 	// 		users[i] = users[i + 1];
	// 	// 	}
	// 	// 	--registered_users;
	// 	// }
	}
	return 2;
}
