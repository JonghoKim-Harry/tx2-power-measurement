#ifndef PARSE_CAFFELOG_H
#define PARSE_CAFFELOG_H

#include "measurement_info.h"

void init_caffelog_parser();
int64_t diff_timestamp(const struct timespec timestamp1, const struct timespec timestamp2);
off_t parse_caffelog(const int caffelog_fd, const off_t offset, const struct tm calendar, caffelog_struct *caffelog);
void free_caffelog_parser();

#endif   // PARSE_CAFFELOG_H
