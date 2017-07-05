#include <assert.h>

#include <fcntl.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <sys/wait.h>

#include <security/pam_appl.h>
#include <security/pam_modules.h>

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

        execl(filename, filename, 0, (char*) 0);
        exit(0);
    }

    int status;
    waitpid(childpid, &status, 0);

    close(pipes[1]);

    unsigned size = 20;
    char* buffer = (char*) malloc(size * sizeof(char));
    read(pipes[0], buffer, size);

    close(pipes[0]);

    return buffer;
}

/* PAM entry point for authentication verification */
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char* user = NULL;
    int pgu_ret;
    int file = open("/dev/raspchar", O_RDWR);
    char* command;
    char response[5];

    pgu_ret = pam_get_user(pamh, &user, NULL);

    if (pgu_ret != PAM_SUCCESS || user == NULL) {
        return PAM_SYSTEM_ERR;
    }

    printf("Please, scan your card within 5 seconds...\n");

    const char* rfid = call_python("/usr/sbin/rfid_reader");

    command = (char*)malloc(sizeof(char) * (7 + strlen(user) + strlen(rfid)));

    memcpy(command, "AUTH \0", 6);
    strcat(command, user);
    strcat(command, " ");
    strcat(command, rfid);

    int write_status = write(file, command, strlen(command));

    free(command);

    if (write_status > 0) {
        if (read(file, &response, sizeof(response)) > 0) {
            if (strncmp(response, "OK", 2) == 0) {
                return PAM_SUCCESS;
            }
        }
    }
    
    return PAM_AUTH_ERR;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}
