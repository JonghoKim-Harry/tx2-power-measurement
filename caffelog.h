#ifndef CAFFELOG_H
#define CAFFELOG_H

#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <regex.h>

#include "rawdata.h"

// match[0]: the whole match. Do not care about it
// match[1]: caffelog timestamp
// match[2]: caffelog event
#define CAFFELOG_PATTERN                                                 \
    "[[:alpha:][:space:]]*"                                              \
    "([[:digit:]]{2}:[[:digit:]]{2}:[[:digit:]]{2}[.][[:digit:]]{6})"    \
    "[^\]]*\][[:space:]]*"                                               \
    "([^[:space:]].*)"

typedef struct caffelog_struct {
    struct timespec gmt_timestamp;

    /*
     *  HMS: Hour, Minute, Second
     *  Note that year, month, day are ignored to be 0
     */
    struct tm gmt_date_hms;
    char event[128];
} caffelog_struct;

int64_t compare_timestamp_hms(const struct tm timestamp1, const struct tm timestamp2);
off_t parse_caffelog(const int caffelog_fd, const regex_t timestamp_pattern, const off_t offset, caffelog_struct *caffelog);

#endif   // CAFFELOG_H
