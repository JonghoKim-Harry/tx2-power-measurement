#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "measurement_info.h"

#ifdef DEBUG
void print_stat_info(const struct sysfs_stat stat_info) {

    int i;

    printf("\n--- stat_info ---");
    printf("\n * num_sysfs_fd: %d", stat_info.num_sysfs_fd);
    printf("\n * sysfs file info");
    for(i=0; i<stat_info.num_sysfs_fd; i++) {
        printf("\n   - max_strlen[%d]: %d", i, stat_info.max_strlen[i]);
        printf("\n   - sysfs_fd[%d]: %d", i, stat_info.sysfs_fd[i]);
    }
    printf("\n-----------------\n");
}

void print_info(const struct measurement_info info) {

    printf("\n--- info ---");
    printf("\n * rawdata_fd: %d", info.rawdata_fd);
    printf("\n * num_sysfs_data: %d", info.num_sysfs_data);
    printf("\n * header raw: %s", info.header_raw);
    printf("\n------------\n");

    return;
}
#endif   // DEBUG

void init_info(measurement_info_struct *info) {

    info->num_rawdata = 0;
    info->offset_2ndstat = 0;

    return;
}

void register_rawdata
    (measurement_info_struct *info,
     ssize_t (*func_read_rawdata)(const int rawdata_fd, ...),
     ssize_t (*func_rawdata_to_powerlog)(powerlog_struct *powerlog, const int rawdata_fd),
     const int num_sysfs_file, ...) {

    const int index = info->num_rawdata;
    rawdata_info_struct *rawdata_info = &info->rawdata_info[index];
    va_list sysfs_file_list;
    int i;
    char buff[128];

#ifdef DEBUG
    printf("\nregister_rawdata()   START");
    printf("\nregister_rawdata()   given parameters");
    printf("\n * func_read_rawdata: %p", func_read_rawdata);
    printf("\n * func_rawdata_to_powerlog: %p", func_rawdata_to_powerlog);
    printf("\n * num_sysfs_file: %d", num_sysfs_file);
#endif   // DEBUG

    // Register to rawdata_info:
    //  1) A function pointer to read rawdata: (ex) read_rawdata_1
    //  2) A function pointer to conver rawdata to powerlog: (ex) rawdata_to_powerlog_1
    //  3) Data size in bytes
    //  4) The number of sysfs files to read, sysfs files' fds,
    //     and maximum lengths of strings to read from sysfs files
    rawdata_info->func_read_rawdata = func_read_rawdata;
    rawdata_info->func_rawdata_to_powerlog = func_rawdata_to_powerlog;
    rawdata_info->num_sysfs_fd = num_sysfs_file;

    va_start(sysfs_file_list, num_sysfs_file);

    for(i=0; i<num_sysfs_file; i++) {
        strcpy(buff, va_arg(sysfs_file_list, char*));
        rawdata_info->sysfs_fd[i] = open(buff, O_RDONLY | O_NONBLOCK);
    }

    va_end(sysfs_file_list);

#ifdef DEBUG
    for(i=0; i<num_sysfs_file; i++) {
        printf("\nregister_sysfs()   rawdata_info->sysfs_fd[%d]: %d", i, rawdata_info->sysfs_fd[i]);
    }
#endif   // DEBUG

    // Register to info: the number of rawdata
    ++(info->num_rawdata);

#ifdef DEBUG
    printf("\nregister_sysfs()   FINISHED");
#endif   // DEBUG

    return;
}

void close_sysfs_files(measurement_info_struct info) {

    int i, j;
    rawdata_info_struct *target;
    int num1, num2;

#ifdef DEBUG
    printf("\nclose_sysfs_files()   START");
#endif   // DEBUG

    num1 = info.num_rawdata;

    for(i=0; i<num1; i++) {
    
        target = &info.rawdata_info[i];
        num2 = target->num_sysfs_fd;
        for(j=0; j<num2; j++)
            close(target->sysfs_fd[j]);
    }

#ifdef DEBUG
    printf("\nclose_sysfs_files()   FINISHED");
#endif   // DEBUG
};

