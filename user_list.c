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

    while (begin <= end) {   
        middle = (begin + end) / 2;
        comparison = strcmp(user, list->entries[middle]->user);
        if (comparison == 0) {
            return middle;
        } else if (comparison < 0) {
            end = middle - 1;
        } else if (comparison > 0) {
            ++middle;
            begin = middle;
        }
    }

    return -1;
}

int list_insert(struct List* list, const char* user, hash_t rfid) {
    struct Entry entry;
    int begin = 0;
    int end = list->size - 1;
    int middle = 0;
    int comparison;
    int i;

    if (list->size < MAX_AUTHORIZED_USERS) {
        entry.user = kmalloc((strlen(user) + 1) * sizeof(char), GFP_KERNEL);
        strcpy(entry.user, user);
        entry.rfid_hash = rfid;

        while (begin <= end) {
            middle = (begin + end) / 2;
            comparison = strcmp(user, list->entries[middle]->user);
            if (comparison == 0) {
                return 1;
            } else if (comparison < 0) {
                end = middle - 1;
            } else if (comparison > 0) {
                ++middle;
                begin = middle;
            }
        }

        for (i = list->size; i > middle; --i) {
            list->entries[i] = list->entries[i-1];
        }

        list->entries[middle] = kmalloc(sizeof(struct Entry), GFP_KERNEL);
        memcpy(list->entries[middle], &entry, sizeof(struct Entry));

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
