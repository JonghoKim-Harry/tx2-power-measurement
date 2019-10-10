#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>   // perror()

static const uid_t ROOT_UID = 0;
static const gid_t ROOT_GID = 0;


int restore_root_privilege() {

    if(seteuid(ROOT_UID) == -1) {
        perror("Can not switch to root UID");
        return -1;
    }

    if(setegid(ROOT_GID) == -1) {
        perror("Can not switch to root GID");
        return -1;
    }

    return 0;
}

int drop_root_privilege_temp() {

    if (setegid(getgid()) != 0) {
        perror("setegid() failed");
        return -1;
    }

    if (seteuid(getuid()) != 0) {
        perror("seteuid() failed");
        return -1;
    }

    return 0;
}

int drop_root_privilege_perm() {

    if (setgid(getgid()) != 0) {
        perror("setgid() failed");
        return -1;
    }

    if (setuid(getuid()) != 0) {
        perror("setuid() failed");
        return -1;
    }

    return 0;
}
