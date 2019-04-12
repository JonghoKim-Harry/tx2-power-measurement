#ifndef MKDIR_P_H
#define MKDIR_P_H

#include <sys/stat.h>

int mkdir_p(const char *path, mode_t mode);

#endif   // MKDIR_P_H
