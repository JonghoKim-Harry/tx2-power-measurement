#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "tx2_sysfs_power.h"
#include "register_rawdata.h"

#define WHITESPACE   "                                                       "

void register_rawdata
    (measurement_info_struct *info,
     ssize_t (*func_read_rawdata)(const int rawdata_fd, ...),
     ssize_t (*func_rawdata_to_powerlog)(powerlog_struct *powerlog, const int rawdata_fd),
     const int num_sysfs_file, ...) {

    const int index = info->num_rawdata;
    rawdata_info_struct *rawdata_info = &info->rawdata_info[index];
    va_list sysfs_file_list;
    int i;
    char buff[128];

#ifdef DEBUG
    printf("\nregister_rawdata()   START");
    printf("\nregister_rawdata()   given parameters");
    printf("\n * func_read_rawdata: %p", func_read_rawdata);
    printf("\n * func_rawdata_to_powerlog: %p", func_rawdata_to_powerlog);
    printf("\n * num_sysfs_file: %d", num_sysfs_file);
#endif   // DEBUG

    // Register to rawdata_info:
    //  1) A function pointer to read rawdata: (ex) read_rawdata_1
    //  2) A function pointer to conver rawdata to powerlog: (ex) rawdata_to_powerlog_1
    //  3) Data size in bytes
    //  4) The number of sysfs files to read, sysfs files' fds,
    //     and maximum lengths of strings to read from sysfs files
    rawdata_info->func_read_rawdata = func_read_rawdata;
    rawdata_info->func_rawdata_to_powerlog = func_rawdata_to_powerlog;
    rawdata_info->num_sysfs_fd = num_sysfs_file;

    va_start(sysfs_file_list, num_sysfs_file);

    for(i=0; i<num_sysfs_file; i++) {
        strcpy(buff, va_arg(sysfs_file_list, char*));
        rawdata_info->sysfs_fd[i] = open(buff, O_RDONLY | O_NONBLOCK);
    }

    va_end(sysfs_file_list);

#ifdef DEBUG
    for(i=0; i<num_sysfs_file; i++) {
        printf("\nregister_sysfs()   rawdata_info->sysfs_fd[%d]: %d", i, rawdata_info->sysfs_fd[i]);
    }
#endif   // DEBUG

    // Register to info: the number of rawdata
    ++(info->num_rawdata);

#ifdef DEBUG
    printf("\nregister_sysfs()   FINISHED");
#endif   // DEBUG

    return;
}

void close_sysfs_files(measurement_info_struct info) {

    int i, j;
    rawdata_info_struct *target;
    int num1, num2;

#ifdef DEBUG
    printf("\nclose_sysfs_files()   START");
#endif   // DEBUG

    num1 = info.num_rawdata;

    for(i=0; i<num1; i++) {
    
        target = &info.rawdata_info[i];
        num2 = target->num_sysfs_fd;
        for(j=0; j<num2; j++)
            close(target->sysfs_fd[j]);
    }

#ifdef DEBUG
    printf("\nclose_sysfs_files()   FINISHED");
#endif   // DEBUG
};

ssize_t collect_timestamp(const int rawdata_fd) {

    int result;
    struct timespec time;

    //  0: Sucess
    // -1: Fail
    if(clock_gettime(CLOCK_REALTIME, &time) == -1)
        return -1;

    write(rawdata_fd, &time, sizeof(struct timespec));

    return result;
}

ssize_t collect_gpupower(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_POWER_MAX_STRLEN];

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

    return num_read_bytes;
}

ssize_t collect_gpufreq(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

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

    return num_read_bytes;
}

ssize_t collect_gpuutil(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_UTIL_MAX_STRLEN];

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

    return num_read_bytes;
}

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
