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

#include "read_sysfs_stat.h"
#include "tx2_sysfs_power.h"
#include "caffelog.h"
#include "mkdir_p.h"

#define AVAILABLE_OPTIONS   "-"   "c:f:hi:"
#define MIN_TIME_INTERVAL                       10000   // 10000 us = 10 ms
#define ONE_PER_MICRO                         1000000
#define MICRO_PER_NANO                           1000
#define ONE_MILLISECOND_TO_NANOSECOND         1000000
#define ONE_SECOND_TO_NANOSECOND           1000000000
// No use of macro in order to prevent integer overflow
const int64_t FOUR_SECONDS_TO_NANOSECOND = 4000000000;
#define WATTHOUR_TO_PICOWATTHOUR        1000000000000

#define HOUR_TO_SECOND       3600
#define GMT_TO_KOREA_TIME   32400   // 9 hours = 32400 seconds

#define HELP_FIRST_COLWIDTH   30

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

void prepare_measurement(const int argc, char *argv[], struct measurement_info *info) {

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

    char buff[256], filename_buff[64], gmt_buff[256], korea_time_buff[256];
    size_t buff_len, gmt_buff_len, korea_time_buff_len;
    int gpu_power_fd;
    struct timeval walltime;
    struct tm *walltime_calendar;

#ifdef DEBUG
    printf("\nprepare_measurement()   START");
#endif   // DEBUG

    while((option = getopt(argc, argv, AVAILABLE_OPTIONS)) != -1) {
        switch(option) {

        case 'c':   // option -c
            strcpy(component_str, optarg);

            if(!strcmp(optarg, "all"))
                strcpy(raw_power_filename, TX2_SYSFS_POWER_ALL);
            else if(!strcmp(optarg, "cpu"))
                strcpy(raw_power_filename, TX2_SYSFS_POWER_CPU);
            else if(!strcmp(optarg, "gpu"))
                strcpy(raw_power_filename, TX2_SYSFS_POWER_GPU);
            else if(!strcmp(optarg, "ddr"))
                strcpy(raw_power_filename, TX2_SYSFS_POWER_DDR);
            else if(!strcmp(optarg, "soc"))
                strcpy(raw_power_filename, TX2_SYSFS_POWER_SOC);
            else if(!strcmp(optarg, "wifi"))
                strcpy(raw_power_filename, TX2_SYSFS_POWER_WIFI);
            else {
                fprintf(stderr, "\nInvalid component!\n");
                help();
                exit(-1);
            }

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
    gmt_buff_len = snprintf(gmt_buff, 256, "\nStart measurement at %s (GMT)", buff);

    // Korea Timezone
    walltime.tv_sec += GMT_TO_KOREA_TIME;
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
    basename_ptr = stat_filename + strlen(given_dirname) + 1;
    strcpy(token, basename_ptr);
    strtok_r(token, ".", &next_token);
    strcpy(filename_prefix, token);

    // mkdir -p
    mkdir_p(given_dirname, 0755);

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

    /* 
     *  One-time Reading of Informations: CPUFREQ Groups, DEVFREQ Infos,
     *                                    Walltime, Stat Format, etc.
     */

    // CPUFreq Group0 Informations
    message = "\n\nCPUFreq Group0";
    write(stat_fd, message, strlen(message));
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_GOVERNOR(0), "\n   * Governor: %s");
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_REGISTERED_CPUS(0), "\n   * Registered CPUs: %s");
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_MINFREQ(0), "\n   * MIN Frequency: %skHz");
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_MAXFREQ(0), "\n   * MAX Frequency: %skHz");

    // CPUFreq Group1 Informations
    message = "\n\nCPUFreq Group1";
    write(stat_fd, message, strlen(message));
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_GOVERNOR(1), "\n   * Governor: %s");
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_REGISTERED_CPUS(1), "\n   * Registered CPUs: %s");
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_MINFREQ(1), "\n   * MIN Frequency: %skHz");
    sysfs_to_stat(stat_fd, TX2_SYSFS_CPUFREQ_GROUP_MAXFREQ(1), "\n   * MAX Frequency: %skHz");

    // GPUFreq Informations
    message = "\n\nGPU Frequency Informations";
    write(stat_fd, message, strlen(message));
    sysfs_to_stat(stat_fd, TX2_SYSFS_GPU_GOVERNOR, "\n   * Governor: %s");
    sysfs_to_stat(stat_fd, TX2_SYSFS_GPU_MINFREQ,  "\n   * MIN Frequency: %sHz");
    sysfs_to_stat(stat_fd, TX2_SYSFS_GPU_MAXFREQ,  "\n   * MAX Frequency: %sHz");

    // Walltime
    write(stat_fd, "\n\n", 2);
    write(stat_fd, gmt_buff, gmt_buff_len);
    write(stat_fd, korea_time_buff, korea_time_buff_len);

    // Raw data format: Column name of the statistics table
    write(stat_fd, "\n", 1);

    /*
     *  Prefix of header raw.
     *  Note that 'TIME' and 'GPU-Power' should NOT be counted
     */
    info->num_sysfs_data = 0;
    info->rawdata_linesize = 0;
    snprintf(buff, 256, "%*s%*s%*s",
            19, "GMT-Time-Stamp",
            28, "TIME(ns)",
            15, "GPU-Power(mW)");
    strcpy(info->header_raw, buff);
    strcpy(info->rawdata_print_format, "");
    strcpy(info->rawdata_scan_format, "");

    if(sizeof(time_t) <= sizeof(int32_t)) {
        strcpy(info->rawdata_print_format, "\n%11d%09dns");
        info->rawdata_linesize += 23;
        strcpy(info->rawdata_scan_format, "\n%20ldns");
    }
    else if(sizeof(time_t) == sizeof(int64_t)) {
        strcpy(info->rawdata_print_format, "\n%19ld%09dns");
        info->rawdata_linesize += 31;
        strcpy(info->rawdata_scan_format, "\n%28ldns");
    }

    // Write column names in the first raw of the statistics file
    info->offset_2ndstat = lseek(stat_fd, 0, SEEK_CUR);
    close(stat_fd);

    // Statistics file informations
    strcpy(info->stat_filename, stat_filename);
    info->gpu_power_fd = gpu_power_fd;


    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "GPU-Freq(Hz)", "%*s", ONE_SYSFS_FILE, TX2_SYSFS_GPU_FREQ, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_util1, "GPU-Util(%)", "%*s", ONE_SYSFS_FILE, TX2_SYSFS_GPU_UTIL, TX2_SYSFS_GPU_UTIL_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_util2, "EMC-Util(%)", "%*s", ONE_SYSFS_FILE, TX2_SYSFS_EMC_UTIL, TX2_SYSFS_GPU_UTIL_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "DDR-Power(mW)", "%*s", ONE_SYSFS_FILE, TX2_SYSFS_POWER_DDR, TX2_SYSFS_DDR_POWER_MAX_STRLEN);

#ifdef TRACE_CPU
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "ALL-CPU-Power(mW)", "%*s", ONE_SYSFS_FILE, TX2_SYSFS_POWER_CPU, TX2_SYSFS_CPU_POWER_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "CPU0-Freq(kHz)", "%*s", ONE_SYSFS_FILE, TX2_SYSFS_CPU_FREQ(0), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);

    /* We should resolve too much measurement overhead
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU1-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(1), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(1), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU2-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(2), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(2), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU3-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(3), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(3), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU4-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(4), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(4), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU5-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(5), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(5), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    */
#endif   // TRACE_CPU

    /*
     *  Suffix of header row.
     *  Caffe event should be the right-most column,
     *  in order to be freindly with MS Excel
     */
    snprintf(buff, 256, "%*s%*s%*s",
             22, "specific-Caffe-event",
             7, "batch",
             13, "Caffe-event");
    strcat(info->header_raw, buff);

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
    clock_gettime(CLOCK_REALTIME, &info->gmt_start_time);
    info->gmt_calendar_start_time = localtime(&info->gmt_start_time.tv_sec);
    return;
}

void measure_rawdata(const int pid, const struct measurement_info info) {

    int i;
    int flag_childexit;
    int64_t ns_since_childexit;
    int diff_ns, diff_sec;
    const struct timespec sleep_request = info.powertool_interval;
    struct timespec sleep_remain;
    struct timespec cooldown_remain;
    struct timespec prev_time, curr_time;
    char gpu_power_str[TX2_SYSFS_GPU_POWER_MAX_STRLEN + 1];
    int child_status;
    int num_read_bytes;
    struct sysfs_stat stat_info;

#ifdef TRACE_DDR
    int mem_freq_fd;
    int mem_power;
    int mem_freq;
#endif  // TRACE_DDR

    flag_childexit = 0;
    ns_since_childexit = 0;
    cooldown_remain = info.cooldown_period;

    while(1) {

        if(waitpid(pid, &child_status, WNOHANG))
            flag_childexit = 1;

        if(flag_childexit)
            prev_time = curr_time;

        if(clock_gettime(CLOCK_REALTIME, &curr_time) == -1) continue;

        if(flag_childexit) {
            diff_sec = curr_time.tv_sec - prev_time.tv_sec;
            diff_ns = curr_time.tv_nsec - prev_time.tv_nsec;

            cooldown_remain.tv_sec  -= diff_sec;
            cooldown_remain.tv_nsec -= diff_ns;

            while(cooldown_remain.tv_nsec >= ONE_SECOND_TO_NANOSECOND) {
                cooldown_remain.tv_nsec -= ONE_SECOND_TO_NANOSECOND;
                cooldown_remain.tv_sec  += 1;
            }

            while(cooldown_remain.tv_nsec < 0) {
                cooldown_remain.tv_nsec += ONE_SECOND_TO_NANOSECOND;
                cooldown_remain.tv_sec  -= 1;
            }

            if(cooldown_remain.tv_sec < 0)
                break;
        }

        // Write timestamp
        write(info.rawdata_fd, &curr_time, sizeof(struct timespec));

        // GPU Power
        lseek(info.gpu_power_fd, 0, SEEK_SET);
        // Note that read() may read EOF character at the end
        num_read_bytes = read(info.gpu_power_fd, gpu_power_str, TX2_SYSFS_GPU_POWER_MAX_STRLEN);
#ifdef DEBUG
        if(num_read_bytes <= 0)
            fprintf(stderr, "\nCould not read GPU power\n");
#endif   // DEBUG
        for(i=0; i<(TX2_SYSFS_GPU_POWER_MAX_STRLEN - (num_read_bytes-1) ); i++)
            write(info.rawdata_fd, " ", 1);
        write(info.rawdata_fd, gpu_power_str, (num_read_bytes-1));

        for(i=0; i<info.num_sysfs_data; i++) {
            stat_info = info.stat_info[i];
            stat_info.read_sysfs_func(stat_info, info.rawdata_fd);
        }

        // Sleep enough
        while(nanosleep(&sleep_request, &sleep_remain) == -1)
            nanosleep(&sleep_remain, &sleep_remain);
    }

#ifdef DEBUG
    printf("\nmeasure_rawdata() does NOT have infinite loop");
#endif   // DEBUG

    close(info.gpu_power_fd);
    close(info.rawdata_fd);
    close(info.caffelog_fd);
    close_sysfs(info);
}

void calculate_2ndstat(const struct measurement_info info) {

#ifdef TRACE_CAFFE_TIMESTAMP
    // Caffelog
    int caffelog_fd;
    off_t offset;
    caffelog_struct caffelog;
    int64_t caffelog_powerlog_hms_diff_ns;
    int caffelog_powerlog_comparison;
    int caffelog_buffered;   // flag
    int powerlog_buffered;   // flag
#endif   // TRACE_CAFFE_TIMESTAMP

    /*
     *  This function get file name of statistics file.
     *  Then, calculate 2nd stats and write to the given stat file
     */
    int i;
    ssize_t read_result;
    int rawdata_fd;
    int stat_fd;
    char time_buff[256], buff[256], gpu_power_str[TX2_SYSFS_GPU_POWER_MAX_STRLEN + 1];
    int buff_len;
    struct timespec prev_powerlog_timestamp, powerlog_timestamp;
    struct tm *powerlog_calendar_timestamp;
    const char separation_line[256] = "\n\n__________________________________________________________________________________________________________________________________________________________________________\n";
    time_t elapsed_time_sec;
    int64_t elapsed_time_ns;
    int64_t diff_time_ns;
    int32_t gpu_energy;         // mW x ns
    int64_t gpu_energy_pWh;     // pWh (10^-12 Wh)
    int32_t gpu_energy_Wh;      // Wh
    int32_t prev_gpu_power, gpu_power, avg_gpu_power;   // mW

    struct sysfs_stat stat_info;

#ifdef TRACE_CAFFE_TIMESTAMP
    // Caffelog
    offset = 0;
    caffelog_fd = open(info.caffelog_filename, O_RDONLY | O_NONBLOCK);
    caffelog.gmt_date_hms = *info.gmt_calendar_start_time;
    caffelog_buffered = 0;
    powerlog_buffered = 0;
#endif   // TRACE_CAFFE_TIMESTAMP

    // Rawdata
    rawdata_fd = open(info.rawdata_filename, O_RDONLY | O_NONBLOCK);
    lseek(rawdata_fd, 0, SEEK_SET);
    stat_fd = open(info.stat_filename, O_WRONLY);
    lseek(stat_fd, info.offset_2ndstat, SEEK_SET);

    /* Keep some empty space for summary at the top of stat file */
    lseek(stat_fd, 105, SEEK_CUR);

    gpu_power = 0;
    gpu_energy = 0;
    gpu_energy_pWh = 0;
    gpu_energy_Wh = 0;

    printf("\nSTART calculating 2nd stats\n");
    write(stat_fd, separation_line, strlen(separation_line));
    write(stat_fd, info.header_raw, strlen(info.header_raw));

    while(1) {

#ifdef TRACE_CAFFE_TIMESTAMP
#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: get a caffelog timestamp");
#endif   // DEBUG

        offset = parse_caffelog(caffelog_fd, info.caffelog_pattern, offset, &caffelog);
        if(offset < 0)    
            goto write_a_powerlog;

        caffelog_buffered = 1;   // Set a flag

        if(powerlog_buffered)
            goto compare_timestamp;
#endif   // TRACE_CAFFE_TIMESTAMP

get_a_powerlog:
#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: get powerlog timestamp");
#endif   // DEBUG

        // Get a powerlog timestamp
        prev_powerlog_timestamp = powerlog_timestamp;
        read_result = read(rawdata_fd, &powerlog_timestamp, sizeof(struct timespec));
        if(read_result <= 0) break;
        powerlog_calendar_timestamp = localtime(&powerlog_timestamp.tv_sec);

#ifdef TRACE_CAFFE_TIMESTAMP
        powerlog_buffered = 1;   // Set a flag

        if(!caffelog_buffered)
            goto write_a_powerlog;

compare_timestamp:
        assert(caffelog_buffered == 1);
        assert(powerlog_buffered == 1);

        caffelog_powerlog_hms_diff_ns
        = compare_timestamp_hms(caffelog.gmt_date_hms, *powerlog_calendar_timestamp);

        if(caffelog_powerlog_hms_diff_ns > 0)
            caffelog_powerlog_comparison = 1;
        else if(caffelog_powerlog_hms_diff_ns < 0)
            caffelog_powerlog_comparison = -1;
        else if(caffelog.gmt_timestamp.tv_nsec > powerlog_timestamp.tv_nsec)
            caffelog_powerlog_comparison = 1;
        else if(caffelog.gmt_timestamp.tv_nsec < powerlog_timestamp.tv_nsec)
            caffelog_powerlog_comparison = -1;
        else
            caffelog_powerlog_comparison = 0;

#ifdef DEBUG
        printf("\ncalculate_2ndstat()   caffelog_powerlog_hms_diff_ns: %ld", caffelog_powerlog_hms_diff_ns);
        printf("\ncalculate_2ndstat()   caffelog tv_nsec: %ld", caffelog.gmt_timestamp.tv_nsec);
        printf("\ncalculate_2ndstat()   powerlog tv_nsec: %ld", powerlog_timestamp.tv_nsec);
        printf("\ncalculate_2ndstat()   caffelog_powerlog_comparison: %d", caffelog_powerlog_comparison);
#endif   // DEBUG

        if(caffelog_powerlog_comparison < 0)
            goto write_a_caffelog;

write_a_powerlog:
#endif   // TRACE_CAFFE_TIMESTAMP

#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: write a powerlog");
#endif   // DEBUG


#ifdef TRACE_CAFFE_TIMESTAMP
        powerlog_buffered = 0;   // Reset a flag
#endif   // TRACE_CAFFE_TIMESTAMP

        // Write powerlog: POWERLOG TIMESTAMP
        write(stat_fd, "\n", 1);
        strftime(time_buff, 256, " %H:%M:%S", powerlog_calendar_timestamp);
        buff_len = snprintf(buff, 256, "%s.%09ld", time_buff, powerlog_timestamp.tv_nsec);
        write(stat_fd, buff, buff_len);

        // Calculate and write powerlog: ELAPSED TIME (ns)
        elapsed_time_sec = powerlog_timestamp.tv_sec - info.gmt_start_time.tv_sec;
        elapsed_time_ns = powerlog_timestamp.tv_nsec - info.gmt_start_time.tv_nsec;

        if(elapsed_time_ns < 0) {
        
            -- elapsed_time_sec;
            elapsed_time_ns += ONE_SECOND_TO_NANOSECOND;
        }

        if(elapsed_time_sec == 0)
            buff_len = snprintf(buff, 256, "%19s%9ld", " ", elapsed_time_ns);   // ns
        else
            buff_len = snprintf(buff, 256, "%19ld%09ld", elapsed_time_sec, elapsed_time_ns);   // ns
        write(stat_fd, buff, buff_len);

        // Read and write powerlog: GPU POWER (mW)
        read_result = read(rawdata_fd, gpu_power_str, TX2_SYSFS_GPU_POWER_MAX_STRLEN);
        if(read_result <= 0) break;
        gpu_power_str[read_result] = '\0';
        buff_len = snprintf(buff, 256, "%10s%*s", "", TX2_SYSFS_GPU_POWER_MAX_STRLEN, gpu_power_str);   // mW
        write(stat_fd, buff, buff_len);

        // Write sysfs data: GPU frequency, GPU utilization, CPU infos, etc.
        for(i=0; i<info.num_sysfs_data; i++) {
            stat_info = info.stat_info[i];
            read_result = stat_info.rawdata_to_stat_func(stat_info, rawdata_fd, stat_fd);
            if (read_result <= 0) break;
        }

        // Suffixes.
        // Write #N/A values to some last columns,
        // in order to be friendly with MS Excel.
        // The last column is Caffe-event
        buff_len = snprintf(buff, 256, "%*s%*s%*s",
                            22, "#N/A",
                            7, "#N/A",
                            13, "#N/A");
        write(stat_fd, buff, buff_len);

        // Calculate powerlog: GPU ENERGY PARTIAL SUM
        prev_gpu_power = gpu_power;
        gpu_power = atoi(gpu_power_str);
        avg_gpu_power = (gpu_power + prev_gpu_power) / 2;
        diff_time_ns = ONE_SECOND_TO_NANOSECOND * (powerlog_timestamp.tv_sec - prev_powerlog_timestamp.tv_sec)
                      + (powerlog_timestamp.tv_nsec - prev_powerlog_timestamp.tv_nsec);

        gpu_energy += avg_gpu_power * diff_time_ns;

        if(gpu_energy >= HOUR_TO_SECOND) {
            gpu_energy_pWh += (gpu_energy / HOUR_TO_SECOND);
            gpu_energy %= HOUR_TO_SECOND;
        }

        if(gpu_energy_pWh >= WATTHOUR_TO_PICOWATTHOUR) {
            gpu_energy_Wh += (gpu_energy_pWh / WATTHOUR_TO_PICOWATTHOUR);
            gpu_energy_pWh %= WATTHOUR_TO_PICOWATTHOUR;
        }
        goto get_a_powerlog;

#ifdef TRACE_CAFFE_TIMESTAMP
write_a_caffelog:
#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: write a caffelog");
#endif   // DEBUG

        assert(caffelog_buffered == 1);
        caffelog_buffered = 0;   // Reset a flag

        // Write a caffelog: Timestamp
        write(stat_fd, "\n", 1);
        strftime(time_buff, 256, " %H:%M:%S", &caffelog.gmt_date_hms);
        buff_len = snprintf(buff, 256, "%s.%09ld", time_buff, caffelog.gmt_timestamp.tv_nsec);
        write(stat_fd, buff, buff_len);

        // Calculate and write caffelog: ELAPSED TIME (ns)
        elapsed_time_sec = caffelog.gmt_timestamp.tv_sec - info.gmt_start_time.tv_sec;
        elapsed_time_ns = caffelog.gmt_timestamp.tv_nsec - info.gmt_start_time.tv_nsec;

        if(elapsed_time_ns < 0) {
        
            -- elapsed_time_sec;
            elapsed_time_ns += ONE_SECOND_TO_NANOSECOND;
        }

        if(elapsed_time_sec == 0)
            buff_len = snprintf(buff, 256, "%19s%9ld", " ", elapsed_time_ns);   // ns
        else
            buff_len = snprintf(buff, 256, "%19ld%09ld", elapsed_time_sec, elapsed_time_ns);   // ns
        write(stat_fd, buff, buff_len);

        // Put #N/A values for MS Excel
        buff_len = snprintf(buff, 256, "%15s", "#N/A");
        write(stat_fd, buff, buff_len);
        for(i=0; i<info.num_sysfs_data; i++) {

            buff_len = snprintf(buff, 256, "%*s", info.stat_info[i].column_width + 2, "#N/A");
            write(stat_fd, buff, buff_len);
        }

        // Write suffixes.
        // Note that Caffe-event should be excluded
        buff_len = snprintf(buff, 256, "%*s%*s",
                            22, "#N/A",
                            7, "#N/A");
        write(stat_fd, buff, buff_len);

        // Write a caffelog : Caffe-event
        buff_len = snprintf(buff, 256, "%2s%s", "  ", caffelog.event);
        write(stat_fd, buff, buff_len);
#endif   // TRACE_CAFFE_TIMESTAMP
    }   // while(1)

#ifdef DEBUG
    printf("\ncalculate_2ndstat() does NOT have infinite loop");
#endif   // DEBUG

    // Close and remove rawdata.bin file
    close(rawdata_fd);
    remove(info.rawdata_filename);

    lseek(stat_fd, info.offset_2ndstat, SEEK_SET);

    buff_len = snprintf(buff, 256, "\nTOTAL MEASUREMENT TIME: %19ld.%09ld second", elapsed_time_sec, elapsed_time_ns); // 59
    write(stat_fd, buff, buff_len);

    buff_len = snprintf(buff, 256, "\nTOTAL GPU ENERGY: %9d.%012ld Wh", gpu_energy_Wh, gpu_energy_pWh); // 44
    write(stat_fd, buff, buff_len);

    printf("\nEnd Jetson TX2 power measurement\n");
    close(stat_fd);
#ifdef TRACE_CAFFE_TIMESTAMP
    close(caffelog_fd);   // Reopened caffelog's fd
#endif   // TRACE_CAFFE_TIMESTAMP
    close(info.powerlog_fd);

    return;
}

void finish_measurement(struct measurement_info *info) {

    struct timespec gmt_finish_time, overall_interval;

    // Free objects
    regfree(&info->caffelog_pattern);

    // Overall Measurement Time
    clock_gettime(CLOCK_REALTIME, &gmt_finish_time);
    overall_interval.tv_sec = gmt_finish_time.tv_sec - info->gmt_start_time.tv_sec;
    overall_interval.tv_nsec = gmt_finish_time.tv_nsec - info->gmt_start_time.tv_nsec;

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
    struct measurement_info info;
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
