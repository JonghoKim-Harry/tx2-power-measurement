#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "powerlog.h"
#include "tx2_sysfs_power.h"

#define IS_WHITESPACE(x)   (x == 0x20 || x == 0x09 || x == 0x0a)

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

    ssize_t num_read_bytes, num_whitespace;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];
    const char *ptr;

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG
    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);
    num_whitespace = 0;

    if(num_read_bytes > 0) {
        // Hz ---> MHz
        for(ptr=&buff[num_read_bytes - 1]; IS_WHITESPACE(*ptr); ptr--)
            ++num_whitespace;

        buff[num_read_bytes - num_whitespace - 6] = '\0';
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
        // Note that GPU utilization is stored as "x0.1%"
        powerlog->gpu_util = atoi(buff);
    }

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}


#ifdef TRACE_MEM
ssize_t mempower_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_MEM_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_MEM_POWER_MAX_STRLEN);

    if(num_read_bytes > 0)
        powerlog->mem_power = atoi(buff);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t emcfreq_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    ssize_t num_whitespace;
    char buff[TX2_SYSFS_EMC_FREQ_MAX_STRLEN];
    const char *ptr;

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_EMC_FREQ_MAX_STRLEN);
    num_whitespace = 0;

    if(num_read_bytes > 0) {
        // Hz ---> MHz
        for(ptr=&buff[num_read_bytes - 1]; IS_WHITESPACE(*ptr); ptr--)
            ++num_whitespace;

        buff[num_read_bytes - num_whitespace - 6] = '\0';
        powerlog->emc_freq = atoi(buff);
    }

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t emcutil_to_powerlog(powerlog_struct *powerlog, const int rawdata_fd) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_EMC_UTIL_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    num_read_bytes = read(rawdata_fd, buff, TX2_SYSFS_EMC_UTIL_MAX_STRLEN);

    if(num_read_bytes > 0) {
        // Note that EMC utilization is stored as "x0.0001%"
        powerlog->emc_util = atoi(buff);
    }

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}
#endif   // TRACE_MEM
