#include "user_list.h"

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/string.h>

struct List* list_create(void) {
    struct List* list = kmalloc(sizeof(struct List), GFP_KERNEL);
    list->size = 0;
    return list;
}

int list_search(const struct List* list, const char* user) {
    int begin = 0;
    int end = list->size - 1;
    int middle;
    int comparison;
    int caca = 0;

    printk(KERN_INFO "HAUSP: users list = %p\n", list);
    printk(KERN_INFO "HAUSP: user = %s\n", user);

    while (begin <= end) {   
        middle = (begin + end) / 2;
        // printk(KERN_INFO "HAUSP: middle = %d\n", middle);
        printk(KERN_INFO "HAUSP: list->entries[%d]->user = %s\n", middle, list->entries[middle]->user);
        comparison = strcmp(user, list->entries[middle]->user);
        if (comparison == 0) {
            return middle;
        } else if (comparison < 0) {
            end = middle - 1;
        } else if (comparison > 0) {
            ++middle;
            begin = middle;
        }
        caca++;
        if (caca > 10) {
            printk(KERN_INFO "HAUSP: deu caca\n");
            break;
        }
    }

    return -1;
}

int list_insert(struct List* list, const char* user, hash_t rfid) {
    struct Entry entry;
    // char* rfid_text = kmalloc(11 * sizeof(char), GFP_KERNEL);
    int begin = 0;
    int end = list->size - 1;
    int middle = 0;
    int comparison;
    int i;
    int caca = 0;

    if (list->size < MAX_AUTHORIZED_USERS) {
        printk(KERN_INFO "HAUSP: user = %s\n", user);
        // sprintf(rfid_text, "%d", rfid);
        entry.user = kmalloc((strlen(user) + 1) * sizeof(char), GFP_KERNEL);
        strcpy(entry.user, user);
        printk(KERN_INFO "HAUSP: entry.user = %s\n", entry.user);
        entry.rfid_hash = rfid;
        // entry.rfid_hash = rfid_text;

        while (begin <= end) {
            printk(KERN_INFO "HAUSP: begin = %d\n", begin);
            printk(KERN_INFO "HAUSP: end = %d\n", end);
            middle = (begin + end) / 2;
            comparison = strcmp(user, list->entries[middle]->user);
            printk(KERN_INFO "HAUSP: comparison = %d\n", comparison);
            if (comparison == 0) {
                return 1;
            } else if (comparison < 0) {
                end = middle - 1;
            } else if (comparison > 0) {
                ++middle;
                begin = middle;
            }

            if (caca > 10) {
                printk(KERN_INFO "HAUSP: < ABORT(Tx) >\n");
                break;
            }

            caca++;
        }

        for (i = list->size; i > middle; --i) {
            list->entries[i] = list->entries[i-1];
        }

        printk(KERN_INFO "HAUSP: ADDING in middle = %d\n", middle);
        list->entries[middle] = kmalloc(sizeof(struct Entry), GFP_KERNEL);
        memcpy(list->entries[middle], &entry, sizeof(struct Entry));
        // list->entries[middle] = entry;
        list->size++;

        return 0;
    }

    return -1;
}

int list_remove(struct List* list, const char* user) {
    int index = list_search(list, user);
    int i;

    if (index >= 0) {
        list->size--;
        for (i = index; i < list->size; i++) {
            list->entries[i] = list->entries[i + 1];
        }
    }

    return -1;
}

hash_t jenkins_hash(const uint8_t* key, size_t length) {
    size_t i = 0;
    hash_t hash = 0;
    while (i != length) {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

hash_t hash_it(const char* plaintext) {
    int length = strlen(plaintext);
    uint8_t* container = kmalloc(length * sizeof(uint8_t), GFP_KERNEL);
    int i;
    for (i = 0; i < length; i++) {
        container[i] = (uint8_t) plaintext[i];
    }
    return jenkins_hash(container, length);
}
