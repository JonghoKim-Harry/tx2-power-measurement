// Standard C Header Files
#include <stdio.h>
#include <stdlib.h>     // exit(), atol(), system()
#include <stdarg.h>     // va_list, va_start, va_arg, va_end, va_copy
#include <string.h>
#include <stdint.h>     // int32_t, int64_t
#include <time.h>       // struct timespec, localtime_r(), nanosleep()
#include <errno.h>      // strerror(errno)

// POSIX Headers Files
#include <unistd.h>     // access(), read(), write(), close(), getopt(), extern char *optarg, extern int optind, extern int optopt
#include <fcntl.h>      // open()
#include <sys/types.h>
#include <sys/wait.h>
#include <libgen.h>     // dirname()

// Custom Header Files
#include "read_sysfs_stat.h"
#include "tx2_sysfs_power.h"
#include "parse_caffelog.h"

#define AVAILABLE_OPTIONS "-c:f:h"

#define SECOND_TO_NANOSECOND 1000000000
#define HOUR_TO_SECOND 3600
#define WATTHOUR_TO_PICOWATTHOUR 1000000000000

// 9 hours = 540 minutes = 32400 seconds
#define GMT_TO_KOREA_TIME 32400

void help() {

    printf("\nJetson TX2 Power Measurement Script");
    printf("\nUsage: tx2_power_measurement [options] arguments\n");
    printf("\n\t%-20s%s", "-h",
            "Print help message");
    printf("\n\t%-20s%s", "-c <component>",
            "A component whose power consumption will be measured");
    printf("\n\t%21s%s\n", "", "* Supported components: all, cpu, gpu, ddr, wifi, soc");
    printf("\n\t%-20s%s", "-f <file name>",
                    "An execution file");
    printf("\n");
}

void prepare_measurement(const int argc, char *argv[], struct measurement_info *info) {

    //
    const char *message;
    int option, index;
    int cflag = 0, fflag = 0;
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
    char **cmd, cmd_str[256];

    char raw_power_filename[128];

    char buff[256], filename_buff[64], gmt_buff[256], korea_time_buff[256];
    size_t gmt_buff_len, korea_time_buff_len;
    int gpu_power_fd;
    struct timeval walltime;
    struct tm *walltime_callendar;

#ifdef DEBUG
    printf("\nprepare_measurement()   START");
#endif   // DEBUG

    while((option = getopt(argc, argv, AVAILABLE_OPTIONS)) != -1) {
        switch(option) {

        case 'c':
            // Process option -c
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

        case 'f':
            // Process option -f
            strcpy(stat_filename_buff, optarg);
            fflag = 1;
            break;

        case 'h':
            // Process option -h
            help();
            exit(0);

        case 1:
            // End argument processing when we meet the first non-optional argument
            optind--;
            goto end_arg_processing;

        case ':':
            // Missing arguments
            fprintf(stderr, "\nMissing arguments for option %c", optopt);
            help();
            exit(-1);

        case '?':
            // Invalid option
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

    cmd = (char **)malloc(sizeof(char *) * (argc-optind+1));

    for(index=0; index < (argc-optind); index++){
        cmd[index] = (char *)malloc(sizeof(char) * strlen(argv[index + optind]));
        strcpy(cmd[index], argv[index+optind]);
        strcat(cmd_str, cmd[index]);
        if(index != (argc-optind-1))
            strcat(cmd_str, " ");
    }

    cmd[argc-optind] = NULL;

    info->child_cmd = cmd;

    // GMT (Greenwich Mean Time)
    if(gettimeofday(&walltime, NULL) == -1) {
        perror("gettimeofday() call error");
        exit(-1);
    }

    walltime_callendar = localtime(&walltime.tv_sec);
    if(!walltime_callendar) {
        perror("localtime() call error");
        exit(-1);
    }
   
    strftime(buff, 64, "%Y-%m-%d %H:%M:%S", walltime_callendar);
    gmt_buff_len = snprintf(gmt_buff, 256, "\nStart measurement at %s (GMT)", buff);

    // Korea Time
    walltime.tv_sec += GMT_TO_KOREA_TIME;
    walltime_callendar = localtime(&walltime.tv_sec);
    if(!walltime_callendar) {
        perror("localtime() call error");
        exit(-1);
    }
   
    strftime(buff, 64, "%Y-%m-%d %H:%M:%S", walltime_callendar);
    korea_time_buff_len = snprintf(korea_time_buff, 256, "\nStart measurement at %s (Korea Timezone)", buff);


    gpu_power_fd = open(raw_power_filename, O_RDONLY | O_NONBLOCK);

    printf("\nCommand: %s\n", cmd_str);

    // OOO_stats.txt
    stat_filename = stat_filename_buff;
    stat_fd = open(stat_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    printf("\nCreated statistic file: %s", stat_filename);

    // Extract dirname and basename from the given stat file name
    //    * Note that dirname(), basename(), token_r()  may modify argument,
    //      thus, we use copied argument
    strcpy(filename_buff, stat_filename);
    strcpy(given_dirname, dirname(filename_buff));
    basename_ptr = stat_filename + strlen(given_dirname) + 1;
    strcpy(token, basename_ptr);
    strtok_r(token, ".", &next_token);
    strcpy(filename_prefix, token);

    // OOO.rawdata.bin
    strcpy(rawdata_filename, given_dirname);
    strcat(rawdata_filename, "/");
    strcat(rawdata_filename, filename_prefix);
    strcat(rawdata_filename, ".rawdata.bin");
    rawdata_fd = open(rawdata_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

    // OOO.powerlog.txt
    strcpy(powerlog_filename, given_dirname);
    strcat(powerlog_filename, "/");
    strcat(powerlog_filename, filename_prefix);
    strcat(powerlog_filename, ".powerlog.txt");
    powerlog_fd = open(powerlog_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

    // OOO.caffelog.txt
    strcpy(caffelog_filename, given_dirname);
    strcat(caffelog_filename, "/");
    strcat(caffelog_filename, filename_prefix);
    strcat(caffelog_filename, ".caffelog.txt");
    caffelog_fd = open(caffelog_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

    // Start logging
    write(stat_fd, "            JETSON TX2 POWER MEASUREMENT STATS\n", 47);

    message = "\n\n Measurement Informations";
    write(stat_fd, message, strlen(message));

    // Command
    message = "\n   * Running:   ";
    write(stat_fd, message, strlen(message));
    write(stat_fd, cmd_str, strlen(cmd_str));

    // Component
    message = "\n   * Component: ";
    write(stat_fd, message, strlen(message));
    write(stat_fd, component_str, strlen(component_str));
    write(stat_fd, " (", 2);
    write(stat_fd, raw_power_filename, strlen(raw_power_filename));
    write(stat_fd, ")", 1);

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
     *  Note that 'TIME' and 'GPU-Power' should NOT be counted
     */
    info->num_sysfs_data = 0;
    info->rawdata_linesize = 0;
    snprintf(buff, 256, "%*s%*s%*s",
            18, "GMT-Time-Stamp",
            30, "TIME",
            11, "GPU-Power");
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

    // Time information
    clock_gettime(CLOCK_REALTIME, &info->start_time);

    // Raw data file informations
    strcpy(info->rawdata_filename, rawdata_filename);
    info->rawdata_fd = rawdata_fd;

    // Powerlog file informations
    strcpy(info->powerlog_filename, powerlog_filename);
    info->powerlog_fd = powerlog_fd;

    // Caffelog file informations
    strcpy(info->caffelog_filename, caffelog_filename);
    info->caffelog_fd = caffelog_fd;

    dup2(powerlog_fd, STDERR_FILENO);
    dup2(powerlog_fd, STDOUT_FILENO);

    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "GPU-Freq", "%*sHz", ONE_SYSFS_FILE, TX2_SYSFS_GPU_FREQ, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

#ifdef TRACE_CPU
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "ALL-CPU-Power", "%*smW", ONE_SYSFS_FILE, TX2_SYSFS_POWER_CPU, TX2_SYSFS_CPU_POWER_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "CPU0-Freq", "%*skHz", ONE_SYSFS_FILE, TX2_SYSFS_CPU_FREQ(0), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);

    /* We should resolve too much measurement overhead
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU1-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(1), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(1), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU2-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(2), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(2), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU3-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(3), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(3), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU4-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(4), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(4), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU5-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(5), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(5), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    */
#endif   // TRACE_CPU


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
    regcomp(&info->timestamp_pattern, "[a-zA-Z_ \t\r\n\f]*([0-9]{2}:[0-9]{2}:[0-9]{2}[.][0-9]{6})[^\]]*\][ \t\r\n\f]*([^ \t\r\n\f].*)", REG_EXTENDED);
#pragma GCC diagnostic pop

#ifdef DEBUG
    printf("\nprepare_measurement()   FINISHED");
#endif   // DEBUG

    return;
}

void measure_rawdata(const int pid, const struct measurement_info info) {

    int i;
    const struct timespec sleep_request = {.tv_sec = 0, .tv_nsec = 4000};
    struct timespec sleep_remain;
    struct timespec curr_time;
    char gpu_power_str[TX2_SYSFS_GPU_POWER_MAX_STRLEN + 1];
    int child_status;
    int num_read_bytes;
    struct sysfs_stat stat_info;

#ifdef TRACE_DDR
    int mem_freq_fd;
    int mem_power;
    int mem_freq;
#endif  // TRACE_DDR

    while(1) {

        if(waitpid(pid, &child_status, WNOHANG))
            break;

        // Sleep, wakeup, and then check time
        if(nanosleep(&sleep_request, &sleep_remain) == -1) continue;
        if(clock_gettime(CLOCK_REALTIME, &curr_time) == -1) continue;

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

    // Caffelog
    int caffelog_fd;
    off_t offset;
    struct caffe_event event;
    int64_t caffelog_powerlog_hms_diff_ns;
    int caffelog_powerlog_comparison;

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
    struct tm *powerlog_callendar_timestamp;
    time_t elapsed_time_sec;
    int64_t elapsed_time_ns;
    int64_t diff_time_ns;
    int powerlog_buffered;      // flag
    int32_t gpu_energy;         // mW x ns
    int64_t gpu_energy_pWh;     // pWh (10^-12 Wh)
    int32_t gpu_energy_Wh;      // Wh
    int32_t prev_gpu_power, gpu_power, avg_gpu_power;   // mW

    struct sysfs_stat stat_info;

    // Caffelog
    offset = 0;
    event.offset = 0;
    caffelog_fd = open(info.caffelog_filename, O_RDONLY | O_NONBLOCK);

    // Rawdata
    rawdata_fd = open(info.rawdata_filename, O_RDONLY | O_NONBLOCK);
    lseek(rawdata_fd, 0, SEEK_SET);
    stat_fd = open(info.stat_filename, O_WRONLY);
    lseek(stat_fd, info.offset_2ndstat, SEEK_SET);

    /* Empty space for summary at the top of stat file */
    lseek(stat_fd, 103, SEEK_CUR);

    powerlog_buffered = 0;
    gpu_power = 0;
    gpu_energy = 0;
    gpu_energy_pWh = 0;
    gpu_energy_Wh = 0;

    printf("\nSTART calculating 2nd stats\n");
    write(stat_fd, "\n________________________________________________________________________________________________________________\n", 114);
    write(stat_fd, info.header_raw, strlen(info.header_raw));
    write(stat_fd, "\n----------------------------------------------------------------------------------------------------------------", 113);

    while(1) {

get_a_caffelog:
#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: get a caffelog timestamp");
#endif   // DEBUG

        if(offset < 0)
            goto write_a_powerlog;

        // Get a caffelog
        offset = parse_caffelog(caffelog_fd, info.timestamp_pattern, offset, &event);
        if(powerlog_buffered)
            goto compare_timestamp;

get_a_powerlog:
#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: get powerlog timestamp");
#endif   // DEBUG

        powerlog_buffered = 1;   // Set a flag

        // Get a powerlog timestamp
        prev_powerlog_timestamp = powerlog_timestamp;
        read_result = read(rawdata_fd, &powerlog_timestamp, sizeof(struct timespec));
        if(read_result <= 0) break;
        powerlog_callendar_timestamp = localtime(&powerlog_timestamp.tv_sec);

compare_timestamp:
        // TODO: Compare Timestmaps
        caffelog_powerlog_hms_diff_ns
        = compare_timestamp_hms(event.gmt_date_hms, *powerlog_callendar_timestamp);

        if(caffelog_powerlog_hms_diff_ns > 0)
            caffelog_powerlog_comparison = 1;
        else if(caffelog_powerlog_hms_diff_ns < 0)
            caffelog_powerlog_comparison = -1;
        else if(event.gmt_timestamp.tv_nsec > powerlog_timestamp.tv_nsec)
            caffelog_powerlog_comparison = 1;
        else if(event.gmt_timestamp.tv_nsec < powerlog_timestamp.tv_nsec)
            caffelog_powerlog_comparison = -1;
        else
            caffelog_powerlog_comparison = 0;

#ifdef DEBUG
        printf("\ncalculate_2ndstat()   caffelog_powerlog_hms_diff_ns: %ld", caffelog_powerlog_hms_diff_ns);
        printf("\ncalculate_2ndstat()   caffelog tv_nsec: %ld", event.gmt_timestamp.tv_nsec);
        printf("\ncalculate_2ndstat()   powerlog tv_nsec: %ld", powerlog_timestamp.tv_nsec);
        printf("\ncalculate_2ndstat()   caffelog_powerlog_comparison: %d", caffelog_powerlog_comparison);
#endif   // DEBUG

        // TODO: Decide
        if(caffelog_powerlog_comparison < 0)
            goto write_a_caffelog;
// end get_a_caffelog

write_a_powerlog:
#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: write a powerlog");
#endif   // DEBUG

        powerlog_buffered = 0;   // Reset a flag

        // Write powerlog: POWERLOG TIMESTAMP
        write(stat_fd, "\n", 1);
        strftime(time_buff, 256, "%H:%M:%S", powerlog_callendar_timestamp);
        buff_len = snprintf(buff, 256, "%s.%09ld", time_buff, powerlog_timestamp.tv_nsec);
        write(stat_fd, buff, buff_len);

        // Calculate and write powerlog: ELAPSED TIME
        elapsed_time_sec = powerlog_timestamp.tv_sec - info.start_time.tv_sec;
        elapsed_time_ns = powerlog_timestamp.tv_nsec - info.start_time.tv_nsec;

        if(elapsed_time_ns < 0) {
        
            -- elapsed_time_sec;
            elapsed_time_ns += SECOND_TO_NANOSECOND;
        }

        if(elapsed_time_sec == 0)
            buff_len = snprintf(buff, 256, "%19s%9ldns", " ", elapsed_time_ns);
        else
            buff_len = snprintf(buff, 256, "%19ld%09ldns", elapsed_time_sec, elapsed_time_ns);
        write(stat_fd, buff, buff_len);

        // Read and write powerlog: GPU POWER
        read_result = read(rawdata_fd, gpu_power_str, TX2_SYSFS_GPU_POWER_MAX_STRLEN);
        if(read_result <= 0) break;
        gpu_power_str[read_result] = '\0';
        buff_len = snprintf(buff, 256, "%4s%*smW", "", TX2_SYSFS_GPU_POWER_MAX_STRLEN, gpu_power_str);
        write(stat_fd, buff, buff_len);

        // Calculate powerlog: GPU ENERGY PARTIAL SUM
        for(i=0; i<info.num_sysfs_data; i++) {
            stat_info = info.stat_info[i];
            read_result = stat_info.rawdata_to_stat_func(stat_info, rawdata_fd, stat_fd);
            if (read_result <= 0) break;
        }

        prev_gpu_power = gpu_power;
        gpu_power = atoi(gpu_power_str);
        avg_gpu_power = (gpu_power + prev_gpu_power) / 2;
        diff_time_ns = SECOND_TO_NANOSECOND * (powerlog_timestamp.tv_sec - prev_powerlog_timestamp.tv_sec)
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

write_a_caffelog:
#ifdef DEBUG
        printf("\ncalculate_2ndstat()   BB: write a caffelog");
#endif   // DEBUG
        // TODO: Write caffelog
        write(stat_fd, "\n", 1);
        strftime(time_buff, 256, "%H:%M:%S", &event.gmt_date_hms);
        buff_len = snprintf(buff, 256, "%s.%09ld", time_buff, event.gmt_timestamp.tv_nsec);
        write(stat_fd, buff, buff_len);
        buff_len = snprintf(buff, 256, "%6s[Caffe]%6s%s", "      ", "      ", event.event);
        write(stat_fd, buff, buff_len);
        goto get_a_caffelog;
// end write_a_caffelog
    }

#ifdef DEBUG
    printf("\ncalculate_2ndstat() does NOT have infinite loop");
#endif   // DEBUG

    // Close and remove rawdata.bin file
    close(rawdata_fd);
    remove(info.rawdata_filename);

    lseek(stat_fd, info.offset_2ndstat, SEEK_SET);

    buff_len = snprintf(buff, 256, "\nTOTAL EXECUTION TIME: %19ld.%09ld second", elapsed_time_sec, elapsed_time_ns); // 59
    write(stat_fd, buff, buff_len);

    buff_len = snprintf(buff, 256, "\nTOTAL GPU ENERGY: %9d.%012ld Wh", gpu_energy_Wh, gpu_energy_pWh); // 44
    write(stat_fd, buff, buff_len);

    printf("\nEnd Jetson TX2 power measurement\n");
    close(stat_fd);
    close(caffelog_fd);   // Reopened caffelog's fd
    close(info.powerlog_fd);

    return;
}

int main(int argc, char *argv[]) {

    int pid;
    struct measurement_info info;

#ifdef DEBUG
    printf("\nYou are running debug mode");
#endif   // DEBUG

    prepare_measurement(argc, argv, &info);

    // Run
    pid = fork();

    if(pid == 0) {
        // Child Process
        dup2(info.caffelog_fd, STDERR_FILENO);
        execve(info.child_cmd[0], info.child_cmd, NULL);

        // If error, execve() returns -1. Otherwise, execve() does not return value
        perror("\nexecve() error");
    }

    // Parent Process
    measure_rawdata(pid, info);
    calculate_2ndstat(info);
    regfree(&info.timestamp_pattern);

    return 0;
}
