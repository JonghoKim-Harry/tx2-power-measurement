#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>
#include <errno.h>

#include "measurement_info.h"
#include "runtime/collect_rawdata.h"
#include "rawdata_to_powerlog.h"
#include "parse_caffelog.h"
#include "log_to_stat.h"
#include "stat.h"
#include "tx2_sysfs_power.h"
#include "enhanced_shcmd.h"
#include "constants.h"
#include "default_values.h"

#define HELP_FIRST_COLWIDTH         30
#define AVAILABLE_OPTIONS   "-"   "c:f:hi:"

void help() {

    printf("\nJetson TX2 Power Measurement Script");
    printf("\nUsage: tx2_power_measurement [options] arguments\n");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-c <component>",
            "A component whose power consumption will be measured.");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "", "Supported components: all, cpu, gpu, ddr, wifi, soc");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-f <file name>",
                    "An execution file");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-h",
            "Print help message");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-i <interval in us>",
            "Measurement interval in micro-second (Default and MIN: 10000)");
    printf("\n");
}

void prepare_measurement(const int argc, char *argv[], measurement_info_struct *info) {

    //
    const char *message;
    int option, index;
    int cflag = 0, fflag = 0, iflag = 0;
    int interval_us;
    char component_str[16];
    char given_dirname[128], filename_prefix[128], stat_filename_buff[128];
    const char *stat_filename, *basename_ptr;
    int stat_fd;
    char rawdata_filename[128];
    int rawdata_fd;
    char caffelog_filename[128];
    int caffelog_fd;
    char powerlog_filename[128];
    int powerlog_fd;
    char token[128], *next_token;
    char **child_cmd, child_cmd_str[256];
    char raw_power_filename[128];

    char buff[256], filename_buff[64], time_buff[256], korea_time_buff[256];
    size_t buff_len, time_buff_len, korea_time_buff_len;
    int gpu_power_fd;
    struct timeval walltime;
    struct tm *walltime_calendar;

#ifdef DEBUG
    printf("\nprepare_measurement()   START");
#endif   // DEBUG

    init_info(info);

    while((option = getopt(argc, argv, AVAILABLE_OPTIONS)) != -1) {
        switch(option) {

        case 'c':   // option -c
            strcpy(component_str, optarg);
            cflag = 1;
            break;

        case 'f':   // option -f with required argument
            strcpy(stat_filename_buff, optarg);
            fflag = 1;
            break;

        case 'h':   // option -h without argument
            help();
            exit(0);

        case 'i':   // option -i with required argument
            interval_us = atoi(optarg);
            if(interval_us < MIN_TIME_INTERVAL) {
                printf("\nYou should give time interval at least 10,000 us (10 ms)\n");
                exit(0);
            }
            info->powertool_interval.tv_sec = interval_us / ONE_PER_MICRO;
            info->powertool_interval.tv_nsec = (interval_us % ONE_PER_MICRO) * MICRO_PER_NANO;
            iflag = 1;
            break;

        case 1:   // non-optional argument
            // End argument processing
            optind--;
            goto end_arg_processing;

        case ':':   // Missing arguments
            fprintf(stderr, "\nMissing arguments for option %c", optopt);
            help();
            exit(-1);

        case '?':   // Invalid option
            fprintf(stderr, "\nInvalid option: %c\n", optopt);
            break;
        }
    }

end_arg_processing:

    if(!fflag) {

        printf("\nYou did not specify statistics file name; thus, the file name is set to the default name, stats.txt");
        strcpy(stat_filename_buff, "stats.txt");
    }

    if(!cflag) {

        fprintf(stderr, "\nYou should specify which component to measure");
        help();
        exit(-1);
    }

    if(argc == optind) {

        fprintf(stderr, "\nYou should give more arguments: the program to run is missing");
        help();
        exit(-1);
    }

    child_cmd = (char **)malloc(sizeof(char *) * (argc-optind+1));

    for(index=0; index < (argc-optind); index++){
        child_cmd[index] = (char *)malloc(sizeof(char) * strlen(argv[index + optind]));
        strcpy(child_cmd[index], argv[index+optind]);
        strcat(child_cmd_str, child_cmd[index]);
        if(index != (argc-optind-1))
            strcat(child_cmd_str, " ");
    }

    child_cmd[argc-optind] = NULL;

    info->child_cmd = child_cmd;

    // GMT (Greenwich Mean Time)
    if(gettimeofday(&walltime, NULL) == -1) {
        perror("gettimeofday() call error");
        exit(-1);
    }

    walltime_calendar = localtime(&walltime.tv_sec);
    if(!walltime_calendar) {
        perror("localtime() call error");
        exit(-1);
    }

    strftime(buff, 64, "%Y-%m-%d %H:%M:%S", walltime_calendar);
    time_buff_len = snprintf(time_buff, 256, "\nStart measurement at %s (GMT)", buff);

    // Korea Timezone
    walltime.tv_sec += SECONDS_GMT_TO_KOREA_TIME;
    walltime_calendar = localtime(&walltime.tv_sec);
    if(!walltime_calendar) {
        perror("localtime() call error");
        exit(-1);
    }
 
    strftime(buff, 64, "%Y-%m-%d %H:%M:%S", walltime_calendar);
    korea_time_buff_len = snprintf(korea_time_buff, 256, "\nStart measurement at %s (Korea Timezone)", buff);

    // Set Caffe sleep request
    info->caffe_sleep_request.tv_sec =  2;
    info->caffe_sleep_request.tv_nsec = 0;

    // Set powertool measurement interval
    if(!iflag) {
        printf("\nSet measurement interval as default: 10 ms");
        info->powertool_interval.tv_sec = 0;
        info->powertool_interval.tv_nsec = 10 * ONE_MILLISECOND_TO_NANOSECOND;
    }

    // Set cooldown period
    info->cooldown_period.tv_sec =  6;
    info->cooldown_period.tv_nsec = 0;

    gpu_power_fd = open(raw_power_filename, O_RDONLY | O_NONBLOCK);

    printf("\nCommand: %s\n", child_cmd_str);


    // Extract dirname and basename from the given stat file name
    //    * Note that dirname(), basename(), token_r()  may modify argument,
    //      thus, we use copied argument
    stat_filename = stat_filename_buff;
    strcpy(filename_buff, stat_filename);
    strcpy(given_dirname, dirname(filename_buff));
    strcpy(info->result_dirname, given_dirname);
    basename_ptr = stat_filename + strlen(given_dirname) + 1;
    strcpy(token, basename_ptr);
    strtok_r(token, ".", &next_token);
    strcpy(filename_prefix, token);

    if(access(given_dirname, F_OK) == -1) {

        info->flag_mkdir = 1;

        // $ mkdir -p
        if(mkdir_p(given_dirname, 0755) == -1)
            perror("mkdir_p()   fail");
    }
    else
        info->flag_mkdir = 0;

    // Powerlog File: OOO.powerlog.txt
    strcpy(powerlog_filename, given_dirname);
    strcat(powerlog_filename, "/");
    strcat(powerlog_filename, filename_prefix);
    strcat(powerlog_filename, ".powerlog.txt");
    strcpy(info->powerlog_filename, powerlog_filename);
    powerlog_fd = open(powerlog_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    info->powerlog_fd = powerlog_fd;

    dup2(powerlog_fd, STDERR_FILENO);
    dup2(powerlog_fd, STDOUT_FILENO);

    // Rawdata File: OOO.rawdata.bin
    strcpy(rawdata_filename, given_dirname);
    strcat(rawdata_filename, "/");
    strcat(rawdata_filename, filename_prefix);
    strcat(rawdata_filename, ".rawdata.bin");
    strcpy(info->rawdata_filename, rawdata_filename);
    rawdata_fd = open(rawdata_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    info->rawdata_fd = rawdata_fd;

    // Caffelog File: OOO.caffelog.txt
    strcpy(caffelog_filename, given_dirname);
    strcat(caffelog_filename, "/");
    strcat(caffelog_filename, filename_prefix);
    strcat(caffelog_filename, ".caffelog.txt");
    strcpy(info->caffelog_filename, caffelog_filename);
    caffelog_fd = open(caffelog_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    info->caffelog_fd = caffelog_fd;

    // Statistics File: OOO.txt
    stat_fd = open(stat_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    printf("\nCreated statistic file: %s", stat_filename);

    // Start logging to statistics file
    lseek(stat_fd, 0, SEEK_SET);
    write(stat_fd, "            JETSON TX2 POWER MEASUREMENT STATS\n", 47);

    message = "\n\n Measurement Informations";
    write(stat_fd, message, strlen(message));

    // Child Command
    message = "\n   * Running:   ";
    write(stat_fd, message, strlen(message));
    write(stat_fd, child_cmd_str, strlen(child_cmd_str));

    // Component
    message = "\n   * Component: ";
    write(stat_fd, message, strlen(message));
    write(stat_fd, component_str, strlen(component_str));
    write(stat_fd, " (", 2);
    write(stat_fd, raw_power_filename, strlen(raw_power_filename));
    write(stat_fd, ")", 1);

    // Caffe sleep period
    buff_len = snprintf(buff, 256, "\n   * Sleep:    %ld.%09ld seconds before Caffe START",
                        info->caffe_sleep_request.tv_sec,
                        info->caffe_sleep_request.tv_nsec);
    write(stat_fd, buff, buff_len);

    // Measurement Interval
    buff_len = snprintf(buff, 256, "\n   * Interval: %ld.%09ld seconds",
                        info->powertool_interval.tv_sec,
                        info->powertool_interval.tv_nsec);
    write(stat_fd, buff, buff_len);

    // Cooldown Period
    buff_len = snprintf(buff, 256, "\n   * Cooldown: %ld.%09ld seconds after  Caffe FINISH",
                        info->cooldown_period.tv_sec,
                        info->cooldown_period.tv_nsec);
    write(stat_fd, buff, buff_len);

    // Walltime
    write(stat_fd, "\n\n", 2);
    write(stat_fd, time_buff, time_buff_len);
    write(stat_fd, korea_time_buff, korea_time_buff_len);

    // Raw data format: Column name of the statistics table
    write(stat_fd, "\n", 1);

    /*
     *  Prefix of header raw.
     *  Note that 'TIME' and 'GPU-Power' should NOT be counted
     */
    snprintf(buff, 256, "%*s%*s%*s",
            19, "GMT-Time-Stamp",
            28, "TIME(ns)",
            15, "GPU-Power(mW)");

    // Write column names in the first raw of the statistics file
    info->metadata_end = lseek(stat_fd, 0, SEEK_CUR);
    close(stat_fd);

    // Statistics file informations
    strcpy(info->stat_filename, stat_filename);

    // TODO: Register rawdata to collect
    register_rawdata(info,  collect_timestamp,  timestamp_to_powerlog,   NO_SYSFS_FILE);
    register_rawdata(info,  collect_gpupower,   gpupower_to_powerlog,   ONE_SYSFS_FILE,  TX2_SYSFS_GPU_POWER);
    register_rawdata(info,  collect_gpufreq,    gpufreq_to_powerlog,    ONE_SYSFS_FILE,  TX2_SYSFS_GPU_FREQ);
    register_rawdata(info,  collect_gpuutil,    gpuutil_to_powerlog,    ONE_SYSFS_FILE,  TX2_SYSFS_GPU_UTIL);

    // TODO: Register statistics

/*
    register_stat(info,  "Time(ns)",            28,
                    LOGTYPE_POWERLOG_SUMMARY,     elapsedtime_to_stat);
*/
    register_stat(info,  "GPU-power(mW)",       13,
                    LOGTYPE_POWERLOG,             gpupower_to_stat);
/*
    register_stat(info,  "GPU-energy(pWh)",     15,
                    LOGTYPE_POWERLOG_SUMMARY,     gpuenergy_to_stat);
*/
    register_stat(info,  "GPU-freq(MHz)",       13,
                    LOGTYPE_POWERLOG,             gpufreq_to_stat);
    register_stat(info,  "GPU-util(%)",         11,
                    LOGTYPE_POWERLOG,             gpuutil_to_stat);
    register_stat(info,  "Timestamp",           18,
                    LOGTYPE_POWERLOG,             timestamp_to_stat);

    /*
    register_stat(info,  "Event:CAFFE_START",       17,
                    LOGTYPE_CAFFELOG,     
    register_stat(info,  "Event:CNN_START",         15,
                    LOGTYPE_CAFFELOG,     
    register_stat(info,  "State:GPU_POWER_PEAK",    20,
                    LOGTYPE_POWERLOG_SUMMARY,     
    register_stat(info,  "Event:CNN_FINISH",        16,
                    LOGTYPE_CAFFELOG,     
    register_stat(info,  "Event:CAFFE_FINISH",      18,
                    LOGTYPE_CAFFELOG,     
    register_stat(info,  "State:GPU_POWER_IDLE",    20,
                    LOGTYPE_POWERLOG_SUMMARY,     
    */


    /*
     *  Suffix of header row.
     *  Caffe event should be the right-most column,
     *  in order to be freindly with MS Excel
    snprintf(buff, 256, "%*s%*s%*s",
             22, "specific-Caffe-event",
             7, "batch",
             13, "Caffe-event");
     */

    /*
     *   Ignore the compilation warning message: 
     *
     *      warning: unknown escape sequence: '\]'
     *
     *   REG_EXTENDED supports the escape sequence '\]', thus just ignore it
     */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
    // Produce special data structure for fast regex execution
    regcomp(&info->caffelog_pattern, CAFFELOG_PATTERN, REG_EXTENDED);
#pragma GCC diagnostic pop

#ifdef DEBUG
    printf("\nprepare_measurement()   FINISHED");
#endif   // DEBUG

    // Time information
    // Note that this job should be done at last to be more accurate
    clock_gettime(CLOCK_REALTIME, &info->start_time);
    info->calendar_start_time = localtime(&info->start_time.tv_sec);
    return;
}

void calculate_2ndstat(const measurement_info_struct info) {

    rawdata_info_struct            *rawdata_info;
    powerlog_struct                 powerlog;
    powerlog_summary_struct         powerlog_summary;
    stat_info_struct               *stat_info;
    ssize_t num_read_bytes, num_written_bytes;

    int rawdata_fd;
    int stat_fd;
    int i, j;

    // Rawdata
    rawdata_fd = open(info.rawdata_filename, O_RDONLY | O_NONBLOCK);
    lseek(rawdata_fd, 0, SEEK_SET);

    // Statistics
    stat_fd = open(info.stat_filename, O_WRONLY | O_APPEND);
    printf("\nSTART calculating 2nd stats\n");
    print_header_raw(stat_fd, info);

    while(1) {

        // Read rawdata: GPU frequency, GPU utilization, CPU infos, etc.
        for(i=0; i<info.num_rawdata; i++) {
            rawdata_info = &info.rawdata_info[i];
            num_read_bytes = rawdata_info->func_rawdata_to_powerlog(&powerlog, rawdata_fd);
            if (num_read_bytes <= 0) goto eof_found;
        }

        write(stat_fd, "\n", 1);
        // Convert rawdata to stat
        for(j=0; j<info.num_stat; j++) {
            write(stat_fd, "  ", 2);
            stat_info = &info.stat_info[j];
            switch(info.stat_info[j].logtype) {

                case LOGTYPE_POWERLOG:
                    num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, powerlog);
                    break;

                case LOGTYPE_POWERLOG_SUMMARY:
                    num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, powerlog_summary);
                    break;

                //case LOGTYPE_CAFFELOG: stat_info->func_log_to_stat(stat_fd, caffelog); break;

                //case LOGTYPE_TEGRALOG: stat_info->func_log_to_stat(stat_fd, tegralog); break;
                case LOGTYPE_NA:
                default:
                    break;
            }

            if (num_written_bytes <= 0) break;
        }
    }   // while(1)
eof_found:


#ifdef DEBUG
    printf("\ncalculate_2ndstat() does NOT have infinite loop");
#endif   // DEBUG

    // Close and remove rawdata.bin file
    close(rawdata_fd);
    close(stat_fd);
    printf("\nEnd Jetson TX2 power measurement\n");

    return;
}

void finish_measurement(measurement_info_struct *info) {

    struct timespec finish_time, overall_interval;

    remove(info->rawdata_filename);
    close(info->powerlog_fd);

    if(info->flag_mkdir) {

        printf("\nchown -R %s", info->result_dirname);

        // $ chown -R
        if(chown_R(info->result_dirname, DEFAULT_UID, DEFAULT_GID) == -1)
            perror("chown_R() fail");
    }

    // Free objects
    regfree(&info->caffelog_pattern);

    // Overall Measurement Time
    clock_gettime(CLOCK_REALTIME, &finish_time);
    overall_interval.tv_sec = finish_time.tv_sec - info->start_time.tv_sec;
    overall_interval.tv_nsec = finish_time.tv_nsec - info->start_time.tv_nsec;

    if(overall_interval.tv_nsec < 0) {
        -- overall_interval.tv_sec;
        overall_interval.tv_nsec += ONE_SECOND_TO_NANOSECOND;
    }

    printf("\nTOTAL MEASUREMENT TOOK: %ld.%09ld seconds",
           overall_interval.tv_sec,
           overall_interval.tv_nsec);
}

int main(int argc, char *argv[]) {

    int pid;
    measurement_info_struct info;
    struct timespec sleep_remain;

#ifdef DEBUG
    printf("\nYou are running debug mode");
#endif   // DEBUG

    if(geteuid() != 0) {

        printf("\nPlease run this power measurement tool as root privilege\n");
        exit(0);
    }

    prepare_measurement(argc, argv, &info);

    // Run
    pid = fork();

    if(pid == 0) {
        // Child Process
        dup2(info.caffelog_fd, STDOUT_FILENO);
        dup2(info.caffelog_fd, STDERR_FILENO);

        // Sleep enough
        while(nanosleep(&info.caffe_sleep_request, &sleep_remain) == -1)
            nanosleep(&sleep_remain, &sleep_remain);

        execvp(info.child_cmd[0], info.child_cmd);

        // If error, execve() returns -1. Otherwise, execve() does not return value
        perror("\nexecve() error");
    }

    // Parent Process
    measure_rawdata(pid, info);
    calculate_2ndstat(info);
    finish_measurement(&info);
    return 0;
}
