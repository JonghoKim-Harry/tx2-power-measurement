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
#include "sysfs_to_stat.h"
#include "tx2_sysfs_power.h"

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
    char rawdata_filename[128], stat_filename[128];
    char **cmd, cmd_str[256];

    char raw_power_filename[128];

    char buff[256], filename_buff[64], gmt_buff[256], korea_time_buff[256];
    size_t gmt_buff_len, korea_time_buff_len;
    int rawdata_fd;
    int stat_fd;
    int gpu_power_fd;
    struct timeval walltime;
    struct tm *walltime_detailed;

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
            strcpy(stat_filename, optarg);
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
        strcpy(stat_filename, "stats.txt");
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

    walltime_detailed = localtime(&walltime.tv_sec);
    if(!walltime_detailed) {
        perror("localtime() call error");
        exit(-1);
    }
   
    strftime(buff, 64, "%Y-%m-%d %H:%M:%S", walltime_detailed);
    gmt_buff_len = snprintf(gmt_buff, 256, "\nStart measurement at %s (GMT)", buff);

    // Korea Time
    walltime.tv_sec += GMT_TO_KOREA_TIME;
    walltime_detailed = localtime(&walltime.tv_sec);
    if(!walltime_detailed) {
        perror("localtime() call error");
        exit(-1);
    }
   
    strftime(buff, 64, "%Y-%m-%d %H:%M:%S", walltime_detailed);
    korea_time_buff_len = snprintf(korea_time_buff, 256, "\nStart measurement at %s (Korea Timezone)", buff);


    gpu_power_fd = open(raw_power_filename, O_RDONLY | O_NONBLOCK);

    printf("\nCommand: %s\n", cmd_str);

    //
    stat_fd = open(stat_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    printf("\nCreated statistic file: %s", stat_filename);

    //
    strcpy(filename_buff, stat_filename);
    strcpy(rawdata_filename, dirname(filename_buff));
    strcat(rawdata_filename, "/rawdata.bin");
    rawdata_fd = open(rawdata_filename, O_CREAT | O_TRUNC | O_WRONLY, 0644);

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

    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "GPU-Freq", "%*sHz", ONE_SYSFS_FILE, TX2_SYSFS_GPU_FREQ, TX2_SYSFS_GPU_FREQ_MAX_STRLEN);

#ifdef TRACE_CPU
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "ALL-CPU-Power", "%*smW", ONE_SYSFS_FILE, TX2_SYSFS_POWER_CPU, TX2_SYSFS_CPU_POWER_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_1, &rawdata_to_stat_1, "CPU0-Freq", "%*skHz", ONE_SYSFS_FILE, TX2_SYSFS_CPU_FREQ(0), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);

    /* TODO: Resolve too much measurement overhead
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU1-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(1), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(1), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU2-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(2), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(2), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU3-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(3), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(3), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU4-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(4), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(4), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    register_sysfs(info, &read_sysfs_2, &rawdata_to_stat_2, "CPU5-Freq", "%*s", TWO_SYSFS_FILES, TX2_SYSFS_CPU_ONLINE(5), TX2_SYSFS_CPU_ONLINE_MAX_STRLEN, TX2_SYSFS_CPU_FREQ(5), TX2_SYSFS_CPU_FREQ_MAX_STRLEN);
    */
#endif   // TRACE_CPU


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
    struct sysfs_stat_info stat_info;

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
    close_sysfs(info);
}

void calculate_2ndstat(const struct measurement_info info) {

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
    struct timespec prev_time, time;
    struct tm *caffe_format_time;
    int64_t diff_time_ns;
    time_t time_sec;
    int64_t time_ns;
    int32_t gpu_energy;         // mW x ns
    int64_t gpu_energy_pWh;     // pWh (10^-12 Wh)
    int32_t gpu_energy_Wh;      // Wh
    int32_t prev_gpu_power, gpu_power, avg_gpu_power;   // mW

    struct sysfs_stat_info stat_info;

    rawdata_fd = open(info.rawdata_filename, O_RDONLY | O_NONBLOCK);
    lseek(rawdata_fd, 0, SEEK_SET);
    stat_fd = open(info.stat_filename, O_WRONLY);
    lseek(stat_fd, info.offset_2ndstat, SEEK_SET);

    /* Empty space for summary at the top of stat file */
    lseek(stat_fd, 103, SEEK_CUR);

    gpu_power = 0;
    gpu_energy = 0;
    gpu_energy_pWh = 0;
    gpu_energy_Wh = 0;

    printf("\nSTART calculating 2nd stats\n");
    write(stat_fd, "\n________________________________________________________________________________________________________________\n", 114);
    write(stat_fd, info.header_raw, strlen(info.header_raw));
    write(stat_fd, "\n----------------------------------------------------------------------------------------------------------------", 113);

    while(1) {
        prev_time = time;
        prev_gpu_power = gpu_power;
        write(stat_fd, "\n", 1);

        read_result = read(rawdata_fd, &time, sizeof(struct timespec));
        if(read_result <= 0) break;

        // Time stamp in order to compare with Caffe time stamp
        caffe_format_time = localtime(&time.tv_sec);
        strftime(time_buff, 256, "%H:%M:%S", caffe_format_time);
        buff_len = snprintf(buff, 256, "%s.%09ld", time_buff, time.tv_nsec);
        write(stat_fd, buff, buff_len);

        time_sec = time.tv_sec - info.start_time.tv_sec;
        time_ns = time.tv_nsec - info.start_time.tv_nsec;

        if(time_ns < 0) {
        
            -- time_sec;
            time_ns += SECOND_TO_NANOSECOND;
        }

        // TIME
        if(time_sec == 0)
            buff_len = snprintf(buff, 256, "%19s%9ldns", " ", time_ns);
        else
            buff_len = snprintf(buff, 256, "%19ld%09ldns", time_sec, time_ns);
        write(stat_fd, buff, buff_len);

        // GPU POWER
        read_result = read(rawdata_fd, gpu_power_str, TX2_SYSFS_GPU_POWER_MAX_STRLEN);
        if(read_result <= 0) break;
        gpu_power_str[read_result] = '\0';
        buff_len = snprintf(buff, 256, "%4s%*smW", "", TX2_SYSFS_GPU_POWER_MAX_STRLEN, gpu_power_str);
        write(stat_fd, buff, buff_len);

        for(i=0; i<info.num_sysfs_data; i++) {
            stat_info = info.stat_info[i];
            read_result = stat_info.rawdata_to_stat_func(stat_info, rawdata_fd, stat_fd);
            if (read_result <= 0) break;
        }

        gpu_power = atoi(gpu_power_str);
        avg_gpu_power = (gpu_power + prev_gpu_power) / 2;
        diff_time_ns = SECOND_TO_NANOSECOND * (time.tv_sec - prev_time.tv_sec) + time.tv_nsec - prev_time.tv_nsec;

        gpu_energy += avg_gpu_power * diff_time_ns;

        if(gpu_energy >= HOUR_TO_SECOND) {
            gpu_energy_pWh += (gpu_energy / HOUR_TO_SECOND);
            gpu_energy %= HOUR_TO_SECOND;
        }

        if(gpu_energy_pWh >= WATTHOUR_TO_PICOWATTHOUR) {
            gpu_energy_Wh += (gpu_energy_pWh / WATTHOUR_TO_PICOWATTHOUR);
            gpu_energy_pWh %= WATTHOUR_TO_PICOWATTHOUR;
        }
    }

#ifdef DEBUG
    printf("\ncalculate_2ndstat() does NOT have infinite loop");
#endif   // DEBUG

    // Close and remove rawdata.bin file
    close(rawdata_fd);
    remove(info.rawdata_filename);

    lseek(stat_fd, info.offset_2ndstat, SEEK_SET);

    buff_len = snprintf(buff, 256, "\nTOTAL EXECUTION TIME: %19ld.%09ld second", time_sec, time_ns); // 59
    write(stat_fd, buff, buff_len);

    buff_len = snprintf(buff, 256, "\nTOTAL GPU ENERGY: %9d.%012ld Wh", gpu_energy_Wh, gpu_energy_pWh); // 44
    write(stat_fd, buff, buff_len);

    close(stat_fd);
    printf("\nEnd Jetson TX2 power measurement\n");

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
        execve(info.child_cmd[0], info.child_cmd, NULL);
        // If error, execve() returns -1. Otherwise, execve() does not return value
        perror("\nexecve() error");
    }
    else {
        // Parent Process
        measure_rawdata(pid, info);
        calculate_2ndstat(info);
    }
    return 0;
}
