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
#include "privilege.h"

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

#ifdef TRACE_POWER
ssize_t collect_boardpower(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_BOARD_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_BOARD_POWER_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_BOARD_POWER_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t collect_socpower(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_SOC_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_SOC_POWER_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_SOC_POWER_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}

ssize_t collect_wifipower(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_WIFI_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_WIFI_POWER_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_WIFI_POWER_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}
#endif   // TRACE_POWER


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

#ifdef TRACE_CPU
ssize_t collect_allcpu_power(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_CPU_POWER_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_CPU_POWER_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_CPU_POWER_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}
#endif   // TRACE_CPU

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

void register_rawdata
    (measurement_info_struct *info,
     ssize_t (*func_read_rawdata)(const int rawdata_fd, ...),
     ssize_t (*func_rawdata_to_powerlog)(struct powerlog_struct *powerlog, const int rawdata_fd),
     const int num_sysfs_file, ...) {

    const int idx = info->num_rawdata;
    rawdata_info_struct *rawdata_info = &info->rawdata_info[idx];
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

    restore_root_privilege();

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
        printf("\nregister_rawdata()   rawdata_info->sysfs_fd[%d]: %d", i, rawdata_info->sysfs_fd[i]);
    }
#endif   // DEBUG

    // Register to info: the number of rawdata
    ++(info->num_rawdata);

    drop_root_privilege_temp();

#ifdef DEBUG
    printf("\nregister_rawdata()   FINISHED");
#endif   // DEBUG

    return;
}

#ifdef TRACE_TEMP
ssize_t collect_gputemp(const int rawdata_fd, const int sysfs_fd1) {

    ssize_t num_read_bytes;
    char buff[TX2_SYSFS_GPU_TEMP_MAX_STRLEN];

#ifdef DEBUG
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG

    lseek(sysfs_fd1, 0, SEEK_SET);
    num_read_bytes = read(sysfs_fd1, buff, TX2_SYSFS_GPU_TEMP_MAX_STRLEN);

    if(num_read_bytes < 0)
        return num_read_bytes;

    if(buff[num_read_bytes-1] == '\n' || buff[num_read_bytes-1] == EOF || buff[num_read_bytes-1] == ' ') {
        buff[num_read_bytes-1] = '\0';
        --num_read_bytes;
    }

    write(rawdata_fd, buff, num_read_bytes);
    write(rawdata_fd, WHITESPACE, TX2_SYSFS_GPU_TEMP_MAX_STRLEN - num_read_bytes);

#ifdef DEBUG
    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_read_bytes);
#endif   // DEBUG
    return num_read_bytes;
}
#endif   // TRACE_TEMP

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
