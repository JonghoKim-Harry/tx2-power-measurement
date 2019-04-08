#ifndef MEASUREMENT_INFO_H
#define MEASUREMENT_INFO_H

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <regex.h>

#define MAX_NUM_SYSFS_FD    8
#define ONE_SYSFS_FILE      1
#define TWO_SYSFS_FILES     2
#define THREE_SYSFS_FILES   3
#define FOUR_SYSFS_FILES    4
#define FIVE_SYSFS_FILES    5
#define SIX_SYSFS_FILES     6
#define SEVEN_SYSFS_FILES   7
#define EIGHT_SYSFS_FILES   8

struct sysfs_stat {

#ifdef DEBUG
    char column_name[256];
#endif   // DEBUG

    //
    ssize_t (*read_sysfs_func)(const struct sysfs_stat stat_info, const int rawdata_fd);
    int num_sysfs_fd;
    int max_strlen[MAX_NUM_SYSFS_FD];
    int sysfs_fd[MAX_NUM_SYSFS_FD];

    //
    ssize_t (*rawdata_to_stat_func)(const struct sysfs_stat info, const int rawdata_fd, const int stat_fd);
    int column_width;
    char stat_format[64];
};

#ifdef DEBUG
void print_stat_info(const struct sysfs_stat stat_info);
#endif   // DEBUG


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
    // TODO
    regex_t timestamp_pattern;

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


#endif   // MEASUREMENT_INFO_H
