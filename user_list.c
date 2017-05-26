#include "user_list.h"

#include <crypto/hash.h>
#include <linux/err.h>
#include <linux/scatterlist.h>
#include <linux/string.h>

int search(const List* list, const char* user) {
    unsigned int begin = 0;
    unsigned int end = list->size;
    unsigned middle;
    int comparison;

    while (begin < end) {
        middle = (begin + end) / 2;
        comparison = strcmp(user, list->entries[middle].user);
        if (comparison == 0) {
            return middle;
        } else if (comparison < 0) {
            end = middle;
        } else if (comparison > 0) {
            begin = middle;
        }
    }

    return -1;
}

// TODO: check duplicates & ensure sortedness
int insert(List* list, const char* user, rfid_t rfid) {
    Entry entry;

    if (list->size < MAX_AUTHORIZED_USERS) {
        entry.user = user;
        entry.rfid_hash = hash_it(rfid);
        list->entries[list->size] = entry;
        list->size++;
        return 0;
    }

    return -1;
}

int remove(List* list, const char* user) {
    int index = search(list, user);
    int i;

    if (index >= 0) {
        list->size--;
        for (i = index; i < list->size; i++) {
            list->entries[i] = list->entries[i + 1];
        }
    }

    return -1;
}

uint8_t* hash_it(const char* plaintext) {
    size_t len = strlen(plaintext);
    struct scatterlist sg;
    struct hash_desc desc;
    uint8_t hashval[32];

    sg_init_one(&sg, plaintext, len);
    desc.tfm = crypto_alloc_hash("sha256", 0, CRYPTO_ALG_ASYNC);

    crypto_hash_init(&desc);
    crypto_hash_update(&desc, &sg, len);
    crypto_hash_final(&desc, hashval);
    
    crypto_free_hash(desc.tfm);

    return hashval;
}
