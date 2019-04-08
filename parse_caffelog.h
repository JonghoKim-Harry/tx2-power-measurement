#ifndef PARSE_CAFFELOG_H
#define PARSE_CAFFELOG_H

#include <unistd.h>
#include <time.h>
#include <regex.h>

#include "read_sysfs_stat.h"

struct caffe_event {
    struct timespec gmt_timestamp;
    off_t offset;
    char event[64];
};

off_t parse_caffelog(const int caffelog_fd, const regex_t timestamp_pattern, const off_t offset, struct caffe_event *event);

#endif   // PARSE_CAFFELOG_H
