#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "measurement_info.h"

#ifdef DEBUG
void print_info(const struct measurement_info_struct info) {

    printf("\n--- info ---");
    printf("\n * num_rawdata: %d", info.num_rawdata);
    printf("\n * num_stat: %d", info.num_stat);
    printf("\n------------\n");

    return;
}

void print_rawdata_info(const rawdata_info_struct rawdata_info) {

    int i;

    printf("\n--- rawdta_info ---");
    printf("\n * num_sysfs_fd: %d", rawdata_info.num_sysfs_fd);
    printf("\n * sysfs file info");
    for(i=0; i<rawdata_info.num_sysfs_fd; i++) {
        printf("\n   - sysfs_fd[%d]: %d", i, rawdata_info.sysfs_fd[i]);
    }
    printf("\n-----------------\n");
}

#endif   // DEBUG

void init_info(measurement_info_struct *info) {

    info->userspace_gpugovernor = 0;
    info->num_rawdata = 0;
    info->num_stat = 0;

    return;
}

void register_rawdata
    (measurement_info_struct *info,
     ssize_t (*func_read_rawdata)(const int rawdata_fd, ...),
     ssize_t (*func_rawdata_to_powerlog)(powerlog_struct *powerlog, const int rawdata_fd),
     const int num_sysfs_file, ...) {

    const int idx = info->num_rawdata;
    rawdata_info_struct *rawdata_info = &info->rawdata_info[idx];
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
        printf("\nregister_rawdata()   rawdata_info->sysfs_fd[%d]: %d", i, rawdata_info->sysfs_fd[i]);
    }
#endif   // DEBUG

    // Register to info: the number of rawdata
    ++(info->num_rawdata);

#ifdef DEBUG
    printf("\nregister_rawdata()   FINISHED");
#endif   // DEBUG

    return;
}

void register_stat
    (measurement_info_struct *info,
     const char *colname,
     const int colwidth,
     enum logtype_t logtype,
     ssize_t (*func_log_to_stat)(const int stat_fd, const int colwidth, ...)) {

    const int idx = info->num_stat;
    stat_info_struct *stat_info = &info->stat_info[idx];

    strcpy(stat_info->colname, colname);
    stat_info->colwidth = colwidth;
    stat_info->logtype = logtype;
    stat_info->func_log_to_stat = func_log_to_stat;

    ++(info->num_stat);

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

