#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "governor.h"
#include "privilege.h"

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
    int32_t freq;
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
        freq = atoi(buff);
        if(freq) {
            freq_list[count] = freq;
            ++count;
        }
    } while(bufflen > 0);
    close(fd);
    gpugov_info.available_gpufreq = malloc(count * sizeof(int32_t));
    memcpy(gpugov_info.available_gpufreq, freq_list, count * sizeof(int32_t));
    gpugov_info.num_available_gpufreq = count;
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   @gpugov_info.num_available_gpufreq: %ld", __func__, __FILE__, __LINE__, gpugov_info.num_available_gpufreq);
    printf("\n%s() in %s:%d   @gpugov_info.available_gpufreq:", __func__, __FILE__, __LINE__);
    for(count=0; count<gpugov_info.num_available_gpufreq; count++)
       printf("\n%s() in %s:%d   %d", __func__, __FILE__, __LINE__, gpugov_info.available_gpufreq[count]);
#endif   // DEBUG or DEBUG_GOVERNOR

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
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   @gpugov_info.max_gpufreq: %d", __func__, __FILE__, __LINE__, gpugov_info.max_gpufreq);
#endif   // DEBUG or DEBUG_GOVERNOR

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

void select_gpugovernor(const char *gpugov_name, void *data) {

    set_gpufreq(gpugov_info.min_gpufreq);

    // Select by gpu governor name
    if(!strcmp(gpugov_name, "do_nothing"))
        curr_gpugov = &do_nothing;
    else if(!strcmp(gpugov_name, "scale_100"))
        curr_gpugov = &scale_100;
    else if(!strcmp(gpugov_name, "scale_10"))
        curr_gpugov = &scale_10;
    else if(!strcmp(gpugov_name, "scale_always"))
        curr_gpugov = &scale_always;
    else if(!strcmp(gpugov_name, "ondemand8050"))
        curr_gpugov = &ondemand8050;
    else if(!strcmp(gpugov_name, "policy_0_or_not"))
        curr_gpugov = &policy_0_or_not;
    else if(!strcmp(gpugov_name, "maxmin_cap_b"))
        curr_gpugov = &maxmin_cap_b;
    else if(!strcmp(gpugov_name, "cnngov_a"))
        curr_gpugov = &cnngov_a;
    else if(!strcmp(gpugov_name, "cnngov_b"))
        curr_gpugov = &cnngov_b;
    else if(!strcmp(gpugov_name, "cnngov_c"))
        curr_gpugov = &cnngov_c;
    else if(!strcmp(gpugov_name, "cnngov_c_30"))
        curr_gpugov = &cnngov_c_30;
    else if(!strcmp(gpugov_name, "cnngov_c_40"))
        curr_gpugov = &cnngov_c_40;
    else if(!strcmp(gpugov_name, "cnngov_d"))
        curr_gpugov = &cnngov_d;
    else if(!strcmp(gpugov_name, "cnngov_20190919"))
        curr_gpugov = &cnngov_20190919;
    else if(!strcmp(gpugov_name, "cnngov_20190920"))
        curr_gpugov = &cnngov_20190920;
    else if(!strcmp(gpugov_name, "slow_scale"))
        curr_gpugov = &slow_scale;
    else if(!strcmp(gpugov_name, "cnngov_e"))
        curr_gpugov = &cnngov_e;
    else if(!strcmp(gpugov_name, "hilo"))
        curr_gpugov = &hilo;
    else if(!strcmp(gpugov_name, "pingpong"))
        curr_gpugov = &pingpong;
    else if(!strcmp(gpugov_name, "emc_conservative"))
        curr_gpugov = &emc_conservative;
    else
        perror("There is no gpu governor with the given name");

    curr_gpugov->init(data);

    return;
}

void finish_gpugovernor() {

    int fd;
    ssize_t num_written_bytes;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR

    close(fd_write_gpufreq);
    close(fd_read_gpufreq);
    close(fd_gpuutil);
    close(fd_gpupower);
    close(fd_emcutil);

    // Restore original gpu governor
    push_privilege();
    get_root_privilege();

    fd = open(TX2_SYSFS_GPU_GOVERNOR, O_WRONLY);
    lseek(fd, 0, SEEK_SET);
    num_written_bytes = write(fd, orig_gpugov_name, strlen(orig_gpugov_name));
    if(num_written_bytes < 0)
        perror("write() fail");
    close(fd);

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   fd: %d", __func__, __FILE__, __LINE__, fd);
    printf("\n%s() in %s:%d   orig_gpugov_name: %s", __func__, __FILE__, __LINE__, orig_gpugov_name);
    printf("\n%s() in %s:%d   num_written_bytes: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_GOVERNOR

    pop_privilege();

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   FINISHED", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
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

int64_t get_emcutil() {

    int64_t ret;
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

int32_t gpufreq_level_to_hz(int level) {

    if(level < 0 || level >= gpugov_info.num_available_gpufreq)
        return -1;

    return gpugov_info.available_gpufreq[level];
}

int gpufreq_hz_to_level(int32_t gpufreq) {

    int ret;

    for(ret=0; ret<gpugov_info.num_available_gpufreq; ++ret) {
        if(gpufreq == gpugov_info.available_gpufreq[ret])
            return ret;
    }

    return -1;
}

/**
 *  Return frequency equal or greater than n% of given frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scaling down factor (integer, percent)
 */
int32_t scale_down_by_n(const int32_t gpufreq, int n) {

    const int64_t target_x100 = (int64_t)gpufreq * (100-n);
    const int num_levels = gpugov_info.num_available_gpufreq;
    int level;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    int32_t ret;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   @gpufreq: %d", __func__, __FILE__, __LINE__, gpufreq);
    printf("\n%s() in %s:%d   @n: %d", __func__, __FILE__, __LINE__, n);
#endif   // DEBUG or DEBUG_GOVERNOR

    for(level=0; level<num_levels; level++) {
        if(100 * (int64_t)gpugov_info.available_gpufreq[level] >= target_x100) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
            ret = gpugov_info.available_gpufreq[level];
            printf("\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, ret);
#endif   // DEBUG or DEBUG_GOVERNOR
            return gpugov_info.available_gpufreq[level];
        }
    }

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
            printf("\n%s() in %s:%d   RETURNED: -1", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
    return -1;
}

/**
 *  Return frequency equal or less than n% of given frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scale up factor (integer, percent)
 */
int32_t scale_up_by_n(const int32_t gpufreq, int n) {

    const int64_t target_x100 = (int64_t)gpufreq * (100+n);
    const int num_levels = gpugov_info.num_available_gpufreq;
    int level;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    int32_t ret;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   @gpufreq: %d", __func__, __FILE__, __LINE__, gpufreq);
    printf("\n%s() in %s:%d   @n: %d", __func__, __FILE__, __LINE__, n);
#endif   // DEBUG or DEBUG_GOVERNOR

    for(level=(num_levels-1); level>=0; level--) {
    
        if(100 * (int64_t)gpugov_info.available_gpufreq[level] <= target_x100) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
            ret = gpugov_info.available_gpufreq[level];
            printf("\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, ret);
#endif   // DEBUG or DEBUG_GOVERNOR
            return gpugov_info.available_gpufreq[level];
        }
    }

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
            printf("\n%s() in %s:%d   RETURNED: -1", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
    return -1;
}

/**
 *  Scale down given frequency by n% of maximum frequency.
 *  Return frequency equal or greater than the target frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scaling down factor (integer, percent)
 */
int32_t scale_down_by_n_of_max(const int32_t gpufreq, int n) {

    const int64_t target_x100 = (100 * (int64_t)gpufreq) - (n * (int64_t)gpugov_info.max_gpufreq);
    const int num_levels = gpugov_info.num_available_gpufreq;
    int level;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    int32_t ret;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   @gpufreq: %d", __func__, __FILE__, __LINE__, gpufreq);
    printf("\n%s() in %s:%d   @n: %d", __func__, __FILE__, __LINE__, n);
#endif   // DEBUG or DEBUG_GOVERNOR

    for(level=0; level<num_levels; level++) {
        if(100 * (int64_t)gpugov_info.available_gpufreq[level] >= target_x100) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
            ret = gpugov_info.available_gpufreq[level];
            printf("\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, ret);
#endif   // DEBUG or DEBUG_GOVERNOR
            return gpugov_info.available_gpufreq[level];
        }
    }

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
            printf("\n%s() in %s:%d   RETURNED: -1", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
    return -1;
}

/**
 *  Scale up given frequency by n% of maximum frequency.
 *  Return frequency equal or less than the target frequency
 *  @gpufreq: Given frequency in Hz
 *  @n: Scale up factor (integer, percent)
 */
int32_t scale_up_by_n_of_max(const int32_t gpufreq, int n) {

    const int64_t target_x100 = (100 * (int64_t)gpufreq) + (n * (int64_t)gpugov_info.max_gpufreq);
    const int num_levels = gpugov_info.num_available_gpufreq;
    int level;

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    int32_t ret;
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
    printf("\n%s() in %s:%d   @gpufreq: %d", __func__, __FILE__, __LINE__, gpufreq);
    printf("\n%s() in %s:%d   @n: %d", __func__, __FILE__, __LINE__, n);
    printf("\n%s() in %s:%d   target_x100: %ld", __func__, __FILE__, __LINE__, target_x100);
#endif   // DEBUG or DEBUG_GOVERNOR

    for(level=(num_levels-1); level>=0; level--) {
        if(100 * (int64_t)gpugov_info.available_gpufreq[level] <= target_x100) {
#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
            ret = gpugov_info.available_gpufreq[level];
            printf("\n%s() in %s:%d   RETURNED: %d", __func__, __FILE__, __LINE__, ret);
#endif   // DEBUG or DEBUG_GOVERNOR
            return gpugov_info.available_gpufreq[level];
        }
    }

#if defined(DEBUG) || defined(DEBUG_GOVERNOR)
    printf("\n%s() in %s:%d   RETURNED: -1", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_GOVERNOR
    return -1;
}

