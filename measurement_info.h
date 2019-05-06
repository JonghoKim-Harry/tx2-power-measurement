#ifndef MEASUREMENT_INFO_H
#define MEASUREMENT_INFO_H

#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <stdarg.h>
#include <regex.h>

// sysfs fds
#define MAX_NUM_SYSFS_FD     8
#define  NO_SYSFS_FILE       0
#define ONE_SYSFS_FILE       1
#define TWO_SYSFS_FILES      2
#define THREE_SYSFS_FILES    3
#define FOUR_SYSFS_FILES     4
#define FIVE_SYSFS_FILES     5
#define SIX_SYSFS_FILES      6
#define SEVEN_SYSFS_FILES    7
#define EIGHT_SYSFS_FILES    8

#define MAX_NUM_RAWDATA     20
#define MAX_NUM_STAT        20
#define RAWDATA_BUFFSIZE    64
#define MAX_COLWIDTH        40

#define NUM_CPUS             6

struct rawdata_info_struct;
struct powerlog_struct;

/*
 *  Rawdata
 */
typedef struct rawdata_info_struct {

    ssize_t (*func_read_rawdata)(const int rawdata_fd, ...);
    ssize_t (*func_rawdata_to_powerlog)(struct powerlog_struct *powerlog, const int rawdata_fd);
    ssize_t data_size;
    int num_sysfs_fd;
    int sysfs_fd[MAX_NUM_SYSFS_FD];
} rawdata_info_struct;


enum logtype_t {
    LOGTYPE_NA = 0,
    LOGTYPE_POWERLOG,
    LOGTYPE_POWERLOG_SUMMARY,
    LOGTYPE_CAFFELOG,
    LOGTYPE_TEGRALOG,
    NUM_LOGTYPES
};


/*
 *  Powerlog
 */
typedef struct powerlog_struct {

    struct timespec timestamp;   // GMT

    int16_t gpu_power;           // mW
    int16_t gpu_freq;            // MHz
    int16_t gpu_util;            // x10%

#ifdef TRACE_CPU
    int16_t allcpu_power;        // mW
    int group0_cpus[6];
    int16_t cpu_group0_freq;     // MHz
    int16_t cpu_group1_freq;     // MHz
#endif   // TRACE_CPU

#ifdef TRACE_DDR
    int32_t mem_power;           // mW
    int16_t emc_freq;            // MHz
#endif   // TRACE_DDR

    int num_rawdata;
    //uint8_t rawdata[MAX_NUM_RAWDATA][RAWDATA_BUFFSIZE];
} powerlog_struct;

typedef struct powerlog_summary_struct {

    struct timespec start_timestamp, finish_timestamp;
    struct powerlog_struct *last_powerlog;
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


/*
 *  Caffelog
 */
// match[0]: the whole match. Do not care about it
// match[1]: caffelog timestamp
// match[2]: caffelog event
#define CAFFELOG_PATTERN                                                 \
    "[[:alpha:][:space:]]*"                                              \
    "([[:digit:]]{2}:[[:digit:]]{2}:[[:digit:]]{2}[.][[:digit:]]{6})"    \
    "[^\]]*\][[:space:]]*"                                               \
    "([^[:space:]].*)"

typedef struct caffelog_struct {
    struct timespec timestamp;   // GMT

    /*
     *  HMS: Hour, Minute, Second
     *  Note that year, month, day are ignored to be 0
     */
    struct tm date_hms;   // GMT, HH:MM:SS
    char event[128];
} caffelog_struct;


/*
 *  Tegralog
 */
//  REGEX
//  regmatch[0] is the whole match. Do not care about it
//  regmatch[1]:   CPU0 utilization
//  regmatch[2]:   CPU0 frequency
//  regmatch[3]:   CPU1 status: utilization and frequency
//  regmatch[4]:   CPU2 status: utilization and frequency
//  regmatch[5]:   CPU3 status: utilization and frequency
//  regmatch[6]:   CPU4 status: utilization and frequency
//  regmatch[7]:   CPU5 status: utilization and frequency
//  regmatch[8]:   EMC UTIL
//  regmatch[9]:   EMC FREQ
//  regmatch[10]:  GPU UTIL
//  regmatch[11]:  GPU FREQ
//  regmatch[12]:  GPU Temparature
//  regmatch[13]:  CPU Power
//  regmatch[14]:  GPU Power
//  regmatch[15]:  DDR Power
#define TEGRALOG_PATTERN                                                   \
    "RAM[[:space:]]+[[:digit:]]+/[[:digit:]]+MB[[:space:]]+"               \
    "[(]lfb[[:space:]]+[[:digit:]]+x[[:digit:]]+MB[)][[:space:]]+"         \
    "CPU[[:space:]]+[\[]([[:digit:]]+)%@([[:digit:]]+),"                   \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off),"                                    \
    "([[:digit:]]+%@[[:digit:]]+|off)[\]][[:space:]]+"                     \
    "EMC_FREQ[[:space:]]+([[:digit:]]+)%@([[:digit:]]+)[[:space:]]+"       \
    "GR3D_FREQ[[:space:]]+([[:digit:]]+)%@([[:digit:]]+)[[:space:]]+"      \
    "APE[[:space:]]+[[:digit:]]+[[:space:]]+"                              \
    "BCPU@[.[:digit:]]+C[[:space:]]+"                                      \
    "MCPU@[.[:digit:]]+C[[:space:]]+"                                      \
    "GPU@([.[:digit:]]+)+C[[:space:]]+"                                    \
    "PLL@[.[:digit:]]+C[[:space:]]+"                                       \
    "Tboard@[.[:digit:]]+C[[:space:]]+"                                    \
    "Tdiode@[.[:digit:]]+C[[:space:]]+"                                    \
    "PMIC@[.[:digit:]]+C[[:space:]]+"                                      \
    "thermal@[.[:digit:]]+C[[:space:]]+"                                   \
    "VDD_IN[[:space:]]+[[:digit:]]+/[[:digit:]]+[[:space:]]+"              \
    "VDD_CPU[[:space:]]+([[:digit:]]+)/[[:digit:]]+[[:space:]]+"           \
    "VDD_GPU[[:space:]]+([[:digit:]]+)/[[:digit:]]+[[:space:]]+"           \
    "VDD_SOC[[:space:]]+[[:digit:]]+/[[:digit:]]+[[:space:]]+"             \
    "VDD_WIFI[[:space:]]+[[:digit:]]+/[[:digit:]]+[[:space:]]+"            \
    "VDD_DDR[[:space:]]+([[:digit:]]+)/[[:digit:]]+"                       \
    "[[:space:]]*"

// Tegralog Example:
// RAM 1911/7854MB (lfb 224x4MB) CPU [0%@345,off,off,0%@345,0%@345,0%@345] EMC_FREQ 0%@665 GR3D_FREQ 0%@140 APE 150 BCPU@32C MCPU@32C GPU@31.5C PLL@32C Tboard@28C Tdiode@28.75C PMIC@100C thermal@31.8C VDD_IN 1182/1182 VDD_CPU 152/152 VDD_GPU 152/152 VDD_SOC 381/381 VDD_WIFI 0/0 VDD_DDR 192/192
typedef struct tegralog_struct {
    // Time (ns)
    // Note that the 'tegrastats' do NOT give any time informations
    int64_t time;

    // Utilizations (%) & Frequencies (MHz)
    int cpu_util[NUM_CPUS], cpu_freq[NUM_CPUS];
    int gpu_util;
    int gpu_freq;
    int emc_util;
    int emc_freq;
    // int ape_freq;

    // Power Measurement I (mW)
    int gpu_cur_power, gpu_avg_power;
    int soc_cur_power, soc_avg_power;
    int wifi_cur_power, wifi_avg_power;

    // Power Measurement II (mW)
    int ddr_cur_power, ddr_avg_power;
    int all_cur_power, all_avg_power;
    int cpu_cur_power, cpu_avg_power;

    // Thermal Informations (degree Celsius)
    //int bcpu_thinfo;   // BCPU Cluster: 4x ARM Cortex A57 CPUs
    //int mcpu_thinfo;   // MCPU Cluster: 2x NVidia Denver2 CPUs
    int gpu_thinfo;
    //int pll_thinfo;
    //int tboard;
    //int tdiode;
    //int pmic_thinfo;
    //int thermal_thinfo;

    // RAM Informations
    //int ram_use;
    //int ram_avail;
    //int ram_lfb;

} tegralog_struct;

/*
 *  Statistics
 */
typedef struct stat_info_struct {

    char colname[MAX_COLWIDTH];
    int  colwidth;
    enum logtype_t logtype;
    ssize_t (*func_log_to_stat)(const int stat_fd, ...);
} stat_info_struct;


/*
 *  Measurement Information
 */
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

    //  I will use execve() to run child command.
    //  Therefore, the child_cmd is an NULL-terminated array of arguments.
    //  Child will redirect its stderr message to caffelog file;
    //  thus, we store caffelog file name and fd
    char **child_cmd;

    // Caffelog
    char caffelog_filename[128];
    int caffelog_fd;
    regex_t caffelog_pattern;

    // Informations used by measure_rawdata()
    struct tm *calendar_start_time;   // GMT
    struct timespec     start_time;   // GMT

    //  In order to use sysfs interface easily.
    //  See addsysfs(), read_sysfs(), rawdata_to_stat()
    struct rawdata_info_struct rawdata_info[MAX_NUM_RAWDATA];
    int num_rawdata;
    char rawdata_filename[128];
    int rawdata_fd;
    int gpu_power_fd;
    char rawdata_print_format[256];
    char rawdata_scan_format[256];
    int rawdata_linesize;
    int offset_2ndstat;

    struct stat_info_struct stat_info[MAX_NUM_STAT];
    int num_stat;
    char stat_filename[128];

} measurement_info_struct;


#ifdef DEBUG
void print_info(const measurement_info_struct info);
void print_rawdata_info(const rawdata_info_struct rawdata_info);
#endif   // DEBUG

void init_info(measurement_info_struct *info);

void register_rawdata(
     measurement_info_struct *info,
     ssize_t (*func_read_rawdata)(const int rawdata_fd, ...),
     ssize_t (*func_rawdata_to_powerlog)(powerlog_struct *powerlog, const int rawdata_fd),
     const int num_sysfs_file, ...
);

void register_column(
     measurement_info_struct *info,
     const char *colname,
     const int colwidth,
     enum logtype_t logtype,
     ssize_t (*func_log_to_stat)(const int stat_fd, ...)
);

void close_sysfs_files(struct measurement_info_struct info);

#endif   // MEASUREMENT_INFO_H
