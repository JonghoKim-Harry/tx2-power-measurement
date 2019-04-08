#ifndef PARSE_CAFFELOG_H
#define PARSE_CAFFELOG_H

#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>

#include "read_sysfs_stat.h"

struct caffe_event {
    struct timespec gmt_timestamp;

    /*
     *  HMS: Hour, Minute, Second
     *  Note that year, month, day are ignored to be 0
     */
    struct tm gmt_date_hms;
    off_t offset;
    char event[64];
};

int64_t compare_timestamp_hms(const struct tm timestamp1, const struct tm timestamp2);
off_t parse_caffelog(const int caffelog_fd, const regex_t timestamp_pattern, const off_t offset, struct caffe_event *event);

#endif   // PARSE_CAFFELOG_H
