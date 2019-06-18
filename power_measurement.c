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
#include "rawdata_to_powerlog.h"
#include "summary.h"
#include "parse_caffelog.h"
#include "log_to_stat.h"
#include "stat.h"
#include "tx2_sysfs_power.h"
#include "enhanced_shcmd.h"
#include "constants.h"
#include "default_values.h"

// Runtime
#include "runtime/collect_rawdata.h"
#include "governor/governor.h"

#define HELP_FIRST_COLWIDTH         30
#define AVAILABLE_OPTIONS   "-"   "c:f:g::hi:"

void help() {

    printf("\nJetson TX2 Power Measurement Script");
    printf("\nUsage: tx2_power_measurement [options] arguments\n");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-c <component>",
            "A component whose power consumption will be measured.");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "", "Supported components: all, cpu, gpu, ddr, wifi, soc");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-f <file name>",
                    "An execution file");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-g [<governor>]",
                    "Set an userspace governor");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-h",
            "Print help message");
    printf("\n\t%-*s%s", HELP_FIRST_COLWIDTH, "-i <interval in us>",
            "Measurement interval in micro-second (Default and MIN: 10000)");
    printf("\n");
}

void prepare_measurement(const int argc, char *argv[], measurement_info_struct *info) {

    //
    int option, index;
    int cflag = 0, fflag = 0, gflag = 0, iflag = 0;
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

    char buff[MAX_BUFFLEN], filename_buff[MAX_BUFFLEN];
    struct timeval walltime;
    struct tm *walltime_calendar;

#ifdef DEBUG
    printf("\nprepare_measurement()   START");
#endif   // DEBUG

    init_info(info);

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

    info->start_time.tv_sec = walltime.tv_sec;
    info->start_time.tv_nsec = MICRO_PER_NANO * walltime.tv_usec;
    info->calendar_start_time = *walltime_calendar;

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

        case 'g':   // option -g with optional argument
            gflag = 1;
            break;

        case 'h':   // option -h without argument
            help();
            exit(0);

        case 'i':   // option -i with required argument
            interval_us = atoi(optarg);
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

    if(gflag) {
        info->userspace_gpugovernor = 1;
        start_gpugovernor();
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

    print_expinfo(stat_fd, *info);

    // START RESERVATION
    info->summary_start = lseek(stat_fd, 0, SEEK_CUR);
    info->summary_len  = 0;

    // Reserve space to write summary
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n\nGPU Stat Summary");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * Elapsed Time:    %*s.%9s seconds", 9, "", "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Utilization: (MIN) %*s.%*s %s - %*s.%*s %s (MAX)", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Frequency:   (MIN) %*s MHz - %*s MHz (MAX)", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Power:       (MIN) %*s mW - %*s mW (MAX)", TX2_SYSFS_GPU_POWER_MAX_STRLEN, "", TX2_SYSFS_GPU_UTIL_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Energy:      %*s.%6s%6s%1s J", 9, "", "", "", "");

    // Reserve space to write summary during CNN
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n\nGPU Stat Summary during CNN");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * Elapsed Time:    %*s.%9s seconds", 9, "", "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Utilization: (MIN) %*s.%*s %s - %*s.%*s %s (MAX)", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), "", 1, "", "%");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Frequency:   (MIN) %*s MHz - %*s MHz (MAX)", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Power:       (MIN) %*s mW - %*s mW (MAX)", TX2_SYSFS_GPU_POWER_MAX_STRLEN, "", TX2_SYSFS_GPU_UTIL_MAX_STRLEN, "");
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n   * GPU Energy:      %*s.%6s%6s%1s J", 9, "", "", "", "");

    // FINISH RESERVATION
    info->summary_len += snprintf(buff, MAX_BUFFLEN, "\n");
    info->metadata_end = lseek(stat_fd, info->summary_len, SEEK_CUR);
    close(stat_fd);

#ifdef DEBUG
    printf("\n%s() in %s:%d   info->summary_start: %ld", __func__, __FILE__, __LINE__, info->summary_start);
    printf("\n%s() in %s:%d   info->summary_len: %ld", __func__, __FILE__, __LINE__, info->summary_len);
    printf("\n%s() in %s:%d   info->metadata_end: %ld", __func__, __FILE__, __LINE__, info->metadata_end);
#endif   // DEBUG

    // Statistics file informations
    strcpy(info->stat_filename, stat_filename);

    // Register rawdata to collect
    register_rawdata(info,  collect_timestamp,  timestamp_to_powerlog,   NO_SYSFS_FILE);
    register_rawdata(info,  collect_gpupower,   gpupower_to_powerlog,   ONE_SYSFS_FILE,  TX2_SYSFS_GPU_POWER);
    register_rawdata(info,  collect_gpufreq,    gpufreq_to_powerlog,    ONE_SYSFS_FILE,  TX2_SYSFS_GPU_FREQ);
    register_rawdata(info,  collect_gpuutil,    gpuutil_to_powerlog,    ONE_SYSFS_FILE,  TX2_SYSFS_GPU_UTIL);

    // Register statistics
    register_stat(info,  "Time(s)",            18,
                    LOGTYPE_TIME,              elapsedtime_to_stat);
    register_stat(info,  "GPU-power(mW)",      13,
                    LOGTYPE_POWERLOG,          gpupower_to_stat);
    register_stat(info,  "GPU-energy(J)",      21,
                    LOGTYPE_SUMMARY,           gpuenergy_to_stat);
    register_stat(info,  "GPU-freq(MHz)",      13,
                    LOGTYPE_POWERLOG,          gpufreq_to_stat);
    register_stat(info,  "GPU-util(%)",        11,
                    LOGTYPE_POWERLOG,          gpuutil_to_stat);
    register_stat(info,  "Timestamp",          19,
                    LOGTYPE_TIMESTAMP,         timestamp_to_stat);
    register_stat(info,  "CNN-start/finish",   16,
                    LOGTYPE_CAFFELOG,          cnn_event_to_stat);
    register_stat(info,  "Batch-idx",           9,
                    LOGTYPE_CAFFELOG,          batch_idx_to_stat);
    register_stat(info,  "Batch-finish",       13,
                    LOGTYPE_CAFFELOG,          batch_finish_to_stat);
    register_stat(info,  "Caffe-Event",        35,
                    LOGTYPE_CAFFELOG,          caffeevent_to_stat);\

    init_caffelog_parser();

#ifdef DEBUG
    printf("\nprepare_measurement()   FINISHED");
#endif   // DEBUG
    return;
}

void calculate_2ndstat(const measurement_info_struct info) {

    rawdata_info_struct            *rawdata_info;
    powerlog_struct                powerlog;
    caffelog_struct                *caffelog, list_caffelog;
    summary_struct                 summary, summary_cnn;
    stat_info_struct               *stat_info;
    ssize_t num_read_bytes, num_written_bytes;

    struct timespec *timestamp_ptr;
    int rawdata_fd;
    int caffelog_fd;
    off_t caffelog_offset;
    int stat_fd;
    int i, j;

    int flag_powerlog;
    int flag_cnnstart;
    int flag_cnnfinish;

    char buff[MAX_BUFFLEN];
    size_t buff_len;

    // Rawdata
    rawdata_fd = open(info.rawdata_filename, O_RDONLY | O_NONBLOCK);
    lseek(rawdata_fd, 0, SEEK_SET);

    // Caffelog
    caffelog_fd = open(info.caffelog_filename, O_RDONLY | O_NONBLOCK);
    caffelog_offset = 0;
    lseek(caffelog_fd, 0, SEEK_SET);

    // Statistics
    stat_fd = open(info.stat_filename, O_WRONLY);
    printf("\nSTART calculating 2nd stats\n");
    lseek(stat_fd, info.metadata_end, SEEK_SET);
    print_header_raw(stat_fd, info);

    init_summary(&summary);
    init_summary(&summary_cnn);
    flag_cnnstart  = 0;
    flag_cnnfinish = 0;

    // Process Caffe log file
    INIT_LIST_HEAD(&list_caffelog.list);
#ifdef DEBUG
    printf("\n%s() in %s:%d   Linked List for Caffelog: %p", __func__, __FILE__, __LINE__, &list_caffelog);
#endif   // DEBUG
    do {
        caffelog = malloc(sizeof(struct caffelog_struct));
        caffelog_offset = parse_caffelog(caffelog_fd, caffelog_offset, info.calendar_start_time, caffelog);
        if(caffelog_offset <= 0) {
            free(caffelog);
            break;
        }
        list_add_tail(caffelog, &list_caffelog.list);
    } while(1);

    close(caffelog_fd);

    if(!list_empty(&list_caffelog.list))
        caffelog = list_entry(list_caffelog.list.next, struct caffelog_struct, list);
    else
        caffelog = NULL;

    // Process rawdata and write to stat file.
    // Note that making powerlogs to linked list is unreasonable,
    // because we have too many powerlogs
#define PAD_COLUMN \
    do { \
        num_written_bytes  = write(stat_fd, WHITESPACE, (stat_info->colwidth - 4)); \
        num_written_bytes += write(stat_fd, "#N/A", 4); \
    } while(0);

    while(1) {

        // Read rawdata: GPU frequency, GPU utilization, CPU infos, etc.
        for(i=0; i<info.num_rawdata; i++) {
            rawdata_info = &info.rawdata_info[i];
            num_read_bytes = rawdata_info->func_rawdata_to_powerlog(&powerlog, rawdata_fd);
            if (num_read_bytes <= 0) goto rawdata_eof_found;
        }

        // Update summary
        update_summary(&summary, &powerlog);
        if(flag_cnnstart & (!flag_cnnfinish))
            update_summary(&summary_cnn, &powerlog);

compare_timestamp:
        // Compare timestamps and set/unset flag
        if(caffelog == NULL || diff_timestamp(powerlog.timestamp, caffelog->timestamp) < 0)
            flag_powerlog = 1;
        else
            flag_powerlog = 0;

        // Convert rawdata to stat and write to statfile
        write(stat_fd, "\n", 1);
        for(j=0; j<info.num_stat; j++) {
            write(stat_fd, "  ", 2);
            stat_info = &info.stat_info[j];
            switch(info.stat_info[j].logtype) {

                case LOGTYPE_TIME:
                    if(flag_powerlog)
                        timestamp_ptr = &powerlog.timestamp;
                    else
                        timestamp_ptr = &caffelog->timestamp;
                    num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, *timestamp_ptr, info.start_time);
                    break;

                case LOGTYPE_TIMESTAMP:
                    if(flag_powerlog)
                        timestamp_ptr = &powerlog.timestamp;
                    else
                        timestamp_ptr = &caffelog->timestamp;
                    num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, *timestamp_ptr);
                    break;

                case LOGTYPE_POWERLOG:
                    if(flag_powerlog)
                        num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, powerlog);
                    else 
                        PAD_COLUMN;
                    break;

                case LOGTYPE_SUMMARY:
                    if(flag_powerlog)
                        num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, summary);
                    else
                        PAD_COLUMN;
                    break;

                case LOGTYPE_CAFFELOG:
                    if(!flag_powerlog)
                        num_written_bytes = stat_info->func_log_to_stat(stat_fd, stat_info->colwidth, *caffelog);
                    else
                        PAD_COLUMN;
                 break;

                case LOGTYPE_TEGRALOG:
                case LOGTYPE_NA:
                default:
                    break;
            }

            if (num_written_bytes <= 0) break;
        }
 
        if(!flag_powerlog) {
            if(!list_empty(&list_caffelog.list)) {

#ifdef DEBUG
                printf("\n%s() in %s:%d   Iterate to next caffelog: %p -> ", __func__, __FILE__, __LINE__, caffelog);
#endif   // DEBUG

                list_del(&caffelog->list);
                free(caffelog);
                caffelog = list_entry(list_caffelog.list.next, struct caffelog_struct, list);
                if(caffelog == &list_caffelog)
                    caffelog = NULL;
                else {
                    if(caffelog->cnn_start == 100)
                        flag_cnnstart = 1;
                    if(caffelog->cnn_finish == 100)
                        flag_cnnfinish = 1;
                }
#ifdef DEBUG
                printf("%p", caffelog);
#endif   // DEBUG
            }
            else
                caffelog = NULL;
            goto compare_timestamp;
        }
    }   // while(1)
#undef PAD_COLUMN
rawdata_eof_found:

    // START writting summary
    lseek(stat_fd, info.summary_start, SEEK_SET);

    // Write summary
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGPU Stat Summary");
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * Elapsed Time:    %*ld.%09ld seconds", 9, elapsed_time(summary).tv_sec, elapsed_time(summary).tv_nsec);
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Utilization: (MIN) %*d.%*d %s - %*d.%*d %s (MAX)", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), (summary.min_gpu_util / 10), 1, (summary.min_gpu_util % 10), "%", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), (summary.max_gpu_util / 10), 1, (summary.max_gpu_util % 10), "%");
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Frequency:   (MIN) %*d MHz - %*d MHz (MAX)", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, summary.min_gpu_freq, TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, summary.max_gpu_freq);
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Power:       (MIN) %*d mW - %*d mW (MAX)", TX2_SYSFS_GPU_POWER_MAX_STRLEN, summary.min_gpu_power, TX2_SYSFS_GPU_UTIL_MAX_STRLEN, summary.max_gpu_power);
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Energy:      %*ld.%06ld%06ld%01ld J", 9, summary.gpu_energy_J, summary.gpu_energy_uJ, summary.gpu_energy_pJ, summary.gpu_energy_dotone_pJ);
    write(stat_fd, buff, buff_len);

    // Write summary during CNN
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n\nGPU Stat Summary during CNN");
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * Elapsed Time:    %*ld.%09ld seconds", 9, elapsed_time(summary_cnn).tv_sec, elapsed_time(summary_cnn).tv_nsec);
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Utilization: (MIN) %*d.%*d %s - %*d.%*d %s (MAX)", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), (summary_cnn.min_gpu_util / 10), 1, (summary_cnn.min_gpu_util % 10), "%", (TX2_SYSFS_GPU_UTIL_MAX_STRLEN - 1), (summary_cnn.max_gpu_util / 10), 1, (summary_cnn.max_gpu_util % 10), "%");
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Frequency:   (MIN) %*d MHz - %*d MHz (MAX)", TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, summary_cnn.min_gpu_freq, TX2_SYSFS_GPU_MHZFREQ_MAX_STRLEN, summary_cnn.max_gpu_freq);
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Power:       (MIN) %*d mW - %*d mW (MAX)", TX2_SYSFS_GPU_POWER_MAX_STRLEN, summary_cnn.min_gpu_power, TX2_SYSFS_GPU_UTIL_MAX_STRLEN, summary_cnn.max_gpu_power);
    write(stat_fd, buff, buff_len);
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n   * GPU Energy:      %*ld.%06ld%06ld%01ld J", 9, summary_cnn.gpu_energy_J, summary_cnn.gpu_energy_uJ, summary_cnn.gpu_energy_pJ, summary_cnn.gpu_energy_dotone_pJ);
    write(stat_fd, buff, buff_len);

    // FINISH writting summary
    buff_len = snprintf(buff, MAX_BUFFLEN, "\n");
    write(stat_fd, buff, buff_len);

    // Close and remove rawdata.bin file
    close(rawdata_fd);
    close(stat_fd);
    printf("\nEnd Jetson TX2 power measurement\n");

    return;
}

void finish_measurement(measurement_info_struct *info) {

    remove(info->rawdata_filename);
    close(info->powerlog_fd);

    if(info->flag_mkdir) {

        printf("\nchown -R %s", info->result_dirname);

        // $ chown -R
        if(chown_R(info->result_dirname, DEFAULT_UID, DEFAULT_GID) == -1)
            perror("chown_R() fail");
    }

    if(info->userspace_gpugovernor)
        finish_gpugovernor();

    free_caffelog_parser();

    return;
}

int main(int argc, char *argv[]) {

    int pid;
    measurement_info_struct info;
    struct timespec sleep_remain;

#ifdef DEBUG
    printf("\nYou are running debug mode");
#endif   // DEBUG

    /*
    if(geteuid() != 0) {

        printf("\nPlease run this power measurement tool as root privilege\n");
        exit(0);
    }
    */

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
