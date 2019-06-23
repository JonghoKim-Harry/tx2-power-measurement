#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "tx2_sysfs_power.h"
#include "constants.h"
#include "default_values.h"
#include "stat.h"

off_t print_expinfo(const int stat_fd, const measurement_info_struct info) {

    char buff1[MAX_BUFFLEN], buff2[MAX_BUFFLEN];
    int buff1_len, buff2_len;
    int fd;
    const char **ptrptr;
    char *ptr;
    struct timeval walltime;
    struct tm *calendar;

    // Point to the start of the file
    lseek(stat_fd, 0, SEEK_SET);

    // Start logging to statistics file
    buff1_len = snprintf(buff1, MAX_BUFFLEN, "            JETSON TX2 POWER MEASUREMENT STATS\n");
    write(stat_fd, buff1, buff1_len);

    buff1_len = snprintf(buff1, MAX_BUFFLEN, "\n\n Measurement Informations");
    write(stat_fd, buff1, buff1_len);

    // Walltime: GMT
    strftime(buff1, MAX_BUFFLEN, "%Y-%m-%d %H:%M:%S", &info.calendar_start_time);
    buff2_len = snprintf(buff2, MAX_BUFFLEN, "\n   * Start measurement at %s (GMT)", buff1);
    write(stat_fd, buff2, buff2_len);

    // Walltime: Korea Timezone
    walltime.tv_sec = info.start_time.tv_sec + SECONDS_GMT_TO_KOREA_TIME;
    calendar = localtime(&walltime.tv_sec);
    strftime(buff1, MAX_BUFFLEN, "%Y-%m-%d %H:%M:%S", calendar);
    buff2_len = snprintf(buff2, MAX_BUFFLEN, "\n   * Start measurement at %s (Korea Timezone)", buff1);
    write(stat_fd, buff2, buff2_len);

    // Child Command
    strcpy(buff1, info.child_cmd[0]);
    for(ptrptr = &info.child_cmd[1]; *ptrptr != NULL; ptrptr++) {
        strcat(buff1, " ");
        strcat(buff1, *ptrptr);
    }

    buff2_len = snprintf(buff2, MAX_BUFFLEN, "\n   * Running:  %s", buff1);
    write(stat_fd, buff2, buff2_len);

    // Caffe sleep period
    buff1_len = snprintf(buff1, MAX_BUFFLEN, "\n   * Sleep:    %ld.%09ld seconds before Caffe START",
                        info.caffe_sleep_request.tv_sec,
                        info.caffe_sleep_request.tv_nsec);
    write(stat_fd, buff1, buff1_len);

    // Measurement Interval
    buff1_len = snprintf(buff1, MAX_BUFFLEN, "\n   * Interval: %ld.%09ld seconds",
                        info.powertool_interval.tv_sec,
                        info.powertool_interval.tv_nsec);
    write(stat_fd, buff1, buff1_len);

    // Cooldown Period
    buff1_len = snprintf(buff1, MAX_BUFFLEN, "\n   * Cooldown: %ld.%09ld seconds after  Caffe FINISH",
                        info.cooldown_period.tv_sec,
                        info.cooldown_period.tv_nsec);
    write(stat_fd, buff1, buff1_len);

    // GPU Informations
    buff1_len = snprintf(buff1, MAX_BUFFLEN, "\n\n GPU Informations");
    write(stat_fd, buff1, buff1_len);

    // GPU Governor
    strncpy(buff1, "\0", MAX_BUFFLEN);
    fd = open(TX2_SYSFS_GPU_GOVERNOR, O_RDONLY);
    lseek(fd, 0, SEEK_SET);
    buff1_len = read(fd, buff1, MAX_BUFFLEN);
    for(ptr = buff1; ptr < buff1 + buff1_len; ptr++) {
        if(*ptr == '\n')
            *ptr = '\0';
    }

    if(!strcmp(buff1, "userspace"))
        buff2_len = snprintf(buff2, MAX_BUFFLEN, "\n   * GPU Governor: %s (%s)", buff1, info.gpugov_name);
    else
        buff2_len = snprintf(buff2, MAX_BUFFLEN, "\n   * GPU Governor: %s", buff1);
    write(stat_fd, buff2, buff2_len);
    close(fd);

    // GPU MAX/MIN Frequency
    strncpy(buff1, "\0", MAX_BUFFLEN);
    fd = open(TX2_SYSFS_GPU_MAXFREQ, O_RDONLY);
    lseek(fd, 0, SEEK_SET);
    buff1_len = read(fd, buff1, MAX_BUFFLEN);
    for(ptr = buff1; ptr < buff1 + buff1_len; ptr++) {
        if(*ptr == '\n')
            *ptr = '\0';
    }
    buff2_len = snprintf(buff2, MAX_BUFFLEN, "\n   * GPU Max Frequency: %s Hz", buff1);
    write(stat_fd, buff2, buff2_len);
    close(fd);
    strncpy(buff1, "\0", MAX_BUFFLEN);
    fd = open(TX2_SYSFS_GPU_MINFREQ, O_RDONLY);
    lseek(fd, 0, SEEK_SET);
    buff1_len = read(fd, buff1, MAX_BUFFLEN);
    for(ptr = buff1; ptr < buff1 + buff1_len; ptr++) {
        if(*ptr == '\n')
            *ptr = '\0';
    }
    buff2_len = snprintf(buff2, MAX_BUFFLEN, "\n   * GPU Min Frequency: %s Hz", buff1);
    write(stat_fd, buff2, buff2_len);
    close(fd);

    return lseek(stat_fd, 0, SEEK_CUR);
}

ssize_t print_header_raw(const int stat_fd, const measurement_info_struct info) {

    ssize_t num_written_bytes, total_written_bytes;
    char buff[MAX_COLWIDTH];
    int i;
    int buff_len;

    total_written_bytes = 0;

    num_written_bytes = write(stat_fd, "\n\n", 2);
    total_written_bytes += num_written_bytes;

    for(i=0; i<info.num_stat; i++) {

        num_written_bytes = write(stat_fd, "  ", 2);
        total_written_bytes += num_written_bytes;

        buff_len = snprintf(buff, MAX_COLWIDTH, "%*s", info.stat_info[i].colwidth, info.stat_info[i].colname);
        num_written_bytes = write(stat_fd, buff, buff_len);
        total_written_bytes += num_written_bytes;
    }

    return total_written_bytes;
}
