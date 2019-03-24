#ifndef SYSFS_TO_STAT_H
#define SYSFS_TO_STAT_H

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>

#define MAX_NUM_SYSFS_DATA   16

#define MAX_NUM_SYSFS_FD    8
#define ONE_SYSFS_FILE      1
#define TWO_SYSFS_FILES     2
#define THREE_SYSFS_FILES   3
#define FOUR_SYSFS_FILES    4
#define FIVE_SYSFS_FILES    5
#define SIX_SYSFS_FILES     6

struct measurement_info;

struct sysfs_stat_info {

#ifdef DEBUG
    char column_name[128];
#endif   // DEBUG

    /**/
    ssize_t (*read_sysfs_func)(const struct sysfs_stat_info stat_info, const int rawdata_fd);
    int num_sysfs_fd;
    int max_strlen[MAX_NUM_SYSFS_FD];
    int sysfs_fd[MAX_NUM_SYSFS_FD];

    /**/
    ssize_t (*rawdata_to_stat_func)(const struct sysfs_stat_info info, const int rawdata_fd, const int stat_fd);
    int column_width;
    char stat_format[64];
};

struct measurement_info {

    int argc;
    char **argv;

    /* Informations used by main() */
    char *child_cmd;

    /* Informations used by measure_rawdata() */
    struct timespec start_time;
    int rawdata_fd;
    int gpu_power_fd;

    /* 
     *  In order to use sysfs interface easily.
     *  See addsysfs(), read_sysfs(), rawdata_to_stat()
     */
    int num_sysfs_data;
    struct sysfs_stat_info stat_info[MAX_NUM_SYSFS_DATA];
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
void print_stat_info(const struct sysfs_stat_info stat_info);
#endif   // DEBUG

void register_sysfs
    (struct measurement_info *info,
     ssize_t (*read_sysfs_func)(const struct sysfs_stat_info stat_info, const int rawdata_fd),
     ssize_t (*rawdata_to_stat_func)(const struct sysfs_stat_info, const int rawdata_fd, const int stat_fd),
     const char *column_name,
     const char *stat_format,
     const int num_sysfs_file, ...);

void close_sysfs(struct measurement_info info);

ssize_t sysfs_to_stat(const int stat_fd, const char *sysfs_filename, const char *stat_format);

ssize_t read_sysfs_1(const struct sysfs_stat_info stat_info, const int rawdata_fd);
ssize_t read_sysfs_2(const struct sysfs_stat_info stat_info, const int rawdata_fd);

ssize_t rawdata_to_stat_1(const struct sysfs_stat_info stat_info, const int rawdata_fd, const int stat_fd);
ssize_t rawdata_to_stat_2(const struct sysfs_stat_info stat_info, const int rawdata_fd, const int stat_fd);


#endif   // SYSFS_TO_STAT_H
