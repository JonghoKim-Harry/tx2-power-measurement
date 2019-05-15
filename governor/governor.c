#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "governor.h"

static ssize_t gpugovernor_strlen;
static char original_gpugovernor[128];

int max_gpufreq = 1122000000;
int min_gpufreq =  140250000;
int fd_write_gpufreq;
int fd_gpufreq;
int fd_gpuutil;
int fd_gpupower;

const int32_t available_gpufreq[TX2_SYSFS_NUM_AVAILABLE_GPUFREQ] = {
     140250000,
     229500000,
     318750000,
     408000000,
     497250000,
     586500000,
     675750000,
     765000000,
     854250000,
     943500000,
    1032750000,
    1122000000,
    1211250000,
    1300500000
};

void start_gpugovernor() {

    int fd_gpugovernor;

    fd_gpuutil = open(TX2_SYSFS_GPU_UTIL, O_RDONLY);
    fd_gpupower = open(TX2_SYSFS_GPU_POWER, O_RDONLY);

    // Store GPU governor
    fd_gpugovernor = open(TX2_SYSFS_GPU_GOVERNOR, O_RDWR);
    lseek(fd_gpugovernor, 0, SEEK_SET);
    gpugovernor_strlen = read(fd_gpugovernor, original_gpugovernor, 128);
    lseek(fd_gpugovernor, 0, SEEK_SET);
    write(fd_gpugovernor, "userspace", 9);
    close(fd_gpugovernor);

    // NOTE: Open this file after set governor as "userspace"
    fd_gpufreq = open(TX2_SYSFS_GPU_SET_FREQ, O_RDONLY);
    fd_write_gpufreq = open(TX2_SYSFS_GPU_SET_FREQ, O_WRONLY);
    if(fd_write_gpufreq < 0)
        perror("open() fail");

    set_gpufreq(min_gpufreq);

    return;
}

void finish_gpugovernor() {

    int fd_gpugovernor;
    ssize_t num_written_bytes;

    close(fd_gpufreq);
    close(fd_gpuutil);
    close(fd_gpupower);

    fd_gpugovernor = open(TX2_SYSFS_GPU_GOVERNOR, O_WRONLY);
    lseek(fd_gpugovernor, 0, SEEK_SET);
    num_written_bytes = write(fd_gpugovernor, original_gpugovernor, gpugovernor_strlen);
    if(num_written_bytes < 0)
        perror("write() fail");
    close(fd_gpugovernor);
}

int32_t get_gpufreq() {

    ssize_t num_read_bytes;
    int32_t ret;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    lseek(fd_gpufreq, 0, SEEK_SET);
    num_read_bytes = read(fd_gpufreq, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

    ret = atoi(buff);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    if(num_read_bytes < 0)
        perror("read() fail");

    printf("\n%s() in %s:%d   read %ld bytes", __func__, __FILE__, __LINE__, num_read_bytes);
    printf("\n%s() in %s:%d   RETURN: %d\n---", __func__, __FILE__, __LINE__, ret);
#endif   // DEBUG or DEBUG_GOVERNOR

    return ret;
}

int16_t get_gpuutil() {

    int16_t ret;
    char buff[TX2_SYSFS_GPU_UTIL_MAX_STRLEN];

    lseek(fd_gpuutil, 0, SEEK_SET);
    read(fd_gpuutil, buff, TX2_SYSFS_GPU_UTIL_MAX_STRLEN);
    ret = atoi(buff);

    return ret;
}

int16_t get_gpupower() {

    int16_t ret;
    char buff[TX2_SYSFS_GPU_POWER_MAX_STRLEN];

    lseek(fd_gpupower, 0, SEEK_SET);
    read(fd_gpupower, buff, TX2_SYSFS_GPU_POWER_MAX_STRLEN);
    ret = atoi(buff);

    return ret;
}

ssize_t set_gpufreq(const int32_t gpufreq) {

    ssize_t num_written_bytes;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n___\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   gpufreq: %d", __func__, __FILE__, __LINE__, gpufreq);
#endif   // DEBUG or DEBUG_GOVERNOR


    lseek(fd_gpufreq, 0, SEEK_SET);
    snprintf(buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN, "%d", gpufreq);
    num_written_bytes = write(fd_write_gpufreq, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    if(num_written_bytes < 0)
        perror("write() fail");
    printf("\n%s() in %s:%d   RETURN: %ld\n---", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_GOVERNOR

    return num_written_bytes;
}
