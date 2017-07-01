/* Define which PAM interfaces we provide */
#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

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

// const char* call_python(const char* filename) {
//     int link[2];
//     pid_t pid;
//     char foo[4096];

//     if (pipe(link)==-1)
//         die("pipe failed");

//     if ((pid = fork()) == -1)
//         die("fork failed");

//     if(pid == 0) {

//     dup2 (link[1], STDOUT_FILENO);
//     close(link[0]);
//     close(link[1]);
//     execl("/bin/ls", "ls", "-1", (char *)0);
//     die("execl");

//     } else {

//     close(link[1]);
//     int nbytes = read(link[0], foo, sizeof(foo));
//     printf("Output: (%.*s)\n", nbytes, foo);
//     wait(NULL);

//     }
//     return 0;
//     }
// }

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *user = NULL;
    int pgu_ret;
    int file = open("/dev/raspchar", O_RDWR);
    char* command;
    char response[5];

    pgu_ret = pam_get_user(pamh, &user, NULL);

    command = (char*)malloc(sizeof(char) * (6+strlen(user)));

    if (pgu_ret != PAM_SUCCESS || user == NULL) {
        return PAM_IGNORE;
    }

    memcpy(command, "AUTH \0", 6);
    // exec("")
    strcat(command, user);

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
