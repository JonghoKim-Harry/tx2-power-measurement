#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include "rawdata.h"
#include "measurement_info.h"
#include "tx2_sysfs_power.h"
#include "governor/governor.h"
#include "constants.h"
#include "default_values.h"

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

ssize_t collect_allpower(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_ALL_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_ALL_POWER_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_ALL_POWER_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
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

#ifdef TRACE_MEM
ssize_t collect_mempower(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_MEM_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_MEM_POWER_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_MEM_POWER_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t collect_emcfreq(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_EMC_FREQ_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_EMC_FREQ_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_EMC_FREQ_MAX_STRLEN- num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t collect_emcutil(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_EMC_UTIL_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_EMC_UTIL_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_EMC_UTIL_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}
#endif   // TRACE_MEM

void measure_rawdata(const int pid, const struct measurement_info_struct info) {

    int i;
    int flag_childexit;
    int diff_ns, diff_sec;
    const struct timespec sleep_request = info.powertool_interval;
    struct timespec sleep_remain;
    struct timespec cooldown_remain;
    struct timespec prev_time, curr_time;
    int child_status;
    struct rawdata_info_struct rawdata_info;

#ifdef DEBUG
    int num_read_bytes;
#endif   // DEBUG

    flag_childexit = 0;
    cooldown_remain = info.cooldown_period;

    while(1) {

        if(waitpid(pid, &child_status, WNOHANG))
            flag_childexit = 1;

        if(flag_childexit)
            prev_time = curr_time;

        if(clock_gettime(CLOCK_REALTIME, &curr_time) == -1) continue;

        if(flag_childexit) {
            diff_sec = curr_time.tv_sec - prev_time.tv_sec;
            diff_ns = curr_time.tv_nsec - prev_time.tv_nsec;

            cooldown_remain.tv_sec  -= diff_sec;
            cooldown_remain.tv_nsec -= diff_ns;

            while(cooldown_remain.tv_nsec >= ONE_SECOND_TO_NANOSECOND) {
                cooldown_remain.tv_nsec -= ONE_SECOND_TO_NANOSECOND;
                cooldown_remain.tv_sec  += 1;
            }

            while(cooldown_remain.tv_nsec < 0) {
                cooldown_remain.tv_nsec += ONE_SECOND_TO_NANOSECOND;
                cooldown_remain.tv_sec  -= 1;
            }

            if(cooldown_remain.tv_sec < 0)
                break;
        }

        if(info.userspace_gpugovernor)
            curr_gpugov->get_target_freq();

        for(i=0; i<info.num_rawdata; i++) {
            rawdata_info = info.rawdata_info[i];
            switch(rawdata_info.num_sysfs_fd) {
            case 0:
                rawdata_info.func_read_rawdata(info.rawdata_fd);
                break;
            case 1:
                rawdata_info.func_read_rawdata(info.rawdata_fd, rawdata_info.sysfs_fd[0]);
                break;
            case 2:
                rawdata_info.func_read_rawdata(info.rawdata_fd, rawdata_info.sysfs_fd[0], rawdata_info.sysfs_fd[1]);
                break;
            default:
                break;
            }
        }

        // Sleep enough
        while(nanosleep(&sleep_request, &sleep_remain) == -1)
            nanosleep(&sleep_remain, &sleep_remain);
    }

#ifdef DEBUG
    printf("\nmeasure_rawdata() does NOT have infinite loop");
#endif   // DEBUG

    close(info.rawdata_fd);
    close(info.caffelog_fd);
    close_sysfs_files(info);
}
