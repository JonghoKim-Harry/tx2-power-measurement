#ifndef RAWDATA_H
#define RAWDATA_H

#include "../measurement_info.h"

ssize_t collect_timestamp(const int rawdata_fd);
ssize_t collect_gpupower(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_gpufreq(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_gpuutil(const int rawdata_fd, const int sysfs_fd1);
void measure_rawdata(const int pid, const measurement_info_struct info);

#endif   // RAWDATA_H
