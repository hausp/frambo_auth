/* Define which PAM interfaces we provide */
#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

#include <assert.h>

/* Read/Write includes */ 
#include <fcntl.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>

/* Include PAM headers */
#include <security/pam_appl.h>
#include <security/pam_modules.h>

/* PAM entry point for session creation */
int pam_sm_open_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

/* PAM entry point for session cleanup */
int pam_sm_close_session(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

/* PAM entry point for accounting */
int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
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

        execl(filename, filename, 0, (char*) 0);
        exit(0);
    }

    close(pipes[1]);

    unsigned size = 20;
    char* buffer = malloc(size * sizeof(char));
    /*unsigned outputSize = */read(pipes[0], buffer, size);

    close(pipes[0]);

    return buffer;
}

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char* user = NULL;
    int pgu_ret;
    int file = open("/dev/raspchar", O_RDWR);
    char* command;
    char response[5];

    pgu_ret = pam_get_user(pamh, &user, NULL);

    const char* rfid = call_python("loremipsum.py");

    command = (char*)malloc(sizeof(char) * (7 + strlen(user) + strlen(rfid)));

    if (pgu_ret != PAM_SUCCESS || user == NULL) {
        return PAM_IGNORE;
    }

    memcpy(command, "AUTH \0", 6);
    strcat(command, user);
    strcat(command, " ");
    strcat(command, rfid);

    if (write(file, command, strlen(command)) > 0) {
        if (read(file, &response, sizeof(response)) > 0) {
            if (strncmp(response, "OK", 2) == 0) {
                return PAM_SUCCESS;
            }
        }
    }
    
    return PAM_IGNORE;
}

/*
   PAM entry point for setting user credentials (that is, to actually
   establish the authenticated user's credentials to the service provider)
 */
int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}

/* PAM entry point for authentication token (password) changes */
int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    return PAM_SUCCESS;
}
