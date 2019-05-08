#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "collect_rawdata.h"
#include "tx2_sysfs_power.h"

#define WHITESPACE   "                                                       "

ssize_t collect_timestamp(const int rawdata_fd) {

    struct timespec time;

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    //  0: Sucess
    // -1: Fail
    if(clock_gettime(CLOCK_REALTIME, &time) == -1)
        return -1;

    if(write(rawdata_fd, &time, sizeof(struct timespec)) == -1)
        return -1;

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: 0", __func__, __FILE__, __LINE__);
#endif   // DEBUG
    return 0;
}

ssize_t collect_gpupower(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_GPU_POWER_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_GPU_POWER_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t collect_gpufreq(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_GPU_FREQ_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t collect_gpuutil(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_UTIL_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_GPU_UTIL_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_GPU_UTIL_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}
