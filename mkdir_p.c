#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include "mkdir_p.h"

int mkdir_p(const char *path, mode_t mode) {

    int mkdir_result;
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
                mkdir_result = mkdir(buff, mode);
                if(mkdir_result)
                    return mkdir_result;
            }
        }

        if(push) {
            *p = '/';
            push = 0;
        }
    }

    mkdir_result = mkdir(buff, mode);
    if(mkdir_result)
        return mkdir_result;

    // mkdir() returns zero on success, or -1 if an error occured
    // (in which case, errno is set appropriately)
    return 0;
}
