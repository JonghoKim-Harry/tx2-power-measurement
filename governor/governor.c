#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "governor.h"

static char orig_gpugov_name[GPU_GOVERNOR_NAME_LEN];

// Global variables and functions
struct gpugov_info_struct gpugov_info;
struct gpugov *curr_gpugov;
int fd_write_gpufreq;
int fd_read_gpufreq;
int fd_gpuutil;
int fd_gpupower;
int fd_emcutil;

static const size_t buffsize = TX2_SYSFS_GPU_FREQ_MAX_STRLEN;

void init_gpugovernor() {

    int fd;   // Temporal fd for files to be closed after initialization
    size_t bufflen;
    char buff[buffsize];
    const char *ptr;
    size_t count;
    int32_t freq_list[100];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    // GPU governor information: avaialable gpu frequencies
    count = 0;
    fd = open(TX2_SYSFS_AVAILABLE_GPUFREQ, O_RDONLY);
    lseek(fd, 0, SEEK_SET);
    do {
        memset(buff, 0, buffsize);
        bufflen = read(fd, buff, buffsize);
        for(ptr = buff; ptr < (buff + bufflen); ++ptr) {
            //rewind
            if(!isdigit((int)*ptr))
                lseek(fd, (ptr - buff - bufflen + 1), SEEK_CUR);
        }
        freq_list[count] = atoi(buff);
        ++count;
    } while(bufflen > 0);
    close(fd);
    gpugov_info.available_gpufreq = malloc(count * sizeof(int32_t));
    memcpy(gpugov_info.available_gpufreq, freq_list, count * sizeof(int32_t));
    gpugov_info.num_available_gpufreq = count;

    // GPU governor information: minimum gpu frequency
    fd = open(TX2_SYSFS_GPU_MINFREQ, O_RDONLY);
    lseek(fd, 0, SEEK_SET);
    memset(buff, 0, buffsize);
    read(fd, buff, buffsize);
    close(fd);
    gpugov_info.min_gpufreq = atoi(buff);

    // GPU governor information: maximum gpu frequency
    fd = open(TX2_SYSFS_GPU_MAXFREQ, O_RDONLY);
    lseek(fd, 0, SEEK_SET);
    memset(buff, 0, buffsize);
    read(fd, buff, buffsize);
    close(fd);
    gpugov_info.max_gpufreq = atoi(buff);

    // Make a list of available userspace gpu governors
    INIT_LIST_HEAD(&ondemand8050.node);

    // Initialize available gpu governors
    ondemand8050.init(NULL);

    // Store original GPU governor's name
    fd = open(TX2_SYSFS_GPU_GOVERNOR, O_RDWR);
    lseek(fd, 0, SEEK_SET);
    read(fd, orig_gpugov_name, GPU_GOVERNOR_NAME_LEN);
    lseek(fd, 0, SEEK_SET);
    write(fd, "userspace", 9);
    close(fd);

    // Open files and store fds
    //  * NOTE: Open this file after set governor as "userspace"
    fd_read_gpufreq = open(TX2_SYSFS_GPU_SET_FREQ, O_RDONLY);
    fd_write_gpufreq = open(TX2_SYSFS_GPU_SET_FREQ, O_WRONLY);
    if(fd_write_gpufreq < 0)
        perror("open() fail");
    fd_gpuutil = open(TX2_SYSFS_GPU_UTIL, O_RDONLY);
    fd_gpupower = open(TX2_SYSFS_GPU_POWER, O_RDONLY);
    fd_emcutil = open(TX2_SYSFS_EMC_UTIL, O_RDONLY);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   FINISH", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    return;
}

void start_gpugovernor(const char *gpugov_name) {

    set_gpufreq(gpugov_info.min_gpufreq);

    // Select by gpu governor name
    if(!strcmp(gpugov_name, "ondemand8050"))
        curr_gpugov = &ondemand8050;
    else
        perror("There is no gpu governor with the given name");

    return;
}

void finish_gpugovernor() {

    int fd;
    ssize_t num_written_bytes;

    close(fd_read_gpufreq);
    close(fd_gpuutil);
    close(fd_gpupower);
    close(fd_emcutil);

    // Restore original gpu governor
    fd = open(TX2_SYSFS_GPU_GOVERNOR, O_WRONLY);
    lseek(fd, 0, SEEK_SET);
    num_written_bytes = write(fd, orig_gpugov_name, strlen(orig_gpugov_name));
    if(num_written_bytes < 0)
        perror("write() fail");
    close(fd);
}

int32_t gpufreq_i(int level) {

    if(level < 0 || level >= gpugov_info.num_available_gpufreq)
        return -1;

    return gpugov_info.available_gpufreq[level];
}

int32_t level(int32_t gpufreq) {

    int ret;

    for(ret=0; ret<gpugov_info.num_available_gpufreq; ++ret) {
        if(gpufreq == gpugov_info.available_gpufreq[ret])
            return ret;
    }

    return -1;
}

int32_t get_gpufreq() {

    int32_t ret;
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    ssize_t num_read_bytes;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    lseek(fd_read_gpufreq, 0, SEEK_SET);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    num_read_bytes =
#endif   // DEBUG or DEBUG_GOVERNOR
    read(fd_read_gpufreq, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

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

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    ssize_t num_read_bytes;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    lseek(fd_gpuutil, 0, SEEK_SET);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    num_read_bytes =
#endif   // DEBUG or DEBUG_GOVERNOR
    read(fd_gpuutil, buff, TX2_SYSFS_GPU_UTIL_MAX_STRLEN);

    ret = atoi(buff);

    return ret;
}

int16_t get_gpupower() {

    int16_t ret;
    char buff[TX2_SYSFS_GPU_POWER_MAX_STRLEN];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    ssize_t num_read_bytes;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    lseek(fd_gpupower, 0, SEEK_SET);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    num_read_bytes =
#endif   // DEBUG or DEBUG_GOVERNOR
    read(fd_gpupower, buff, TX2_SYSFS_GPU_POWER_MAX_STRLEN);

    ret = atoi(buff);

    return ret;
}

ssize_t set_gpufreq(int32_t gpufreq) {

    ssize_t num_written_bytes;   // return value
    char buff[TX2_SYSFS_GPU_FREQ_MAX_STRLEN];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   gpufreq: %d", __func__, __FILE__, __LINE__, gpufreq);
#endif   // DEBUG or DEBUG_GOVERNOR

    lseek(fd_read_gpufreq, 0, SEEK_SET);
    snprintf(buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN, "%d", gpufreq);
    num_written_bytes = write(fd_write_gpufreq, buff, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    if(num_written_bytes < 0)
        perror("write() fail");
    printf("\n%s() in %s:%d   RETURN: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_GOVERNOR

    return num_written_bytes;
}

int16_t get_emcutil() {

    int16_t ret;
    char buff[TX2_SYSFS_EMC_UTIL_MAX_STRLEN];

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    ssize_t num_read_bytes;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    lseek(fd_emcutil, 0, SEEK_SET);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    num_read_bytes =
#endif   // DEBUG or DEBUG_GOVERNOR
    read(fd_emcutil, buff, TX2_SYSFS_EMC_UTIL_MAX_STRLEN);

    ret = atoi(buff);

    return ret;
}

