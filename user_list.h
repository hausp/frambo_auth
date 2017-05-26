#ifndef USER_LIST_H
#define USER_LIST_H

#include <linux/types.h>

#define USER_SIZE 33
#define MAX_AUTHORIZED_USERS 50

typedef uint32_t rfid_t;
typedef uint8_t[32] hash_t;

struct Entry {
    const char user[USER_SIZE];
    hash_t rfid_hash;
};

struct List {
    Entry entries[MAX_AUTHORIZED_USERS];
    int size = 0;
};

int search(const List* list, const char* user);
int insert(List* list, const char* user, rfid_t rfid);
int remove(List* list, const char* user);

uint8_t* hash_it(const char* plaintext);

#endif
