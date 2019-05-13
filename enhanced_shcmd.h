#ifndef ENHANCED_SHCMD_H
#define ENHANCED_SHCMD_H

#include <unistd.h>
#include <sys/stat.h>

int mkdir_p(const char *path, mode_t mode);
int chown_R(const char *pathname, uid_t owner, gid_t group);

#endif    // ENHANCED_SHCMD_H
