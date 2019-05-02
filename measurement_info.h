#ifndef MEASUREMENT_INFO_H
#define MEASUREMENT_INFO_H

#include <time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <regex.h>

#include "powerlog.h"

#define MAX_NUM_SYSFS_FD    8

typedef struct rawdata_info_struct {

#ifdef DEBUG
    char column_name[256];
#endif   // DEBUG

    ssize_t (*func_read_rawdata)(const int rawdata_fd, ...);
    ssize_t (*func_rawdata_to_powerlog)(powerlog_struct *powerlog, const int rawdata_fd);
    ssize_t data_size;
    int num_sysfs_fd;
    int sysfs_fd[MAX_NUM_SYSFS_FD];
} rawdata_info_struct;

#ifdef DEBUG
void print_rawdata_info(const rawdata_info_struct rawdata_info);
#endif   // DEBUG


#define MAX_NUM_RAWDATA   16

typedef struct measurement_info_struct {

    // Caffe sleep request: in order to cool down CPUs
    struct timespec caffe_sleep_request;

    // Measurement interval in nanosecond
    struct timespec powertool_interval;

    // Cooldown period: in order to cool down GPU, etc.
    struct timespec cooldown_period;

    // Powerlog
    char powerlog_filename[128];
    int powerlog_fd;

    /*
     *  I will use execve() to run child command.
     *  Therefore, the child_cmd is an NULL-terminated array of arguments.
     *  Child will redirect its stderr message to caffelog file;
     *  thus, we store caffelog file name and fd
     */
    char **child_cmd;

    // Caffelog
    char caffelog_filename[128];
    int caffelog_fd;
    regex_t caffelog_pattern;

    /* Informations used by measure_rawdata() */
    struct tm *gmt_calendar_start_time;
    struct timespec gmt_start_time;

    /* 
     *  In order to use sysfs interface easily.
     *  See addsysfs(), read_sysfs(), rawdata_to_stat()
     */
    int num_rawdata;
    struct rawdata_info_struct rawdata_info[MAX_NUM_RAWDATA];
    int offset_2ndstat;
    char rawdata_filename[128];
    int rawdata_fd;
    int gpu_power_fd;
    char rawdata_print_format[256];
    char rawdata_scan_format[256];
    int rawdata_linesize;

    char header_raw[256];
    char stat_filename[128];
} measurement_info_struct;

void init_info(measurement_info_struct *info);

#ifdef DEBUG
void print_info(const struct measurement_info_struct info);
#endif   // DEBUG


#endif   // MEASUREMENT_INFO_H
