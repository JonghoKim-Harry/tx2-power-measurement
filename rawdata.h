#ifndef RAWDATA_H
#define RAWDATA_H

#include "default_values.h"

// Do NOT include measurement_info.h
struct measurement_info_struct;

typedef struct rawdata_info_struct {

    ssize_t (*func_read_rawdata)(const int rawdata_fd, ...);
    ssize_t (*func_rawdata_to_powerlog)(struct powerlog_struct *powerlog, const int rawdata_fd);
    ssize_t data_size;
    int num_sysfs_fd;
    int sysfs_fd[MAX_NUM_SYSFS_FD];
} rawdata_info_struct;

ssize_t collect_timestamp(const int rawdata_fd);
ssize_t collect_gpupower(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_gpufreq(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_gpuutil(const int rawdata_fd, const int sysfs_fd1);

ssize_t collect_mempower(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_emcfreq(const int rawdata_fd, const int sysfs_fd1);
ssize_t collect_emcutil(const int rawdata_fd, const int sysfs_fd1);

void measure_rawdata(const int pid, const struct measurement_info_struct info);

#endif   // RAWDATA_H
