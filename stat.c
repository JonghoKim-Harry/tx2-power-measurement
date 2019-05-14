#include <unistd.h>
#include <string.h>
#include <time.h>

#include "tx2_sysfs_power.h"
#include "constants.h"
#include "default_values.h"
#include "stat.h"

off_t print_expinfo(const int stat_fd, const measurement_info_struct info) {

    char buff1[MAX_BUFFLEN], buff2[MAX_BUFFLEN];
    int buff1_len, buff2_len;
    const char **ptr;
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
    strftime(buff1, MAX_BUFFLEN, "%Y-%m-%d %H:%M:%S", info.calendar_start_time);
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
    for(ptr = &info.child_cmd[1]; *ptr != NULL; ptr++) {
        strcat(buff1, " ");
        strcat(buff1, *ptr);
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

    // Raw data format: Column name of the statistics table
    write(stat_fd, "\n", 1);

    return lseek(stat_fd, 0, SEEK_CUR);
}

static const char separation_line1[MAX_BUFFLEN] = "\n__________________________________________________________________________________________________________________________________________________________________________";
static const char separation_line2[MAX_BUFFLEN] = "\n--------------------------------------------------------------------------------------------------------------------------------------------------------------------------";

ssize_t print_header_raw(const int stat_fd, const measurement_info_struct info) {

    ssize_t num_written_bytes, total_written_bytes;
    char buff[MAX_COLWIDTH];
    int i;
    int buff_len;

    total_written_bytes = 0;

    num_written_bytes = write(stat_fd, separation_line1, strlen(separation_line1));
    total_written_bytes += num_written_bytes;

    num_written_bytes = write(stat_fd, "\n", 1);
    total_written_bytes += num_written_bytes;

    for(i=0; i<info.num_stat; i++) {

        num_written_bytes = write(stat_fd, "  ", 2);
        total_written_bytes += num_written_bytes;

        buff_len = snprintf(buff, MAX_COLWIDTH, "%*s", info.stat_info[i].colwidth, info.stat_info[i].colname);
        num_written_bytes = write(stat_fd, buff, buff_len);
        total_written_bytes += num_written_bytes;
    }

    num_written_bytes = write(stat_fd, separation_line2, strlen(separation_line2));
    total_written_bytes += num_written_bytes;

    return total_written_bytes;
}
