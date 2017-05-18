/* Define which PAM interfaces we provide */
#define PAM_SM_ACCOUNT
#define PAM_SM_AUTH
#define PAM_SM_PASSWORD
#define PAM_SM_SESSION

/* Read/Write includes */ 
#include <fcntl.h>
#include <unistd.h>

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

/* PAM entry point for authentication verification */
int pam_sm_authenticate(pam_handle_t *pamh, int flags, int argc, const char **argv) {
    const char *user = NULL;
    int pgu_ret;

    pgu_ret = pam_get_user(pamh, &user, NULL);

    if (pgu_ret != PAM_SUCCESS || user == NULL) {
        return PAM_IGNORE;
    }

    int file = open("/dev/raspchar", O_RDWR);
    char* command = "AUTH ";
    char[5] response;
    strcat(command, user);

    if (write(file, &command, sizeof(command)) > 0) {
        if (read(file, &response, sizeof(response)) > 0) {
            if (strncmp(response, "OK", 2) == ) {
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