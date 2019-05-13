#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#include "enhanced_shcmd.h"

#define MAX_DEPTH   10

int mkdir_p(const char *path, mode_t mode) {

    /*
     *   $ mkdir -p -m <mode> <path>
     */
    char buff[4096];
    int push;
    char *p;

    strcpy(buff, path);

    push = 0;
    for(p=buff+1; *p; ++p) {
        if(*p == '/') {
            push = 1;
            *p = '\0';
        }

        if(!*p) {
            if(access(buff, F_OK | R_OK | X_OK)) {
                if(mkdir(buff, mode)) return -1;
            }
        }

        if(push) {
            *p = '/';
            push = 0;
        }
    }

    // If directory already exist, do NOT see it as fail
    if(mkdir(buff, mode)  &&  (errno != EEXIST)) return -1;

    // mkdir() returns zero on success, or -1 if an error occured
    // (in which case, errno is set appropriately)
    return 0;
}

int chown_R(const char *pathname, uid_t owner, gid_t group) {

    /*
     *   $ chown -R <owner>:<group>
     */
    DIR *dirptr;
    char curr_dirname[128], filename[128];
    DIR *dirptr_stack[MAX_DEPTH];
    off_t offset_stack[MAX_DEPTH];
    char dirname_stack[MAX_DEPTH][128];
    struct dirent *entry;
    struct stat status;
    int depth;

    depth = 0;
    strcpy(curr_dirname, pathname);

    if(chown(pathname, owner, group)) return -1;
    dirptr = opendir(pathname);

    if(dirptr == NULL) {
        perror("opendir() fail");
        return -1;
    }

chown_entries:

    while((entry = readdir(dirptr)) != NULL) {

        // Skip if filename is "." or ".."
        if(!strcmp(entry->d_name, ".")) continue;
        if(!strcmp(entry->d_name, "..")) continue;

        // Find filename
        strcpy(filename, curr_dirname);
        strcat(filename, "/");
        strcat(filename, entry->d_name);

        printf("\nfilename: %s", filename);

        // chown
        if(chown(filename, owner, group)) return -1;

        stat(filename, &status);
        switch(status.st_mode & S_IFMT) {
            case S_IFDIR:
                dirptr_stack[depth] = dirptr;
                offset_stack[depth] = telldir(dirptr);
                strcpy(dirname_stack[depth], curr_dirname);
                ++depth;
                strcpy(curr_dirname, filename);
                dirptr = opendir(curr_dirname);
                break;
            default:
                break;
        }
    }

    if(depth > 0) {

        --depth;
        dirptr = dirptr_stack[depth];
        seekdir(dirptr, offset_stack[depth]);
        strcpy(curr_dirname, dirname_stack[depth]);
        goto chown_entries;
    }

    // chown() returns zero on success, or -1 if an error occured
    // (in which case, errno is set appropriately)
    return 0;
}
