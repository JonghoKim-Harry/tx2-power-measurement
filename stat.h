#ifndef STAT_H
#define STAT_H

#include <stdio.h>
#include "measurement_info.h"

off_t print_expinfo(const int stat_fd, const measurement_info_struct info);
ssize_t print_header_raw(const int stat_fd, const measurement_info_struct info);

#endif   // STAT_H
