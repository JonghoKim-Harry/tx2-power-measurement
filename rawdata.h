#ifndef RAWDATA_H
#define RAWDATA_H

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

#include "measurement_info.h"

#define  NO_SYSFS_FILE      0
#define ONE_SYSFS_FILE      1
#define TWO_SYSFS_FILES     2
#define THREE_SYSFS_FILES   3
#define FOUR_SYSFS_FILES    4
#define FIVE_SYSFS_FILES    5
#define SIX_SYSFS_FILES     6
#define SEVEN_SYSFS_FILES   7
#define EIGHT_SYSFS_FILES   8


void register_rawdata
    (struct measurement_info_struct *info,
     ssize_t (*func_read_rawdata)(const int rawdata_fd, ...),
     ssize_t (*func_rawdata_to_powerlog)(powerlog_struct *powerlog, const int rawdata_fd),
     const int num_sysfs_file, ...);

void close_sysfs_files(struct measurement_info_struct info);

ssize_t collect_timestamp(const int rawdata_fd);
ssize_t collect_gpupower(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_gpufreq(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_gpuutil(const int rawdata_fd, const int sysfs_fd1);

ssize_t timestamp_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpupower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpufreq_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);
ssize_t gpuutil_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd);

#endif   // RAWDATA_H
