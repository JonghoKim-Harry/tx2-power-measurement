#include <stdio.h>
#include <unistd.h>

#include "tx2_sysfs_power.h"
#include "constants.h"
#include "log_to_stat.h"

// Header Row
//void register_column(measurement_info_struct *info, const char *colname, const int colwidth) {}

// Powerlog to Statistics
ssize_t timestamp_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.timestamp: struct timespec
    ssize_t num_written_bytes;
    struct tm *calendar_timestamp;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH], buff3[MAX_COLWIDTH];
    int buff3_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    calendar_timestamp = localtime(&powerlog.timestamp.tv_sec);
    strftime(buff1, MAX_COLWIDTH, "%H:%M:%S", calendar_timestamp);
    snprintf(buff2, MAX_COLWIDTH, "%s.%09ld", buff1, powerlog.timestamp.tv_nsec);
    buff3_len = snprintf(buff3, MAX_COLWIDTH, "%*s", colwidth, buff2);
    num_written_bytes = write(stat_fd, buff3, buff3_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    if(num_written_bytes < 0)
        perror("\nError while write()");

    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    return num_written_bytes;
}

ssize_t gpupower_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.gpu_power: mW
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth,  powerlog.gpu_power);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    if(num_written_bytes < 0)
        perror("\nError while write()");

    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t gpufreq_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.gpu_freq: MHz
    ssize_t num_written_bytes;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*d", colwidth, powerlog.gpu_freq);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    if(num_written_bytes < 0)
        perror("\nError while write()");

    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t gpuutil_to_stat(const int stat_fd, const int colwidth, const powerlog_struct powerlog) {

    // @powerlog.gpu_util: x10%
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;
    int upper, lower;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    upper = powerlog.gpu_util / 10;
    lower = powerlog.gpu_util % 10;

    snprintf(buff1, MAX_COLWIDTH, "%3d.%1d", upper, lower);
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    if(num_written_bytes < 0)
        perror("\nError while write()");

    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

// Powerlog Summary to Statistics
ssize_t elapsedtime_to_stat(const int stat_fd, const int colwidth, const powerlog_summary_struct powerlog_summary) {

    // return value
    ssize_t num_written_bytes;
    time_t sec;
    int32_t nsec;
    char buff[MAX_COLWIDTH];
    int buff_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    sec = powerlog_summary.finish_timestamp.tv_sec - powerlog_summary.start_timestamp.tv_sec;
    nsec = powerlog_summary.finish_timestamp.tv_nsec - powerlog_summary.start_timestamp.tv_nsec;

    if(nsec < 0) {
        --sec;
        nsec += ONE_SECOND_TO_NANOSECOND;
    }

    buff_len = snprintf(buff, MAX_COLWIDTH, "%*ld%09ld", (colwidth-9), sec, nsec);
    num_written_bytes = write(stat_fd, buff, buff_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    if(num_written_bytes < 0)
        perror("\nError while write()");

    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

ssize_t gpuenergy_to_stat(const int stat_fd, const int colwidth, const powerlog_summary_struct powerlog_summary) {

    // return value
    ssize_t num_written_bytes;
    char buff1[MAX_COLWIDTH], buff2[MAX_COLWIDTH];
    int buff2_len;

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    printf("\n%s() in %s:%d   START", __func__, __FILE__, __LINE__);
#endif   // DEBUG or DEBUG_LOG_TO_STAT

    snprintf(buff1, MAX_COLWIDTH, "%ld.%06ld%ld", powerlog_summary.gpu_energy_uJ, powerlog_summary.gpu_energy_pJ, powerlog_summary.gpu_energy_dotone_pJ);
    buff2_len = snprintf(buff2, MAX_COLWIDTH, "%*s", colwidth, buff1);
    num_written_bytes = write(stat_fd, buff2, buff2_len);

#if defined(DEBUG) || defined(DEBUG_LOG_TO_STAT)
    if(num_written_bytes < 0)
        perror("\nError while write()");

    printf("\n%s() in %s:%d   returned: %ld", __func__, __FILE__, __LINE__, num_written_bytes);
#endif   // DEBUG or DEBUG_LOG_TO_STAT
    return num_written_bytes;
}

// TODO
// ssize_t powerevent_to_stat(const int stat_fd, const int colwidth, const powerlog_summary_struct powerlog_summary) {}

// Caffelog to Statistics
// TODO
ssize_t caffeevent_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog) {

}

// TODO
//ssize_t batchnum_to_stat(const int stat_fd, const int colwidth, const caffelog_struct caffelog) {}
