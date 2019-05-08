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

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    num_read_bytes = read(rawdata_fd, &powerlog->timestamp, sizeof(struct timespec));

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t gpupower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG
    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_GPU_POWER_MAX_STRLEN);

    if(num_read_bytes > 0)
        powerlog->gpu_power = atoi(buff);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t gpufreq_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG
    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

    if(num_read_bytes > 0) {
        // Hz ---> MHz
        buff[num_read_bytes - 6] = '\0';
        powerlog->gpu_freq = atoi(buff);
    }

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t gpuutil_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_UTIL_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG
    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_GPU_UTIL_MAX_STRLEN);

    if(num_read_bytes > 0) {
        // Note that utilization is stored as "x10%"
        powerlog->gpu_util = atoi(buff);
    }

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}
