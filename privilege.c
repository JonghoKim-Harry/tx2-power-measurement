#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>   // perror()

static const uid_t ROOT_UID = 0;
static const gid_t ROOT_GID = 0;
static uid_t ruid, euid, suid;
static gid_t rgid, egid, sgid;

int push_privilege() {

    if(getresuid(&ruid, &euid, &suid) == -1) {
        perror("Can not get user identity (UID)");
        return -1;
    }

    if(getresgid(&rgid, &egid, &sgid) == -1) {
        perror("Can not get group identity (GID)");
        return -1;
    }

    return 0;
}

int get_root_privilege() {

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

int drop_root_privilege() {

    if (setgid(ROOT_GID) != 0) {
        perror("setgid() failed");
        return -1;
    }

    if (setuid(ROOT_UID) != 0) {
        perror("setuid() failed");
        return -1;
    }

    return 0;
}

int pop_privilege() {

    if(setresgid(rgid, egid, sgid) == -1) {
        perror("Can not return to group identity (GID)");
        return -1;
    }

    if(setresuid(ruid, euid, suid) == -1) {
        perror("Can not return to user identity (UID)");
        return -1;
    }
    return 0;
}
