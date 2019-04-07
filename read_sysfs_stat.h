#ifndef READ_SYSFS_STAT_H
#define READ_SYSFS_STAT_H

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

#include "sysfs_stat.h"
#define MAX_NUM_SYSFS_DATA   16

struct measurement_info {

    //
    char powerlog_filename[128];
    int powerlog_fd;

    /*
     *  I will use execve() to run child command.
     *  Therefore, the child_cmd is an NULL-terminated array of arguments.
     *  Child will redirect its stderr message to caffelog file;
     *  thus, we store caffelog file name and fd
     */
    char **child_cmd;
    char caffelog_filename[128];
    int caffelog_fd;

    /* Informations used by measure_rawdata() */
    struct timespec start_time;
    int rawdata_fd;
    int gpu_power_fd;

    /* 
     *  In order to use sysfs interface easily.
     *  See addsysfs(), read_sysfs(), rawdata_to_stat()
     */
    int num_sysfs_data;
    struct sysfs_stat stat_info[MAX_NUM_SYSFS_DATA];
    char header_raw[256];

    /* Informations used by calculate_2ndstat() */
    int offset_2ndstat;
    char rawdata_filename[128];
    char rawdata_print_format[256];
    char rawdata_scan_format[256];
    int rawdata_linesize;
    char stat_filename[128];
};

#ifdef DEBUG
void print_info(const struct measurement_info info);
#endif   // DEBUG

void register_sysfs
    (struct measurement_info *info,
     ssize_t (*read_sysfs_func)(const struct sysfs_stat stat_info, const int rawdata_fd),
     ssize_t (*rawdata_to_stat_func)(const struct sysfs_stat, const int rawdata_fd, const int stat_fd),
     const char *column_name,
     const char *stat_format,
     const int num_sysfs_file, ...);

void close_sysfs(struct measurement_info info);

ssize_t sysfs_to_stat(const int stat_fd, const char *sysfs_filename, const char *stat_format);

ssize_t read_sysfs_1(const struct sysfs_stat stat_info, const int rawdata_fd);
ssize_t read_sysfs_2(const struct sysfs_stat stat_info, const int rawdata_fd);

ssize_t rawdata_to_stat_1(const struct sysfs_stat stat_info, const int rawdata_fd, const int stat_fd);
ssize_t rawdata_to_stat_2(const struct sysfs_stat stat_info, const int rawdata_fd, const int stat_fd);


#endif   // READ_SYSFS_STAT_H
