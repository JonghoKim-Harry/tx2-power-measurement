#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "rawdata_to_powerlog.h"
#include "tx2_sysfs_power.h"

ssize_t timestamp_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;

    num_read_bytes = read(rawdata_fd, &powerlog->gmt_timestamp, sizeof(struct timespec));

    return num_read_bytes;
}

ssize_t gpupower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_POWER_MAX_STRLEN];

    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_GPU_POWER_MAX_STRLEN);

    if(num_read_bytes > 0)
        powerlog->gpu_power = atoi(buff);

    return num_read_bytes;
}

ssize_t gpufreq_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

    if(num_read_bytes > 0) {
        // Hz ---> MHz
        buff[num_read_bytes - 6] = '\0';
        powerlog->gpu_freq = atoi(buff);
    }

    return num_read_bytes;
}

ssize_t gpuutil_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_UTIL_MAX_STRLEN];

    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_GPU_UTIL_MAX_STRLEN);

    if(num_read_bytes > 0) {
        // Note that utilization is stored as "x10%"
        powerlog->gpu_util = atoi(buff);
    }

    return num_read_bytes;
}
