#ifndef MEASUREMENT_INFO_H
#define MEASUREMENT_INFO_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <regex.h>

#define MAX_NUM_SYSFS_FD     8
#define MAXNUM_RAWDATA      20
#define RAWDATA_BUFFSIZE    64

typedef struct powerlog_struct {

    struct timespec gmt_timestamp;

    int16_t gpu_power;         // mW
    int16_t gpu_freq;          // MHz
    int16_t gpu_util;          // x10%

#ifdef TRACE_CPU
    int16_t allcpu_power;      // mW
    int group0_cpus[6];
    int16_t cpu_group0_freq;   // MHz
    int16_t cpu_group1_freq;   // MHz
#endif   // TRACE_CPU

#ifdef TRACE_DDR
    int32_t mem_power;         // mW
    int16_t emc_freq;          // MHz
#endif   // TRACE_DDR

    int num_rawdata;
    uint8_t rawdata[MAXNUM_RAWDATA][RAWDATA_BUFFSIZE];
} powerlog_struct;

typedef struct powerlog_summary_struct {

    struct timespec gmt_start_timestamp, gmt_finish_timestamp;
    struct powerlog_struct last_powerlog;
    int                     num_powerlog;

    // GPU energy
    int32_t gpu_energy_Wh;
    int64_t gpu_energy_pWh;
    int64_t gpu_energy_remainder;

#ifdef TRACE_CPU
    int32_t allcpu_energy_Wh;
    int64_t allcpu_energy_pWh;
    int32_t avg_allcpu_power;    // mW
#endif   // TRACE_CPU

#ifdef TRACE_DDR
    int32_t mem_energy_Wh;
    int64_t mem_energy_pWh;
    int32_t avg_mem_power;       // mW
#endif   // TRACE_DDR
} powerlog_summary_struct;

int update_powerlog_summary(const powerlog_struct powerlog,
                            powerlog_summary_struct *powerlog_summary);

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
