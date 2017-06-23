#ifndef USER_LIST_H
#define USER_LIST_H

#include <linux/types.h>

#define USER_SIZE 33
#define MAX_AUTHORIZED_USERS 50

typedef uint32_t rfid_t;
typedef uint32_t hash_t;

extern struct Entry {
    char* user;
    hash_t rfid_hash;
} Entry;

extern struct List {
    struct Entry* entries[MAX_AUTHORIZED_USERS];
    int size;
} List;

struct List* list_create(void);

int list_search(const struct List* list, const char* user);
int list_insert(struct List* list, const char* user, rfid_t rfid);
int list_remove(struct List* list, const char* user);

extern hash_t hash_it(const char* plaintext);

#endif
