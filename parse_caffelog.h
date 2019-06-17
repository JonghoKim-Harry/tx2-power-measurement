#ifndef PARSE_CAFFELOG_H
#define PARSE_CAFFELOG_H

#include "measurement_info.h"

int64_t diff_timestamp(const struct timespec timestamp1, const struct timespec timestamp2);
off_t parse_caffelog(const int caffelog_fd, const regex_t timestamp_pattern, const off_t offset, const struct tm calendar, caffelog_struct *caffelog);

#endif   // PARSE_CAFFELOG_H
