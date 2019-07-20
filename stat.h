#ifndef STAT_H
#define STAT_H

#include <stdio.h>
#include "default_values.h"

// Do NOT include measurement_info.h
struct measurement_info_struct;

enum logtype_t {
    LOGTYPE_NA = 0,
    LOGTYPE_TIME,
    LOGTYPE_TIMESTAMP,
    LOGTYPE_POWERLOG,
    LOGTYPE_SUMMARY,
    LOGTYPE_CAFFELOG,
    LOGTYPE_TEGRALOG,
    NUM_LOGTYPES
};

typedef struct stat_info_struct {

    char colname[MAX_COLWIDTH];
    int  colwidth;
    enum logtype_t logtype;
    ssize_t (*func_log_to_stat)(const int stat_fd, const int colwidth, ...);
} stat_info_struct;

void register_stat(
     struct measurement_info_struct *info,
     const char *colname,
     const int colwidth,
     enum logtype_t logtype,
     ssize_t (*func_log_to_stat)(const int stat_fd, const int colwidth, ...)
);


off_t print_expinfo(const int stat_fd, const struct measurement_info_struct info);
ssize_t print_header_raw(const int stat_fd, const struct measurement_info_struct info);

#endif   // STAT_H
