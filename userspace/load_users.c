#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

void add_all() {
    FILE* raspchar = fopen("/dev/raspchar", "w");

    if (!raspchar) {
        printf("Failed to open raspchar\n");
        exit(1);
    }

    FILE* users = fopen("/etc/frusers", "r");

    if (!users) {
        printf("Failed to open frusers\n");
        fclose(raspchar);
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), users)) {
        if (strcmp(line, "\n") != 0 && strcmp(line, "") != 0) {
            char* command = (char*) malloc(sizeof(char) * (5 + strlen(line)));
            line[strlen(line) - 1] = '\0';

            memcpy(command, "ADD \0", 5);
            strcat(command, line);

            fwrite(command, sizeof(char), strlen(command), raspchar);
            free(command);
        }
    }

    fclose(raspchar);
    fclose(users);
}

const char* call_python(const char* filename) {
    int pipes[2];

    assert(pipe(pipes) >= 0);
    pid_t childpid = fork();

    assert(childpid >= 0);
    if (childpid == 0) {
        assert(dup2(pipes[1], 1) >= 0);
        assert(dup2(pipes[1], 2) >= 0);
        close(pipes[0]);
        close(pipes[1]);

        // char* command = malloc()
        execl("/usr/bin/python", "/usr/bin/python", filename, "--no-alarm", (char*) 0);
        exit(0);
    }

    close(pipes[1]);

    unsigned size = 20;
    char* buffer = (char*) malloc(size * sizeof(char));
    /*unsigned outputSize = */read(pipes[0], buffer, size);

    close(pipes[0]);

    return buffer;
}

uint32_t jenkins_hash(const uint8_t* key, size_t length) {
    size_t i = 0;
    uint32_t hash = 0;
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

uint32_t hash_it(const char* plaintext) {
    int length = strlen(plaintext);
    uint8_t* container = (uint8_t*) malloc(length * sizeof(uint8_t));
    int i;
    for (i = 0; i < length; i++) {
        container[i] = (uint8_t) plaintext[i];
    }
    return jenkins_hash(container, length);
}

int main(int argc, char** argv) {
    if (argc == 1) {
        add_all();
    } else {
        unsigned user_size = strlen(argv[1]);
        const char* rfid;

        int read_ok = 0;
        printf("Please scan your card.\n");
        while (!read_ok) {
            rfid = call_python("/usr/sbin/rfid_reader");

            if (strcmp(rfid, "") == 0 || strlen(rfid) < 3) {
                printf("RFID read error\n");
            } else {
                read_ok = 1;
            }
        }

        if (strcmp(rfid, "0") == 0) {
            printf("User registration failed.\n");
            return 1;
        }

        uint32_t hash = hash_it(rfid);
        char* hash_text = (char*) malloc(15 * sizeof(char));

        sprintf(hash_text, "%u", hash);

        FILE* users = fopen("/etc/frusers", "a");
        char* command = (char*) malloc(sizeof(char) * (2 + user_size + strlen(hash_text)));

        memcpy(command, argv[1], user_size);
        strcat(command, " ");
        strcat(command, hash_text);
        strcat(command, "\n");

        fwrite(command, sizeof(char), strlen(command), users);
        free(command);

        printf("User registered successfully.\n");

        fclose(users);

    }
}
